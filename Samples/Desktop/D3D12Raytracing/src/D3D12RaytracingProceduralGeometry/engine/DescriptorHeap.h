#pragma once

#include "DeviceHelper.h"

namespace RtxEngine
{
	class DescriptorHeap
	{
	public:
		DescriptorHeap(DeviceResourcesPtr& deviceResources);
		UINT allocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse);
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