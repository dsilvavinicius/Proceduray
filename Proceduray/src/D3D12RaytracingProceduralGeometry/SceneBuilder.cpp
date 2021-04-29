#include "SceneBuilder.h"

namespace RtxEngine
{
	SceneBuilder::SceneBuilder(
		std::shared_ptr<DX::DeviceResources> deviceResources,
		DescriptorHeapPtr descriptorHeap,
		ComPtr<ID3D12Device5> dxrDevice,
		ComPtr<ID3D12GraphicsCommandList5> dxrCommandList
	) : m_deviceResources(deviceResources),
		m_descriptorHeap(descriptorHeap),
		m_dxrDevice(dxrDevice),
		m_dxrCommandList(dxrCommandList)
	{}

	SceneBuilder::BuildedScene SceneBuilder::build()
	{
		CreateSceneConstantBuffer();
		CreateRays();
		CreateHitGroups();
		BuildGeometry();
		CreateAccelerationStructure();
		CreateInstanceBuffer();
		CreateRootSignatures();
		CreateShaderTablesEntries();

		return BuildedScene{ m_sceneCB, m_scene, m_instanceBuffer, m_accelerationStruct, m_shaderTable };
	}

	void SceneBuilder::CreateAccelerationStructure()
	{
		m_accelerationStruct = make_shared<AccelerationStructure>(m_scene, m_dxrDevice, m_dxrCommandList, m_deviceResources);
	}
}