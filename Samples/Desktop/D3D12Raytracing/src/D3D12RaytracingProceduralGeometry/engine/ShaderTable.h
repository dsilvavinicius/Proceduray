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
		void addEntry(const RayPtr& ray, const GeometryPtr& geometry, const HitGroupPtr& hitgroup, const RootSignature& rootSignature);
		void build();

	private:
		using Entry = tuple<RayPtr, GeometryPtr, HitGroupPtr, RootSignature>;
		vector<Entry> m_entries;
	};

	using ShaderTablePtr = shared_ptr<ShaderTable>;
}