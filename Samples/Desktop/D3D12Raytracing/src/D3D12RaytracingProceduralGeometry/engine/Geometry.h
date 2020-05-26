#pragma once

#include "../util/HlslCompat.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;

	/*enum GeometryType
	{
		Triangles = 0,
		Procedural,
		Count,
	};*/

	class Geometry
	{
	public:
		Geometry(pair<float3> aabb, const float4x4& transform);

	private:
		//GeometryType m_type;
	};

	using GeometryPtr = shared_ptr<Geometry>;
	using GeometryMap = unordered_map<string, GeometryPtr>;
}

