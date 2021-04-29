#pragma once

#include "DeviceHelper.h"
#include "DescriptorHeap.h"
#include "RaytracingState.h"

namespace RtxEngine
{
	/** DXR low level and mid level abstraction grouping. */
	struct DxrInternal
	{
		// Lower level abstractions
		DxrDevicePtr device = nullptr;
		DeviceResourcesPtr deviceResources = nullptr;
		DxrCommandListPtr commandList = nullptr;
		
		// Mid level abstractions
		DescriptorHeapPtr descriptorHeap = nullptr;
		RayTracingStatePtr rayTracingState = nullptr;

		// Ray tracing output.
		ComPtr<ID3D12Resource> raytracingOutput = nullptr;
		DescriptorHeap::DescriptorHandles outputHandler;
	};

	using DxrInternalPtr = shared_ptr<DxrInternal>;
}