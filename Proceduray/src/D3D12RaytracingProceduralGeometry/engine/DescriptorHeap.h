#pragma once

#include "DeviceHelper.h"

namespace RtxEngine
{
	class DescriptorHeap
	{
	public:
		struct DescriptorHandles
		{
			D3D12_CPU_DESCRIPTOR_HANDLE cpu;
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;
			UINT descriptorIndex;
			UINT baseHandleIndex;
		};

		DescriptorHeap(DeviceResourcesPtr& deviceResources, UINT numDescriptors);
		~DescriptorHeap();

		DescriptorHandles allocateDescriptor(UINT descriptorIndexToUse = UINT_MAX);
		DxrDescriptorHeapPtr getDxrDescriptorHeap() { return m_descriptorHeap; }
		UINT getDescriptoSize() { return m_descriptorSize; }
		void bind();
	private:
		// Input.
		DeviceResourcesPtr m_deviceResources;

		// Output.
		DxrDescriptorHeapPtr m_descriptorHeap;
		UINT m_descriptorSize = 0u;
		UINT m_descriptorsAllocated = 0u;
	};

	using DescriptorHeapPtr = shared_ptr<DescriptorHeap>;
}