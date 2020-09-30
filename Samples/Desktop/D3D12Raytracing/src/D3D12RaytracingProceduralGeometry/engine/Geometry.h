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

		using Instance = XMMATRIX;
		using Instances = vector<Instance>;
		using InstancesPtr = shared_ptr<Instances>;

		/** Create procedural geometry.*/
		Geometry(D3D12_RAYTRACING_AABB& aabb, DeviceResources& deviceResources, const Instances& instances);
		
		/** Create mesh geometry. If a descriptor heap is passed, then the index and vertex buffer descriptors are pushed to it.*/
		Geometry(vector<Vertex>& vertices, vector<Index>& indices, DeviceResources& deviceResources, DescriptorHeap& descriptorHeap,
			const Instances& instances);
		
		~Geometry();

		const D3DBuffer& getVertexBuffer() const { return m_vertexBuffer; }
		const D3DBuffer& getIndexBuffer() const { return m_indexBuffer; }
		const InstancesPtr getInstances() const { return m_instances; }
		const Type getType() const { return m_type; }

	private:
		UINT Geometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize, DeviceResources& deviceResources, DescriptorHeap& descriptorHeap);

		Type m_type;
		InstancesPtr m_instances;
		D3DBuffer m_vertexBuffer;
		D3DBuffer m_indexBuffer;
	};

	using GeometryPtr = shared_ptr<Geometry>;
	using GeometryMap = unordered_map<string, GeometryPtr>;
	using GeometryVector = vector<GeometryPtr>;
}

