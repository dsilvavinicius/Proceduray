#pragma once

#include "Ray.h"
#include "Geometry.h"
#include "RootSignature.h"
#include "HitGroup.h"
#include "ShaderTable.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace RtxEngine
{
	class StaticScene
	{
	public:
		using ShaderTableEntry = tuple<RayPtr, GeometryPtr, HitGroupPtr, RootSignaturePtr>;
		using ShaderTableMap = unordered_map<string, ShaderTableEntry>;

		// Add scene components.
		void addRay(const string& name, const RayPtr& ray);
		void addGeometry(const string& name, const GeometryPtr& geometry);
		void addGlobalSignature(const RootSignaturePtr& rootSignature);
		void addLocalSignature(const string& name, const RootSignaturePtr& rootSignature);
		void addHitGroup(const string& name, const HitGroupPtr& hitGroup);
		void addShaderTableEntry(const string& ray, const string& geometry, const string& hitgroup, const string& rootSignature);

		const GeometryMap& getGeometry() const { return m_geometry; }
		const HitGroupMap& getHitGroups() const { return m_hitGroups; }
		const RootSignatureMap& getLocalSignatures() const { return m_localSignatures; }

		// Build the scene.
		void build();

	private:
		void createShaderTable();

		// Scene entities.
		RayMap m_rays;
		GeometryMap m_geometry;
		
		// Root signatures.
		RootSignaturePtr m_globalSignature;
		RootSignatureMap m_localSignatures;
		
		// Hitgroups.
		HitGroupMap m_hitGroups;

		// Shader table.
		ShaderTableMap m_shaderTableMap;
		ShaderTablePtr m_shaderTable;
	};

	using StaticScenePtr = shared_ptr<StaticScene>;
}