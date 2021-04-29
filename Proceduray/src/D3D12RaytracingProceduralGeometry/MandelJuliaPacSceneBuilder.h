#pragma once

#include "engine/SceneDefines.h"
#include "engine/AccelerationStructure.h"
#include "engine/DxrInternal.h"
#include "engine/ShaderTable.h"

using namespace RtxEngine;

class MandelJuliaPacSceneBuilder
{
public:
	MandelJuliaPacSceneBuilder();
	void init(DxrInternalPtr dxr);
	void build();

	shared_ptr<ConstantBuffer<SceneConstantBuffer>> getSceneCB() { return m_sceneCB; }
	shared_ptr<StructuredBuffer<InstanceBuffer>> getInstanceBuffer() { return m_instanceBuffer; }
	PrimitiveConstantBuffer& getPlaneMaterialCB() { return m_planeMaterialCB; }
	PrimitiveConstantBuffer* getAabbMaterialCB() { return m_aabbMaterialCB; }
	StaticScenePtr getScene() { return m_scene; }
	ShaderTablePtr getShaderTable() { return m_shaderTable; }
	void release();

private:
	void BuildConstantBuffers();
	void BuildInstanceBuffer();
	void BuildRays();
	void BuildHitGroups();
	void BuildGeometry();
	void BuildInstancedProcedural();
	void BuildInstancedParallelepipeds();
	void BuildPlaneGeometry(const XMFLOAT3& width);
	void BuildRootSignatures();
	void BuildAccelerationStructure();
	void BuildShaderTablesEntries();

	DxrInternalPtr m_dxr = nullptr;

	StaticScenePtr m_scene;

	AccelerationStructurePtr m_accelerationStruct = nullptr;
	ShaderTablePtr m_shaderTable = nullptr;

	// Global Root Signature components.
	shared_ptr<ConstantBuffer<SceneConstantBuffer>> m_sceneCB = nullptr;
	shared_ptr<StructuredBuffer<InstanceBuffer>> m_instanceBuffer = nullptr;

	// Local Root Signature Constants
	PrimitiveConstantBuffer m_planeMaterialCB;
	PrimitiveConstantBuffer m_aabbMaterialCB[IntersectionShaderType::TotalPrimitiveCount];
};