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
		void addRay(const RayPtr& ray) { m_rays[ray->getName()] = ray; }
		void addGeometry(const GeometryPtr& geometry);
		void addGlobalSignature(const RootSignaturePtr& rootSignature) { m_globalSignature = rootSignature; }
		void addLocalSignature(const RootSignaturePtr& rootSignature) { m_localSignatures[rootSignature->getName()] = rootSignature; }
		void addHitGroup(const HitGroupPtr& hitGroup);

		const RayMap& getRays() const { return m_rays; }
		const GeometryVector& getGeometry() const { return m_geometry; }
		const GeometryMap& getGeometryMap() const { return m_geometryMap; }
		const HitGroupVector& getHitGroups() const { return m_hitGroups; }
		const HitGroupMap& getHitGroupMap() const { return m_hitGroupMap; }
		RootSignature& getGlobalSignature() const { return *m_globalSignature; }
		const RootSignatureMap& getLocalSignatures() const { return m_localSignatures; }
		UINT getMaxRecursion() const { return m_maxRecursion; }

	private:
		// Scene entities.
		RayMap m_rays;
		GeometryVector m_geometry;
		GeometryMap m_geometryMap;
		
		// Root signatures.
		RootSignaturePtr m_globalSignature;
		RootSignatureMap m_localSignatures;
		
		// Hitgroups.
		HitGroupVector m_hitGroups;
		HitGroupMap m_hitGroupMap;

		UINT m_maxRecursion = 3;
	};

	using StaticScenePtr = shared_ptr<StaticScene>;
}