#include "../stdafx.h"
#include "../DirectXRaytracingHelper.h"
#include "AccelerationStructure.h"

namespace RtxEngine
{
	AccelerationStructure::AccelerationStructure(const StaticScenePtr& scene, DxrDevicePtr& device, DxrCommandListPtr& commandList, DeviceResourcesPtr& deviceResources,
		const XMMATRIX& triangleTransform, const XMMATRIX& proceduralTransform)
		: m_device(device),
		m_commandList(commandList),
		m_deviceResources(deviceResources),
		m_scene(scene),
		m_blasTransforms{ triangleTransform, proceduralTransform }
	{
		build();
	}

	AccelerationStructure::~AccelerationStructure()
	{
		for(auto& blas : m_bottomLevelAS)
		{
			blas.Reset();
		}
		m_topLevelAS.Reset();
	}

	// Build acceleration structure needed for raytracing.
	void AccelerationStructure::build()
	{
		auto device = m_deviceResources->GetD3DDevice();
		auto commandList = m_deviceResources->GetCommandList();
		auto commandQueue = m_deviceResources->GetCommandQueue();
		auto commandAllocator = m_deviceResources->GetCommandAllocator();

		// Reset the command list for the acceleration structure construction.
		commandList->Reset(commandAllocator, nullptr);

		// Build bottom-level AS.
		auto blasDescs = buildGeometryDescsForBottomLevelAS();
		
		vector<AccelerationStructureBuffers> bottomLevelAS;
		for (auto descriptors : blasDescs)
		{
			auto currentBlas = buildBottomLevelAS(descriptors);
			bottomLevelAS.push_back(currentBlas);
		}

		// Batch all resource barriers for bottom-level AS builds.
		vector<D3D12_RESOURCE_BARRIER> resourceBarriers;
		for (auto currentBlas : bottomLevelAS)
		{
			D3D12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::UAV(currentBlas.accelerationStructure.Get());
			resourceBarriers.push_back(resourceBarrier);
		}

		commandList->ResourceBarrier(2, resourceBarriers.data());

		// Build top-level AS.
		AccelerationStructureBuffers topLevelAS = buildTopLevelAS(bottomLevelAS);

		// Kick off acceleration structure construction.
		m_deviceResources->ExecuteCommandList();

		// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
		m_deviceResources->WaitForGpu();

		// Store the AS buffers. The rest of the buffers will be released once we exit the function.
		m_bottomLevelAS.push_back(bottomLevelAS[0].accelerationStructure);
		m_bottomLevelAS.push_back(bottomLevelAS[1].accelerationStructure);
		m_topLevelAS = topLevelAS.accelerationStructure;
	}

	// Build geometry descs for bottom-level AS.
	AccelerationStructure::BlasDescriptors AccelerationStructure::buildGeometryDescsForBottomLevelAS() const
	{
		// Mark the geometry as opaque. 
		// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
		// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
		D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		vector<D3D12_RAYTRACING_GEOMETRY_DESC> triGeometryDescs;
		vector<D3D12_RAYTRACING_GEOMETRY_DESC> procGeometryDescs;

		for (const auto& geometry : m_scene->getGeometry())
		{
			auto vertexBuffer = geometry->getVertexBuffer();

			if (geometry->getType() == Geometry::Triangles)
			{
				auto indexBuffer = geometry->getIndexBuffer();

				D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
				geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				geometryDesc.Triangles.IndexBuffer = indexBuffer.resource->GetGPUVirtualAddress();
				geometryDesc.Triangles.IndexCount = static_cast<UINT>(indexBuffer.resource->GetDesc().Width) / sizeof(Index);
				geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
				geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
				geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.resource->GetGPUVirtualAddress();
				geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
				geometryDesc.Flags = geometryFlags;

				triGeometryDescs.push_back(geometryDesc);
			}
			else
			{
				D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
				geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
				geometryDesc.AABBs.AABBCount = 1;
				geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
				geometryDesc.Flags = geometryFlags;
				geometryDesc.AABBs.AABBs.StartAddress = vertexBuffer.resource->GetGPUVirtualAddress();

				procGeometryDescs.push_back(geometryDesc);
			}
		}

		return BlasDescriptors{ triGeometryDescs, procGeometryDescs };
	}

	AccelerationStructureBuffers AccelerationStructure::buildBottomLevelAS(
		const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs,
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags
	)
	{
		auto device = m_deviceResources->GetD3DDevice();
		auto commandList = m_deviceResources->GetCommandList();
		ComPtr<ID3D12Resource> scratch;
		ComPtr<ID3D12Resource> bottomLevelAS;

		// Get the size requirements for the scratch and AS buffers.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
		bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bottomLevelInputs.Flags = buildFlags;
		bottomLevelInputs.NumDescs = static_cast<UINT>(geometryDescs.size());
		bottomLevelInputs.pGeometryDescs = geometryDescs.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
		m_device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

		// Create a scratch buffer.
		AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

		// Allocate resources for acceleration structures.
		// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
		// Default heap is OK since the application doesn’t need CPU read/write access to them. 
		// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
		// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
		//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
		//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
		{
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
		}

		// bottom-level AS desc.
		{
			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
			bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelAS->GetGPUVirtualAddress();
		}

		// Build the acceleration structure.
		m_commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

		AccelerationStructureBuffers bottomLevelASBuffers;
		bottomLevelASBuffers.accelerationStructure = bottomLevelAS;
		bottomLevelASBuffers.scratch = scratch;
		bottomLevelASBuffers.ResultDataMaxSizeInBytes = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		return bottomLevelASBuffers;
	}

	AccelerationStructureBuffers AccelerationStructure::buildTopLevelAS(
		vector<AccelerationStructureBuffers>& bottomLevelAS, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
	{
		auto device = m_deviceResources->GetD3DDevice();
		auto commandList = m_deviceResources->GetCommandList();
		ComPtr<ID3D12Resource> scratch;
		ComPtr<ID3D12Resource> topLevelAS;

		// Get required sizes for an acceleration structure.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
		topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		topLevelInputs.Flags = buildFlags;
		topLevelInputs.NumDescs = 2;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
		m_device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
		ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

		AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

		// Allocate resources for acceleration structures.
		// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
		// Default heap is OK since the application doesn’t need CPU read/write access to them. 
		// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
		// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
		//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
		//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
		{
			D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
			AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
		}

		// Create instance descs for the bottom-level acceleration structures.
		vector<D3D12_GPU_VIRTUAL_ADDRESS> bottomLevelASaddresses{
			bottomLevelAS[0].accelerationStructure->GetGPUVirtualAddress(),
			bottomLevelAS[1].accelerationStructure->GetGPUVirtualAddress()
		};
		ComPtr<ID3D12Resource> instanceDescsResource = buildBottomLevelASInstanceDesc(bottomLevelASaddresses);

		// Top-level AS desc
		{
			topLevelBuildDesc.DestAccelerationStructureData = topLevelAS->GetGPUVirtualAddress();
			topLevelInputs.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
			topLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		}

		// Build acceleration structure.
		m_commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

		AccelerationStructureBuffers topLevelASBuffers;
		topLevelASBuffers.accelerationStructure = topLevelAS;
		topLevelASBuffers.instanceDesc = instanceDescsResource;
		topLevelASBuffers.scratch = scratch;
		topLevelASBuffers.ResultDataMaxSizeInBytes = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		return topLevelASBuffers;
	}

	// For now, each Node in the BLAS will have just one instance.
	ComPtr<ID3D12Resource> AccelerationStructure::buildBottomLevelASInstanceDesc(vector<D3D12_GPU_VIRTUAL_ADDRESS>& bottomLevelASaddresses)
	{
		ComPtr<ID3D12Resource> instanceDescsResource;

		vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;

		auto device = m_deviceResources->GetD3DDevice();

		for (int i = 0; i < bottomLevelASaddresses.size(); ++i)
		{
			auto& blas = bottomLevelASaddresses[i];
			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc;
			instanceDesc = {};
			instanceDesc.InstanceMask = 1;
			instanceDesc.InstanceContributionToHitGroupIndex = i * 2;
			instanceDesc.AccelerationStructure = blas;
			XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), m_blasTransforms[i]);

			instanceDescs.push_back(instanceDesc);
		}

		UINT64 bufferSize = static_cast<UINT64>(instanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		AllocateUploadBuffer(device, instanceDescs.data(), bufferSize, &instanceDescsResource, L"InstanceDescs");

		return instanceDescsResource;
	};
}