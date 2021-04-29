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
#include "stdafx.h"
#include "ProceduralRtxEngineSample.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

ProceduralRtxEngineSample::ProceduralRtxEngineSample(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_animateGeometryTime(0.0f),
	m_animateCamera(false),
	m_animateGeometry(true),
	m_animateLight(false),
	m_cam(make_shared<Camera>()),
	m_camController(m_cam),
	m_sceneBuilder()
{
	m_raytracingOutputHandles.descriptorIndex = UINT_MAX;
	UpdateForSizeChange(width, height);
}


void ProceduralRtxEngineSample::OnInit()
{
	m_deviceResources = std::make_shared<DeviceResources>(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_UNKNOWN,
		FrameCount,
		D3D_FEATURE_LEVEL_11_0,
		// Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since TH2.
		// Since the sample requires build 1809 (RS5) or higher, we don't need to handle non-tearing cases.
		DeviceResources::c_RequireTearingSupport,
		m_adapterIDoverride
		);
	m_deviceResources->RegisterDeviceNotify(this);
	m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);
	m_deviceResources->InitializeDXGIAdapter();

	ThrowIfFalse(IsDirectXRaytracingSupported(m_deviceResources->GetAdapter()),
		L"ERROR: DirectX Raytracing is not supported by your OS, GPU and/or driver.\n\n");

	m_deviceResources->CreateDeviceResources();
	m_deviceResources->CreateWindowSizeDependentResources();

	m_sceneBuilder.init(m_deviceResources);

	// Setup camera.
	{
		// Initialize the view and projection inverse matrices.
		DirectX::XMVECTOR m_eye = { 0.f, 0.f, 50.0f, 1.0f };
		DirectX::XMVECTOR m_at = { 0.0f, 0.f, 0.0f, 1.0f };
		XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

		XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
		DirectX::XMVECTOR m_up = XMVector3Normalize(XMVector3Cross(direction, right));

		// Rotate camera around Y axis.
		//XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
		//m_eye = XMVector3Transform(m_eye, rotate);
		//m_up = XMVector3Transform(m_up, rotate);

		// Init cam.
		m_cam->SetEyeAtUp(Math::Vector3(m_eye), Math::Vector3(m_at), Math::Vector3(m_up));
		//float fovAngleY = 45.0f;
		//m_cam.SetPerspectiveMatrix(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);

		UpdateCameraMatrices(0.f);
	}

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Update camera matrices passed into the shader.
void ProceduralRtxEngineSample::UpdateCameraMatrices(float deltaT)
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	//(*m_sceneCB)->cameraPosition = m_eye;
	float fovRadiansY = 3.14159265359f/4.f;

	m_cam->SetPerspectiveMatrix(fovRadiansY, 1.f / m_aspectRatio, 0.01f, 125.0f);
	m_camController.Update(deltaT*0.01, m_input);
	//m_cam->Update();

	auto sceneCB = m_sceneBuilder.getSceneCB();
	(*sceneCB)->cameraPosition = m_cam->GetPosition();
	//XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);

	//XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	//XMMATRIX viewProj = view * proj;
	//(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
	(*sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, m_cam->GetViewProjMatrix());


	//float an = 0.5 + float(deltaT) * 0.1;

	//// Initialize the view and projection inverse matrices.
	////DirectX::XMVECTOR m_eye = { 4.0 * sin(an), 4.0 * (-0.3 + sin(2.f * an)), 4.0 * cos(an), 1.0f };

	//DirectX::XMVECTOR m_eye = { 135.0 * sin(2*an)+15, 
	//						   -95 * (sin(an) + 0.6),
	//							155.0 * cos(2 * an)+15, 1.0f };//for mandelbulb
	//
	////float t = 13 * abs(cos(an));
	////DirectX::XMVECTOR m_eye = { 6.0 + t.0 * , -110.8, 6.0 + t, 1.0f };//for mandelbulb
	//
	////DirectX::XMVECTOR m_eye = { 5.0 * sin(an), -4.0 * 0.8, 5.0 * cos(an), 1.0f };
	////DirectX::XMVECTOR m_eye = { 5.0 * sin(an), -5.0 * 0.8, 5.0 * cos(an), 1.0f };
	//
	////DirectX::XMVECTOR m_eye = { 2.0 * sin(an), -5.0 * 0.8 + 2.*cos(an), 2.0 * cos(an), 1.0f };
	////DirectX::XMVECTOR m_at = { 0.1 * sin(an), 1.0, 0.1 * cos(an), 1.0f };
	//
	//DirectX::XMVECTOR m_at = { 15.f, -0.3f, 25.f, 1.0f };
	//XMVECTOR right = { 0.0f, 1.0f, 0.0f, 0.0f };

	//XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
	//DirectX::XMVECTOR m_up = XMVector3Normalize(XMVector3Cross(direction, right));

	//// Rotate camera around Y axis.
	//XMMATRIX rotate = XMMatrixRotationZ(XMConvertToRadians(270.0f));
	////m_eye = XMVector3Transform(m_eye, rotate);
	////m_up = XMVector3Transform(m_up, rotate);

	//// Init cam.
	////m_cam->SetEyeAtUp(Math::Vector3(m_eye), Math::Vector3(m_at), Math::Vector3(m_up));
	//float fovAngleY = 45.0f;
	////m_cam.SetPerspectiveMatrix(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);

	////auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	////float fovRadiansY = 3.14159265359f / 4.f;

	////m_cam->SetPerspectiveMatrix(fovRadiansY, 1.f / m_aspectRatio, 0.01f, 125.0f);
	////m_camController.Update(deltaT, m_input);
	////m_cam->Update();

	//(*m_sceneCB)->cameraPosition = m_eye;
	//XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);

	//XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	//XMMATRIX viewProj =  view * rotate * proj ;
	//(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
	////(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, m_cam->GetViewProjMatrix());

}

// Update AABB primite attributes buffers passed into the shader.
void ProceduralRtxEngineSample::UpdateAABBPrimitiveAttributes(float animationTime)
{
	// Apply scale, rotation and translation transforms.
	// The intersection shader tests in this sample work with local space, so here
	// we apply the BLAS object space translation that was passed to geometry descs.
	auto SetTransformForAABB = [&](UINT instanceIndex, XMMATRIX& mTransform)
	{
		auto instanceBuffer = m_sceneBuilder.getInstanceBuffer();
		(*instanceBuffer)[instanceIndex].localSpaceToBottomLevelAS = XMMatrixInverse(nullptr, mTransform);
		(*instanceBuffer)[instanceIndex].bottomLevelASToLocalSpace = mTransform;
	};
	
	UINT i = 0;
	for (auto geometry : m_sceneBuilder.getScene()->getGeometry())
	{
		if (geometry->getType() == Geometry::Procedural)
		{
			for (auto instance : *geometry->getInstances())
			{
				SetTransformForAABB(i++, instance);
			}
		}
	}
}

// Create resources that depend on the device.
void ProceduralRtxEngineSample::CreateDeviceDependentResources()
{
	// Initialize raytracing pipeline.

	// Create raytracing interfaces: raytracing device and commandlist.
	CreateRaytracingInterfaces();

	// Create a heap for descriptors.
	m_descriptorHeap = make_shared<DescriptorHeap>(m_deviceResources, 3);
	
	// Builds the scene.
	m_sceneBuilder.build(m_dxrDevice, m_deviceResources, m_dxrCommandList, m_descriptorHeap, m_raytracingOutputHandles);

	// Create an output 2D texture to store the raytracing result to.
	CreateRaytracingOutputResource();

	auto shaderTable = m_sceneBuilder.getShaderTable();

	// Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
	m_rayTracingState = make_shared<RayTracingState>(m_sceneBuilder.getScene(), shaderTable->getCommonEntries(), m_dxrDevice,
		m_dxrCommandList, m_deviceResources, m_descriptorHeap);

	// Build shader tables, which define shaders and their local root arguments.
	shaderTable->getBuilded(*m_rayTracingState);
}

// Create raytracing device and command list.
void ProceduralRtxEngineSample::CreateRaytracingInterfaces()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
	ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

// Create a 2D output texture for raytracing.
void ProceduralRtxEngineSample::CreateRaytracingOutputResource()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

	// Create the output resource. The dimensions and format should match the swap-chain.
	auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->CreateCommittedResource(
		&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
	NAME_D3D12_OBJECT(m_raytracingOutput);

	m_raytracingOutputHandles = m_descriptorHeap->allocateDescriptor(m_raytracingOutputHandles.descriptorIndex);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, m_raytracingOutputHandles.cpu);
}

void ProceduralRtxEngineSample::OnMouseMove(UINT x, UINT y)
{
	m_input.newMousePos(XMFLOAT2(float(x), float(y)));
}

void ProceduralRtxEngineSample::OnLeftButtonDown(UINT x, UINT y)
{
	m_input.setMouseButton(InputManager::LEFT, true);
	m_input.newMousePos(XMFLOAT2(float(x), float(y)));
	m_input.newMousePos(XMFLOAT2(float(x), float(y)));
}

void ProceduralRtxEngineSample::OnLeftButtonUp(UINT x, UINT y)
{
	m_input.setMouseButton(InputManager::LEFT, false);
}

void ProceduralRtxEngineSample::OnKeyDown(UINT8 key)
{
	m_input.setKey(key, true);

	switch (key)
	{
	case 'G':
		m_animateGeometry = !m_animateGeometry;
		break;
	case 'L':
		m_animateLight = !m_animateLight;
		break;
	}
}

void ProceduralRtxEngineSample::OnKeyUp(UINT8 key)
{
	// DEBUG
	/*{
		stringstream ss; ss << key << " up" << endl << endl;
		OutputDebugStringA(ss.str().c_str());
	}*/

	m_input.setKey(key, false);

	if (key == 'Z')
	{
		auto sceneCB = m_sceneBuilder.getSceneCB();
		(*sceneCB)->debugFlag = !(*sceneCB)->debugFlag;
	}
}

// Update frame-based values.
void ProceduralRtxEngineSample::OnUpdate()
{
	m_timer.Tick();
	CalculateFrameStats();
	float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
	
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
	auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

	auto sceneCB = m_sceneBuilder.getSceneCB();

	// Rotate the second light around Y axis.
	if (m_animateLight)
	{
		float secondsToRotateAround = 8.0f;
		float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
		const XMVECTOR& prevLightPosition = (*sceneCB)->lightPosition;
		(*sceneCB)->lightPosition = XMVector3Transform(prevLightPosition, rotate);
	}

	// Transform the procedural geometry.
	if (m_animateGeometry)
	{
		m_animateGeometryTime += elapsedTime;
	}

	UpdateCameraMatrices(m_animateGeometryTime);

	UpdateAABBPrimitiveAttributes(m_animateGeometryTime);
	(*sceneCB)->elapsedTime = m_animateGeometryTime;
	
	auto keyState = m_input.getKeyState();
}

// Update the application state with the new resolution.
void ProceduralRtxEngineSample::UpdateForSizeChange(UINT width, UINT height)
{
	DXSample::UpdateForSizeChange(width, height);
}

// Copy the raytracing output to the backbuffer.
void ProceduralRtxEngineSample::CopyRaytracingOutputToBackbuffer()
{
	auto commandList = m_deviceResources->GetCommandList();
	auto renderTarget = m_deviceResources->GetRenderTarget();

	D3D12_RESOURCE_BARRIER preCopyBarriers[2];
	preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	commandList->CopyResource(renderTarget, m_raytracingOutput.Get());

	D3D12_RESOURCE_BARRIER postCopyBarriers[2];
	postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

// Create resources that are dependent on the size of the main window.
void ProceduralRtxEngineSample::CreateWindowSizeDependentResources()
{
	CreateRaytracingOutputResource();
	m_sceneBuilder.getScene()->getGlobalSignature().updateHeapHandle(m_raytracingOutputHandles.baseHandleIndex, m_raytracingOutputHandles.gpu);
	UpdateCameraMatrices(0.f);
}

// Release resources that are dependent on the size of the main window.
void ProceduralRtxEngineSample::ReleaseWindowSizeDependentResources()
{
	m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void ProceduralRtxEngineSample::ReleaseDeviceDependentResources()
{
	m_dxrDevice.Reset();
	m_dxrCommandList.Reset();

	m_sceneBuilder.release();

	m_raytracingOutput.Reset();
	m_raytracingOutputHandles.descriptorIndex = UINT_MAX;
}

void ProceduralRtxEngineSample::RecreateD3D()
{
	// Give GPU a chance to finish its execution in progress.
	try
	{
		m_deviceResources->WaitForGpu();
	}
	catch (HrException&)
	{
		// Do nothing, currently attached adapter is unresponsive.
	}
	m_deviceResources->HandleDeviceLost();
}

// Render the scene.
void ProceduralRtxEngineSample::OnRender()
{
	if (!m_deviceResources->IsWindowVisible())
	{
		return;
	}

	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	m_deviceResources->Prepare();

	auto shaderTable = m_sceneBuilder.getShaderTable();
	m_rayTracingState->doRayTracing(shaderTable->getBuilded(*m_rayTracingState), m_width, m_height);
	CopyRaytracingOutputToBackbuffer();

	m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

void ProceduralRtxEngineSample::OnDestroy()
{
	// Let GPU finish before releasing D3D resources.
	m_deviceResources->WaitForGpu();
	OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void ProceduralRtxEngineSample::OnDeviceLost()
{
	ReleaseWindowSizeDependentResources();
	ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void ProceduralRtxEngineSample::OnDeviceRestored()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void ProceduralRtxEngineSample::CalculateFrameStats()
{
	static int frameCnt = 0;
	static double prevTime = 0.0f;
	double totalTime = m_timer.GetTotalSeconds();

	frameCnt++;

	// Compute averages over one second period.
	if ((totalTime - prevTime) >= 1.0f)
	{
		float diff = static_cast<float>(totalTime - prevTime);
		float fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

		frameCnt = 0;
		prevTime = totalTime;
		float raytracingTime = static_cast<float>(m_rayTracingState->getGpuTimer().GetElapsedMS());
		float MRaysPerSecond = NumMRaysPerSecond(m_width, m_height, raytracingTime);

		wstringstream windowText;
		windowText << setprecision(2) << fixed
			<< L"    fps: " << fps
			<< L"    DispatchRays(): " << raytracingTime << "ms"
			<< L"     ~Million Primary Rays/s: " << MRaysPerSecond
			<< L"    GPU[" << m_deviceResources->GetAdapterID() << L"]: " << m_deviceResources->GetAdapterDescription();
		SetCustomWindowText(windowText.str().c_str());
	}
}

// Handle OnSizeChanged message event.
void ProceduralRtxEngineSample::OnSizeChanged(UINT width, UINT height, bool minimized)
{
	if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
	{
		return;
	}

	UpdateForSizeChange(width, height);

	ReleaseWindowSizeDependentResources();
	CreateWindowSizeDependentResources();
}