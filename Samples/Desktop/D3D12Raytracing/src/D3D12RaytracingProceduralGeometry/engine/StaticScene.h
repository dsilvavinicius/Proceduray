#pragma once

#include "Ray.h"
#include "Geometry.h"
#include "RootSignature.h"
#include "HitGroup.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace RtxEngine
{
	class StaticScene
	{
	public:
		// Add scene components.
		void addRay(const string& name, const RayPtr& ray) { m_rays[name] = ray; }
		void addGeometry(const string& name, const GeometryPtr& geometry) { m_geometry[name] = geometry; }
		void addGlobalSignature(const RootSignaturePtr& rootSignature) { m_globalSignature = rootSignature; }
		void addLocalSignature(const string& name, const RootSignaturePtr& rootSignature) { m_localSignatures[name] = rootSignature; }
		void addHitGroup(const string& name, const HitGroupPtr& hitGroup) { m_hitGroups[name] = hitGroup; }

		const RayMap& getRays() const { return m_rays; }
		const GeometryMap& getGeometry() const { return m_geometry; }
		const HitGroupMap& getHitGroups() const { return m_hitGroups; }
		RootSignature& getGlobalSignature() const { return *m_globalSignature; }
		const RootSignatureMap& getLocalSignatures() const { return m_localSignatures; }
		UINT getMaxRecursion() const { return m_maxRecursion; }

	private:
		// Scene entities.
		RayMap m_rays;
		GeometryMap m_geometry;
		
		// Root signatures.
		RootSignaturePtr m_globalSignature;
		RootSignatureMap m_localSignatures;
		
		// Hitgroups.
		HitGroupMap m_hitGroups;

		UINT m_maxRecursion = 3;
	};

	using StaticScenePtr = shared_ptr<StaticScene>;
}