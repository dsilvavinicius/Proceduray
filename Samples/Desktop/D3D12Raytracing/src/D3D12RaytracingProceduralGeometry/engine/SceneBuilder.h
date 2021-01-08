#pragma once

#include "Ray.h"
#include "HitGroup.h"
#include "Geometry.h"
#include "RootSignature.h"

namespace RtxEngine
{
	class SceneBuilder
	{
	public:
		virtual ~SceneBuilder() = 0 {};
		const vector<RayPtr>& getRays() const { return m_rays; }
		const vector<HitGroupPtr>& getHitGroups() const { return m_hitgroups; }
		const vector<GeometryPtr>& getGeometries() const { return m_geometries; }
		const vector<RootSignaturePtr>& getSignatures() const { return m_signatures; }
	private:
		vector<RayPtr> m_rays;
		vector<HitGroupPtr> m_hitgroups;
		vector<GeometryPtr> m_geometries;
		vector<RootSignaturePtr> m_signatures;
	};
}