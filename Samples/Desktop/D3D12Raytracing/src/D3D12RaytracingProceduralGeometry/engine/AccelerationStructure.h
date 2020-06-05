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
		AccelerationStructure(const StaticScenePtr& scene, DxrDevicePtr& device, DxrCommandListPtr& commandList, DeviceResourcesPtr& deviceResources);
		~AccelerationStructure();
		
		ComPtr<ID3D12Resource> getBuilded() { return m_topLevelAS; }
	private:
		void build();
		vector<D3D12_RAYTRACING_GEOMETRY_DESC> buildGeometryDescsForBottomLevelAS() const;
		
		AccelerationStructureBuffers buildBottomLevelAS(
			const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);

		AccelerationStructureBuffers buildTopLevelAS(
			AccelerationStructureBuffers bottomLevelAS,
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE
		);
		
		template <class InstanceDescType, class BLASPtrType>
		void buildBotomLevelASInstanceDesc(BLASPtrType* bottomLevelASaddress, ComPtr<ID3D12Resource>* instanceDescsResource);

		DxrDevicePtr m_device;
		DxrCommandListPtr m_commandList;
		DeviceResourcesPtr m_deviceResources;
		StaticScenePtr m_scene;

		ComPtr<ID3D12Resource> m_bottomLevelAS;
		ComPtr<ID3D12Resource> m_topLevelAS;
	};

	using AccelerationStructurePtr = shared_ptr<AccelerationStructure>;
}