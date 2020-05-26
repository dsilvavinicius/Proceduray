#include "AccelerationStructure.h"
#include "../DirectXRaytracingHelper.h"

namespace RtxEngine
{
	AccelerationStructure::AccelerationStructure(DevicePtr& device, const StaticScenePtr& scene)
		: m_device(device),
		m_scene(scene)
	{
		build();
	}

	// Build acceleration structure needed for raytracing.
	void AccelerationStructure::build()
	{
		auto device = m_device->GetD3DDevice();
		auto commandList = m_device->GetCommandList();
		auto commandQueue = m_device->GetCommandQueue();
		auto commandAllocator = m_device->GetCommandAllocator();

		// Reset the command list for the acceleration structure construction.
		commandList->Reset(commandAllocator, nullptr);

		// Build bottom-level AS.
		auto geometries = m_scene->getGeometry();
		vector<AccelerationStructureBuffers> bottomLevelAS;
		vector<vector<D3D12_RAYTRACING_GEOMETRY_DESC>> geometryDescs;
		{
			// CONTINUE HERE!
			BuildGeometryDescsForBottomLevelAS(geometryDescs);

			// Build all bottom-level AS.
			for (UINT i = 0; i < BottomLevelASType::Count; i++)
			{
				bottomLevelAS[i] = BuildBottomLevelAS(geometryDescs[i]);
			}
		}

		// Batch all resource barriers for bottom-level AS builds.
		D3D12_RESOURCE_BARRIER resourceBarriers[BottomLevelASType::Count];
		for (UINT i = 0; i < BottomLevelASType::Count; i++)
		{
			resourceBarriers[i] = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS[i].accelerationStructure.Get());
		}
		commandList->ResourceBarrier(BottomLevelASType::Count, resourceBarriers);

		// Build top-level AS.
		AccelerationStructureBuffers topLevelAS = BuildTopLevelAS(bottomLevelAS);

		// Kick off acceleration structure construction.
		m_deviceResources->ExecuteCommandList();

		// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
		m_deviceResources->WaitForGpu();

		// Store the AS buffers. The rest of the buffers will be released once we exit the function.
		for (UINT i = 0; i < BottomLevelASType::Count; i++)
		{
			m_bottomLevelAS[i] = bottomLevelAS[i].accelerationStructure;
		}
		m_topLevelAS = topLevelAS.accelerationStructure;
	}
}