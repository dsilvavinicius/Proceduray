#pragma once
#include "StaticScene.h"
#include "ShaderTableEntry.h"

namespace RtxEngine
{
	class RayTracingState
	{
	public:
		RayTracingState(const StaticScenePtr& scene, const ShaderTableEntriesPtr& shaderTableEntries, const DxrDevicePtr& dxrDevice);
	private:
		void createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
		void createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);

		StaticScenePtr m_scene;
		ShaderTableEntriesPtr m_shaderTableEntries;
		DxrDevicePtr m_dxrDevice;
		DxrStatePtr m_dxrState;
	};
}