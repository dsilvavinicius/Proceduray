#include "../stdafx.h"
#include "RayTracingState.h"
#include "ShaderCompatUtils.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "../DirectXRaytracingHelper.h"

namespace RtxEngine
{
	RayTracingState::RayTracingState(const StaticScenePtr& scene, const ShaderTableEntriesPtr& shaderTableEntries, const DxrDevicePtr& dxrDevice,
		const DxrCommandListPtr& dxrCommandList, const DeviceResourcesPtr& deviceResources, const DescriptorHeapPtr& descriptorHeap)
		: m_scene(scene),
		m_shaderTableEntries(shaderTableEntries),
		m_dxrDevice(dxrDevice),
		m_dxrCommandList(dxrCommandList),
		m_deviceResources(deviceResources),
		m_descriptorHeap(descriptorHeap)
	{
		auto device = m_deviceResources->GetD3DDevice();
		auto commandQueue = m_deviceResources->GetCommandQueue();
		m_gpuTimer.RestoreDevice(device, commandQueue, m_deviceResources->GetBackBufferCount());

		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		// DXIL library
		createDxilLibrarySubobject(&raytracingPipeline);

		// Hit groups
		createHitGroupSubobjects(&raytracingPipeline);

		// Shader config
		// Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
		auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(ShaderCompatUtils::getMaxPayloadSize(), ShaderCompatUtils::getMaxAttribStructSize());

		// Local root signature and shader association
		// This is a root signature that enables a shader to have unique arguments that come from shader tables.
		createLocalRootSignatureSubobjects(&raytracingPipeline);

		// Global root signature
		// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
		auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(m_scene->getGlobalSignature().getBuilded().Get());

		// Pipeline config
		// Defines the maximum TraceRay() recursion depth.
		auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		// PERFOMANCE TIP: Set max recursion depth as low as needed
		// as drivers may apply optimization strategies for low recursion depths.
		UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
		pipelineConfig->Config(maxRecursionDepth);

		PrintStateObjectDesc(raytracingPipeline);

		// Create the state object.
		ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrState)), L"Couldn't create DirectX Raytracing state object.\n");
	}

	RayTracingState::~RayTracingState()
	{
		m_gpuTimer.ReleaseDevice();
		m_dxrState.Reset();
	}

	// DXIL library
	// This contains the shaders and their entrypoints for the state object.
	// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
	void RayTracingState::createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
		lib->SetDXILLibrary(&libdxil);
		// Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
	}

	// Hit groups
	// A hit group specifies closest hit, any hit and intersection shaders 
	// to be executed when a ray intersects the geometry.
	void RayTracingState::createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		auto hitGroups = m_scene->getHitGroups();

		for (auto hitGroupEntry : hitGroups)
		{
			auto hitGroup = hitGroupEntry.second;
			auto hitGroupSO = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			if (!hitGroup->intersection.empty())
			{
				hitGroupSO->SetIntersectionShaderImport(hitGroup->intersection.c_str());
				hitGroupSO->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
			}
			
			if (!hitGroup->anyHit.empty())
			{
				hitGroupSO->SetAnyHitShaderImport(hitGroup->anyHit.c_str());
			}


			if (!hitGroup->closestHit.empty())
			{
				hitGroupSO->SetClosestHitShaderImport(hitGroup->closestHit.c_str());
			}

			hitGroupSO->SetHitGroupExport(hitGroup->name.c_str());
		}
	}

	// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	void RayTracingState::createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		unordered_map<string, vector<LPCWSTR>> rootToHit;
		for(const auto& entry : *m_shaderTableEntries)
		{
			const auto hitGroup = m_scene->getHitGroups().at(entry.hitGroupId);
			rootToHit[entry.rootSignatureId].push_back(hitGroup->name.c_str());
		}

		for (const auto& rootToHitEntry : rootToHit)
		{
			auto rootParametersId = rootToHitEntry.first;
			auto hitGroupIds = rootToHitEntry.second;

			const auto& rootSignature = m_scene->getLocalSignatures().at(rootParametersId);
			auto rootSignatureSO = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			rootSignatureSO->SetRootSignature(rootSignature->getBuilded().Get());

			// Shader association
			auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*rootSignatureSO);
			for (const auto& hitGroupId : hitGroupIds)
			{
				rootSignatureAssociation->AddExports(hitGroupIds.data(), hitGroupIds.size());
			}
		}
	}

	void RayTracingState::doRayTracing(const BuildedShaderTablePtr& shaderTable, UINT width, UINT height)
	{
		auto commandList = m_deviceResources->GetCommandList();
		auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

		m_gpuTimer.BeginFrame(commandList);

		auto DispatchRays = [&](auto* raytracingCommandList, auto* stateObject, auto* dispatchDesc)
		{
			dispatchDesc->HitGroupTable.StartAddress = shaderTable->hitGroupShaderTable->GetGPUVirtualAddress();
			dispatchDesc->HitGroupTable.SizeInBytes = shaderTable->hitGroupShaderTable->GetDesc().Width;
			dispatchDesc->HitGroupTable.StrideInBytes = shaderTable->hitGroupShaderTableStrideInBytes;
			dispatchDesc->MissShaderTable.StartAddress = shaderTable->missShaderTable->GetGPUVirtualAddress();
			dispatchDesc->MissShaderTable.SizeInBytes = shaderTable->missShaderTable->GetDesc().Width;
			dispatchDesc->MissShaderTable.StrideInBytes = shaderTable->missShaderTableStrideInBytes;
			dispatchDesc->RayGenerationShaderRecord.StartAddress = shaderTable->rayGenShaderTable->GetGPUVirtualAddress();
			dispatchDesc->RayGenerationShaderRecord.SizeInBytes = shaderTable->rayGenShaderTable->GetDesc().Width;
			dispatchDesc->Width = width;
			dispatchDesc->Height = height;
			dispatchDesc->Depth = 1;
			raytracingCommandList->SetPipelineState1(stateObject);

			m_gpuTimer.Start(commandList);
			raytracingCommandList->DispatchRays(dispatchDesc);
			m_gpuTimer.Stop(commandList);
		};

		auto globalSignature = m_scene->getGlobalSignature();
		globalSignature.prepareToRender();

		// Bind the heaps, acceleration structure and dispatch rays.  
		D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
		DispatchRays(m_dxrCommandList.Get(), m_dxrState.Get(), &dispatchDesc);

		m_gpuTimer.EndFrame(commandList);
	}
}