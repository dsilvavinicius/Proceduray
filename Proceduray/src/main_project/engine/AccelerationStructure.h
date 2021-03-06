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
		AccelerationStructure(const StaticScenePtr& scene, DxrDevicePtr& device, DxrCommandListPtr& commandList,
			DeviceResourcesPtr& deviceResources);
		~AccelerationStructure();
		
		ComPtr<ID3D12Resource> getBuilded() { return m_topLevelAS; }
	private:
		using BlasDescriptors = vector<D3D12_RAYTRACING_GEOMETRY_DESC>;
		using BlasInstances = Geometry::InstancesPtr;
		
		struct BlasInput
		{
			BlasDescriptors descriptors;
			BlasInstances instances;
		};

		void build();
		vector<BlasInput> buildGeometryDescsForBottomLevelAS() const;
		
		AccelerationStructureBuffers buildBottomLevelAS(
			const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);

		AccelerationStructureBuffers buildTopLevelAS(
			vector<AccelerationStructureBuffers>& bottomLevelAS,
			vector<BlasInput>& blasInput,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);
		
		ComPtr<ID3D12Resource> buildBottomLevelASInstanceDesc(vector<D3D12_GPU_VIRTUAL_ADDRESS>& bottomLevelASaddress,
			vector<BlasInput>& blasInput);

		DxrDevicePtr m_device;
		DxrCommandListPtr m_commandList;
		DeviceResourcesPtr m_deviceResources;
		StaticScenePtr m_scene;

		vector<ComPtr<ID3D12Resource>> m_bottomLevelAS;
		ComPtr<ID3D12Resource> m_topLevelAS;
	};

	using AccelerationStructurePtr = shared_ptr<AccelerationStructure>;
}