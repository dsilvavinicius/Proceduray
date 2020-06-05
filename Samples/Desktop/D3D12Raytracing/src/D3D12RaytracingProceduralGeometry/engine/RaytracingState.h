#pragma once

#include "StaticScene.h"
#include "ShaderTableShared.h"
#include "PerformanceTimers.h"

namespace RtxEngine
{
	class RayTracingState
	{
	public:
		RayTracingState(const StaticScenePtr& scene, const ShaderTableEntriesPtr& shaderTableEntries, const DxrDevicePtr& dxrDevice, const DxrCommandListPtr& dxrCommandList,
			const DeviceResourcesPtr& deviceResources, const DescriptorHeapPtr& descriptorHeap);
		~RayTracingState();

		DxrStatePtr& getBuilded() { return m_dxrState; }

		const DX::GPUTimer& getGpuTimer() { return m_gpuTimer; }

		void doRayTracing(const BuildedShaderTablePtr& shaderTable, UINT width, UINT height);

	private:
		void createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);

		// Input.
		StaticScenePtr m_scene;
		ShaderTableEntriesPtr m_shaderTableEntries;
		DxrDevicePtr m_dxrDevice;
		DxrCommandListPtr m_dxrCommandList;
		DeviceResourcesPtr m_deviceResources;
		DescriptorHeapPtr m_descriptorHeap;

		// Generated.
		DX::GPUTimer m_gpuTimer;

		// Output.
		DxrStatePtr m_dxrState;
	};

	using RayTracingStatePtr = shared_ptr<RayTracingState>;
}