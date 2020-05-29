#pragma once

#include "StaticScene.h"
#include "ShaderTableEntry.h"
#include "RayTracingState.h"
#include <wrl/client.h>

namespace RtxEngine
{
	class ShaderTable
	{
	public:
		ShaderTable(const StaticScenePtr& scene, DeviceResourcesPtr& deviceResources, const RayTracingStatePtr& rayTracingState);
		/** Add a ray generation shader entry. */
		void addRayGen(const wstring& rayGenShader);
		/** Add a miss shader entry. */
		void addMiss(const string& rayId);
		/** Add a common entry. */
		void addCommonEntry(const ShaderTableEntry& entry);
		
		const ShaderTableEntriesPtr& getCommonEntries() const { return m_commonEntries; }
		
		void build();

	private:
		StaticScenePtr m_scene;
		DeviceResourcesPtr m_deviceResources;
		RayTracingStatePtr m_rayTracingState;
		
		wstring m_rayGenEntry;
		vector<string> m_missEntries;
		ShaderTableEntriesPtr m_commonEntries;

		ComPtr<ID3D12Resource> m_missShaderTable;
		UINT m_missShaderTableStrideInBytes;
		ComPtr<ID3D12Resource> m_hitGroupShaderTable;
		UINT m_hitGroupShaderTableStrideInBytes;
		ComPtr<ID3D12Resource> m_rayGenShaderTable;
	};

	using ShaderTablePtr = shared_ptr<ShaderTable>;
}