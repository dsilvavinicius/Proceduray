#include "../stdafx.h"
#include "DescriptorHeap.h"

namespace RtxEngine
{
	DescriptorHeap::DescriptorHeap(DeviceResourcesPtr& deviceResources, UINT numDescriptors)
		: m_deviceResources(deviceResources)
	{
		auto device = m_deviceResources->GetD3DDevice();

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;
		device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
		NAME_D3D12_OBJECT(m_descriptorHeap);

		m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	DescriptorHeap::~DescriptorHeap()
	{
		m_descriptorHeap.Reset();
		m_descriptorsAllocated = 0;
	}

	// Allocate a descriptor and return its index.
	// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
	DescriptorHeap::DescriptorHandles DescriptorHeap::allocateDescriptor(UINT descriptorIndexToUse)
	{
		auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
		{
			ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!");
			descriptorIndexToUse = m_descriptorsAllocated++;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptor(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptor(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndexToUse, m_descriptorSize);
		return DescriptorHandles{ cpuDescriptor, gpuDescriptor, descriptorIndexToUse };
	}

	void DescriptorHeap::bind()
	{
		auto commandList = m_deviceResources->GetCommandList();
		commandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
	}
}