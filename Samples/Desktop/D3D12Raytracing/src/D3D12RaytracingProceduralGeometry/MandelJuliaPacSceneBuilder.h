#pragma once

#include "engine/SceneDefines.h"
#include "engine/AccelerationStructure.h"
#include "engine/RayTracingState.h"
#include "engine/ShaderTable.h"

using namespace RtxEngine;

class MandelJuliaPacSceneBuilder
{
public:
	MandelJuliaPacSceneBuilder();
	void init(DeviceResourcesPtr deviceResources);
	void build(DxrDevicePtr device, DeviceResourcesPtr deviceResources, DxrCommandListPtr commandList, DescriptorHeapPtr descriptorHeap,
		DescriptorHeap::DescriptorHandles& descriptorHandles);

	shared_ptr<ConstantBuffer<SceneConstantBuffer>> getSceneCB() { return m_sceneCB; }
	shared_ptr<StructuredBuffer<InstanceBuffer>> getInstanceBuffer() { return m_instanceBuffer; }
	PrimitiveConstantBuffer& getPlaneMaterialCB() { return m_planeMaterialCB; }
	PrimitiveConstantBuffer* getAabbMaterialCB() { return m_aabbMaterialCB; }
	StaticScenePtr getScene() { return m_scene; }
	ShaderTablePtr getShaderTable() { return m_shaderTable; }
	void release();

private:
	void CreateConstantBuffers(DeviceResourcesPtr deviceResources);
	void CreateInstanceBuffer(DeviceResourcesPtr deviceResource);
	void CreateRays();
	void CreateHitGroups();
	void BuildGeometry(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap);
	void BuildInstancedProcedural(DeviceResourcesPtr deviceResources);
	void BuildInstancedParallelepipeds(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap);
	void BuildPlaneGeometry(const XMFLOAT3& width, DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap);
	void CreateRootSignatures(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap, DescriptorHeap::DescriptorHandles& descriptorHandles);
	void CreateAccelerationStructure(DxrDevicePtr device, DeviceResourcesPtr deviceResources, DxrCommandListPtr commandList);
	void CreateShaderTablesEntries(DeviceResourcesPtr deviceResources);

	StaticScenePtr m_scene;

	AccelerationStructurePtr m_accelerationStruct = nullptr;
	ShaderTablePtr m_shaderTable = nullptr;

	// Global Root Signature components.
	shared_ptr<ConstantBuffer<SceneConstantBuffer>> m_sceneCB;
	shared_ptr<StructuredBuffer<InstanceBuffer>> m_instanceBuffer;

	// Local Root Signature Constants
	PrimitiveConstantBuffer m_planeMaterialCB;
	PrimitiveConstantBuffer m_aabbMaterialCB[IntersectionShaderType::TotalPrimitiveCount];
};