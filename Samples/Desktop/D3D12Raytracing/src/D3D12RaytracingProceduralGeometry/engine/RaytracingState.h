#pragma once

#include "StaticScene.h"
#include "ShaderTableShared.h"
#include "PerformanceTimers.h"

namespace RtxEngine
{
	class RayTracingState
	{
	public:
		RayTracingState(const StaticScenePtr& scene, const ShaderTableEntriesPtr& shaderTableEntries, const DxrDevicePtr& dxrDevice,
			const DeviceResourcesPtr& deviceResources, const DescriptorHeapPtr& descriptorHeap);
		~RayTracingState();

		DxrStatePtr& getBuilded() { return m_dxrState; }
		void doRayTracing(const BuildedShaderTablePtr& shaderTable);

	private:
		void createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);

		StaticScenePtr m_scene;
		ShaderTableEntriesPtr m_shaderTableEntries;
		DxrDevicePtr m_dxrDevice;
		DxrStatePtr m_dxrState;
		DeviceResourcesPtr m_deviceResources;
		DescriptorHeapPtr m_descriptorHeap;

		DX::GPUTimer m_gpuTimer;
	};

	using RayTracingStatePtr = shared_ptr<RayTracingState>;
}