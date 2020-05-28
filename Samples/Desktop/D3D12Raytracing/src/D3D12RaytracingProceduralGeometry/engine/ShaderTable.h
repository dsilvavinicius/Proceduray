#pragma once

#include "StaticScene.h"
#include "ShaderTableEntry.h"

namespace RtxEngine
{
	class ShaderTable
	{
	public:
		ShaderTable(const StaticScenePtr& scene, DeviceResourcesPtr& deviceResources);
		/** Add a ray generation shader entry. */
		void addRayGen(const string& rayGenShader);
		/** Add a miss shader entry. */
		void addMiss(const string& rayId);
		/** Add a common entry. */
		void addCommonEntry(const ShaderTableEntry& entry);
		
		const ShaderTableEntriesPtr& getCommonEntries() const { return m_commonEntries; }
		
		void build();

	private:
		StaticScenePtr m_scene;
		DeviceResourcesPtr m_deviceResources;
		string m_rayGenEntry;
		vector<string> m_missEntries;
		ShaderTableEntriesPtr m_commonEntries;
	};

	using ShaderTablePtr = shared_ptr<ShaderTable>;
}