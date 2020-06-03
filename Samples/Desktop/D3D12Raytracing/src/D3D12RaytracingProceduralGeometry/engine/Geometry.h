#pragma once

#include "ShaderCompat.h"
#include "DXSampleHelper.h"
#include "DescriptorHeap.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace RtxEngine
{
	using namespace std;

	class Geometry
	{
	public:
		enum Type
		{
			Triangles = 0,
			Procedural
		};

		Geometry(const DescriptorHeapPtr& descriptorHeap, const pair<XMFLOAT3, XMFLOAT3>& aabb, const XMMATRIX& transform);
		Geometry(const DescriptorHeapPtr& descriptorHeap, const vector<XMFLOAT3>& vertices, const vector<UINT> indices, const XMMATRIX& transform);
		
		const D3DBuffer& getVertexBuffer() const { return m_vertexBuffer; }
		const D3DBuffer& getIndexBuffer() const { return m_indexBuffer; }
		const Type getType() const { return m_type; }

	private:
		Type m_type;
		D3DBuffer m_vertexBuffer;
		D3DBuffer m_indexBuffer;
	};

	using GeometryPtr = shared_ptr<Geometry>;
	using GeometryMap = unordered_map<string, GeometryPtr>;
}

