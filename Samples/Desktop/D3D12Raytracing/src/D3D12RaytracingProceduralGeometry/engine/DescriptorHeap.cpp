#include "../stdafx.h"
#include "DescriptorHeap.h"

namespace RtxEngine
{
	DescriptorHeap::DescriptorHeap(DeviceResourcesPtr& deviceResources)
	{
		auto device = deviceResources->GetD3DDevice();

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = 3;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;
		device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
		NAME_D3D12_OBJECT(m_descriptorHeap);

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Allocate a descriptor and return its index.
	// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
	UINT DescriptorHeap::allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
	{
		auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
		{
			ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
			descriptorIndexToUse = m_descriptorsAllocated++;
		}
		*cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
		return descriptorIndexToUse;
	}
}