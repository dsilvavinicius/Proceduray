#include "../stdafx.h"
#include "../DirectXRaytracingHelper.h"
#include "Geometry.h"

namespace RtxEngine
{
	Geometry::Geometry(D3D12_RAYTRACING_AABB& aabb, DeviceResources& deviceResources, const XMMATRIX& transform)
		: m_transform(transform),
		m_type(Procedural)
	{
		auto device = deviceResources.GetD3DDevice();
		AllocateUploadBuffer(device, &aabb, sizeof(D3D12_RAYTRACING_AABB), &m_vertexBuffer.resource);
	}

	Geometry::Geometry(Vertex vertices[], Index indices[], DeviceResources& deviceResources, DescriptorHeap& descriptorHeap, const XMMATRIX& transform = XMMatrixIdentity())
		: m_transform(transform),
		m_type(Triangles)
	{
		auto device = deviceResources.GetD3DDevice();
		AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
		AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

		// Vertex buffer is passed to the shader along with index buffer as a descriptor range.
		UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, 0, deviceResources, descriptorHeap);
		UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, sizeof(vertices), sizeof(vertices[0]), deviceResources, descriptorHeap);
		ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
	}

    // Create a SRV for a buffer.
    UINT Geometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize, DeviceResources& deviceResources, DescriptorHeap& descriptorHeap)
    {
        auto device = deviceResources.GetD3DDevice();

        // SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = numElements;
        if (elementSize == 0)
        {
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
            srvDesc.Buffer.StructureByteStride = 0;
        }
        else
        {
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            srvDesc.Buffer.StructureByteStride = elementSize;
        }

        auto descriptorHandles = descriptorHeap.allocateDescriptor();
        buffer->cpuDescriptorHandle = descriptorHandles.cpu;
        buffer->gpuDescriptorHandle = descriptorHandles.gpu;
        device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
        return descriptorHandles.descriptorIndex;
    }
}
