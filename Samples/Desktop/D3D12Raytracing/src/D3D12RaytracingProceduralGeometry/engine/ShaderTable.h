#pragma once

#include "Ray.h"
#include "Geometry.h"
#include "HitGroup.h"
#include "RootSignature.h"
#include <memory>

namespace RtxEngine
{
	class ShaderTable
	{
	public:
		ShaderTable(DeviceResourcesPtr& deviceResources) : m_deviceResources(deviceResources) {}
		/** Add a ray generation shader entry. */
		void addEntry(const string& rayGenShader);
		/** Add a miss shader entry. */
		void addEntry(const RayPtr& ray);
		/** Add a common entry. */
		void addEntry(const RayPtr& ray, const GeometryPtr& geometry, const HitGroupPtr& hitgroup, const RootSignaturePtr& rootSignature);
		void build();

	private:
		using Entry = tuple<RayPtr, GeometryPtr, HitGroupPtr, RootSignaturePtr>;
		string m_rayGenEntry;
		vector<RayPtr> m_missEntries;
		vector<Entry> m_commonEntries;
		DeviceResourcesPtr m_deviceResources;
	};

	using ShaderTablePtr = shared_ptr<ShaderTable>;
}