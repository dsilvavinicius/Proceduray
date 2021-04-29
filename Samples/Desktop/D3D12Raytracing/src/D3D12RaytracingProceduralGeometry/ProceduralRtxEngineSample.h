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
#include "MandelJuliaPacSceneBuilder.h"
#include "engine/DxrInternal.h"
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
	virtual IDXGISwapChain* GetSwapchain() { return m_dxr->deviceResources->GetSwapChain(); }

private:
	static const UINT FrameCount = 3;

	// DirectX Raytracing (DXR) low and mid level abstractions.
	DxrInternalPtr m_dxr;

	// Scene builder.
	MandelJuliaPacSceneBuilder m_sceneBuilder;
	
	// Application state
	StepTimer m_timer;
	float m_animateGeometryTime;
	bool m_animateGeometry;
	bool m_animateCamera;
	bool m_animateLight;

	CameraPtr m_cam;
	CamController m_camController;
	InputManager m_input;

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void CreateRaytracingInterfaces();
	void CreateRaytracingOutputResource();
	
	void ReleaseDeviceDependentResources();
	void ReleaseWindowSizeDependentResources();
	
	void RecreateD3D();
	void UpdateCameraMatrices(float deltaT);
	void UpdateAABBPrimitiveAttributes(float animationTime);
	void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
	void CopyRaytracingOutputToBackbuffer();
	void CalculateFrameStats();
};