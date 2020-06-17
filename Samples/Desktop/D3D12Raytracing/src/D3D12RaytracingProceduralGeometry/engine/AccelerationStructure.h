#pragma once

#include "DeviceHelper.h"
#include "StaticScene.h"

namespace RtxEngine
{
	class AccelerationStructure
	{
		/** Acceleration structure for ray tracing. For now, it consists of one bottom level acceleration structure (BLAS) and one top
		* level acceleration structure (TLAS). In other words, the acceleration structure is just one instance of the BLAS.*/
	public:
		AccelerationStructure(const StaticScenePtr& scene, DxrDevicePtr& device, DxrCommandListPtr& commandList, DeviceResourcesPtr& deviceResources,
			const XMMATRIX& triangleTransform, const XMMATRIX& proceduralTransform);
		~AccelerationStructure();
		
		ComPtr<ID3D12Resource> getBuilded() { return m_topLevelAS; }
	private:
		using BlasDescriptors = vector<vector<D3D12_RAYTRACING_GEOMETRY_DESC>>;

		void build();
		BlasDescriptors buildGeometryDescsForBottomLevelAS() const;
		
		AccelerationStructureBuffers buildBottomLevelAS(
			const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);

		AccelerationStructureBuffers buildTopLevelAS(
			vector<AccelerationStructureBuffers>& bottomLevelAS,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);
		
		ComPtr<ID3D12Resource> buildBottomLevelASInstanceDesc(vector<D3D12_GPU_VIRTUAL_ADDRESS>& bottomLevelASaddress);

		DxrDevicePtr m_device;
		DxrCommandListPtr m_commandList;
		DeviceResourcesPtr m_deviceResources;
		StaticScenePtr m_scene;

		vector<XMMATRIX> m_blasTransforms;

		vector<ComPtr<ID3D12Resource>> m_bottomLevelAS;
		ComPtr<ID3D12Resource> m_topLevelAS;
	};

	using AccelerationStructurePtr = shared_ptr<AccelerationStructure>;
}