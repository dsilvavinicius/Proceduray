//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"
#include "StepTimer.h"
#include "PerformanceTimers.h"
#include "engine/SceneDefines.h"
#include "engine/StaticScene.h"
#include "engine/DescriptorHeap.h"
#include "engine/AccelerationStructure.h"
#include "engine/RayTracingState.h"
#include "engine/ShaderTable.h"
#include "engine/CamController.h"
#include "Camera.h"

using namespace RtxEngine;

class ProceduralRtxEngineSample : public DXSample
{
public:
	ProceduralRtxEngineSample(UINT width, UINT height, std::wstring name);

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;

	// Messages
	virtual void OnInit();
	virtual void OnMouseMove(UINT x, UINT y) override;
	virtual void OnLeftButtonDown(UINT x, UINT y) override;
	virtual void OnLeftButtonUp(UINT x, UINT y) override;
	virtual void OnKeyDown(UINT8 key) override;
	virtual void OnKeyUp(UINT8 key) override;
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
	virtual void OnDestroy();
	virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
	static const UINT FrameCount = 3;

	// DirectX Raytracing (DXR) attributes
	ComPtr<ID3D12Device5> m_dxrDevice;
	ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;

	StaticScenePtr m_scene;
	DescriptorHeapPtr m_descriptorHeap = nullptr;
	AccelerationStructurePtr m_accelerationStruct = nullptr;
	RayTracingStatePtr m_rayTracingState = nullptr;
	ShaderTablePtr m_shaderTable = nullptr;

	// Global Root Signature components.
	shared_ptr<ConstantBuffer<SceneConstantBuffer>> m_sceneCB;
	shared_ptr<StructuredBuffer<PrimitiveInstancePerFrameBuffer>> m_aabbPrimitiveAttributeBuffer;
	// Ray tracing output.
	ComPtr<ID3D12Resource> m_raytracingOutput;
	DescriptorHeap::DescriptorHandles m_raytracingOutputHandles;

	// Local Root Signature Constants
	PrimitiveConstantBuffer m_planeMaterialCB;
	PrimitiveConstantBuffer m_aabbMaterialCB[IntersectionShaderType::TotalPrimitiveCount];
	
	// Application state
	StepTimer m_timer;
	float m_animateGeometryTime;
	bool m_animateGeometry;
	bool m_animateCamera;
	bool m_animateLight;

	CameraPtr m_cam;
	CamController m_camController;
	InputManager m_input;

	void InitializeScene();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void CreateRaytracingInterfaces();

	void CreateConstantBuffers();
	void CreateInstanceBuffer();
	void CreateRays();
	void CreateHitGroups();
	void BuildGeometry();
	void BuildProceduralGeometryAABBs(const XMMATRIX& proceduralBlasTransform);
	void BuildInstancedProcedural();
	void BuildInstancedParallelepipeds();
	void BuildPlaneGeometry(const XMFLOAT3& width);
	void CreateRaytracingOutputResource();

	void CreateAccelerationStructures();
	void CreateRootSignatures();
	void CreateShaderTablesEntries();
	
	void ReleaseDeviceDependentResources();
	void ReleaseWindowSizeDependentResources();
	
	void RecreateD3D();
	void UpdateCameraMatrices(float deltaT);
	void UpdateAABBPrimitiveAttributes(float animationTime);
	void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
	void CopyRaytracingOutputToBackbuffer();
	void CalculateFrameStats();
};