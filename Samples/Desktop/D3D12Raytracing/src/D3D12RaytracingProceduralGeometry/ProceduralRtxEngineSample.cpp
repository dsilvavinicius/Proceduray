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
	m_scene(make_shared<StaticScene>())
{
	m_sceneCB = make_shared<ConstantBuffer<SceneConstantBuffer>>();
	m_instanceBuffer = make_shared<StructuredBuffer<InstanceBuffer>>();
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

	InitializeScene();

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

	(*m_sceneCB)->cameraPosition = m_cam->GetPosition();
	//XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);

	//XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	//XMMATRIX viewProj = view * proj;
	//(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
	(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, m_cam->GetViewProjMatrix());


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
		(*m_instanceBuffer)[instanceIndex].localSpaceToBottomLevelAS = XMMatrixInverse(nullptr, mTransform);
		(*m_instanceBuffer)[instanceIndex].bottomLevelASToLocalSpace = mTransform;
	};
	
	UINT i = 0;
	for (auto geometry : m_scene->getGeometry())
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

// Initialize scene rendering parameters.
void ProceduralRtxEngineSample::InitializeScene()
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	// Setup materials.
	{
		auto SetAttributes = [&](
			UINT primitiveIndex,
			const XMFLOAT4& albedo,
			float reflectanceCoef = 0.0f,
			float diffuseCoef = 0.9f,
			float specularCoef = 0.7f,
			float specularPower = 50.0f,
			float stepScale = 1.0f)
		{
			auto& attributes = m_aabbMaterialCB[primitiveIndex];
			attributes.albedo = albedo;
			attributes.reflectanceCoef = reflectanceCoef;
			attributes.diffuseCoef = diffuseCoef;
			attributes.specularCoef = specularCoef;
			attributes.specularPower = specularPower;
			attributes.stepScale = stepScale;
		};


		m_planeMaterialCB = { XMFLOAT4(1.f, 0.9f, 0.7f, 1.0f), 0.25f, 1, 0.4f, 50, 1 };
		
		// Albedos
		XMFLOAT4 green = XMFLOAT4(0.1f, 1.0f, 0.5f, 1.0f);
		XMFLOAT4 red = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
		XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

		//UINT offset = 0;
		// Analytic primitives.
		//{
		//	using namespace AnalyticPrimitive;
		//	//SetAttributes(offset + AABB, red);
		//	SetAttributes(Spheres, ChromiumReflectance, 1);
		//	//offset += AnalyticPrimitive::Count;
		//}

		// Volumetric primitives.
		//{
		//	using namespace VolumetricPrimitive;
		//	SetAttributes(/*offset +*/ Metaballs, ChromiumReflectance, 1);
		//	//offset += VolumetricPrimitive::Count;
		//}

		//// Signed distance primitives.
		//{
			using namespace SignedDistancePrimitive;
			SetAttributes(Mandelbulb, yellow, 0.2f, 0.6f);
			SetAttributes(IntersectedRoundCube, yellow, 0.3f, 0.3f);
			SetAttributes(JuliaSets, red, 0.2f, 0.6f);
		//	SetAttributes(offset + MiniSpheres, green);
			//ChromiumReflectance, 1);
		//	SetAttributes(offset + SquareTorus, ChromiumReflectance, 1);
		//	SetAttributes(offset + TwistedTorus, yellow, 0, 1.0f, 0.7f, 50, 0.5f);
		//	SetAttributes(offset + Cog, yellow, 0, 1.0f, 0.1f, 2);
		//	SetAttributes(offset + Cylinder, red);
		//	SetAttributes(offset + FractalPyramid, green, 0, 1, 0.1f, 4, 0.8f);
		//}
	}

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

	// Setup lights.
	{
		// Initialize the lighting parameters.
		XMFLOAT4 lightPosition;
		XMFLOAT4 lightAmbientColor;
		XMFLOAT4 lightDiffuseColor;

		lightPosition = XMFLOAT4(0.0f, 18.0f, -20.0f, 0.0f);
		(*m_sceneCB)->lightPosition = XMLoadFloat4(&lightPosition);

		lightAmbientColor = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
		(*m_sceneCB)->lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

		float d = 0.6f;
		lightDiffuseColor = XMFLOAT4(d, d, d, 1.0f);
		(*m_sceneCB)->lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
	}
	(*m_sceneCB)->debugFlag = false;
}

// Create constant buffers.
void ProceduralRtxEngineSample::CreateConstantBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	m_sceneCB->Create(device, frameCount, L"Scene Constant Buffer");
}

// Create AABB primitive attributes buffers.
void ProceduralRtxEngineSample::CreateInstanceBuffer()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	UINT nProceduralInstances = 0;
	for (auto geometry : m_scene->getGeometry())
	{
		if(geometry->getType() == Geometry::Procedural)
		{
			for (auto instance : *geometry->getInstances())
			{
				++nProceduralInstances;
			}
		}
	}

	m_instanceBuffer->Create(device, nProceduralInstances, frameCount, L"AABB primitive attributes");
}

// Create resources that depend on the device.
void ProceduralRtxEngineSample::CreateDeviceDependentResources()
{
	// Initialize raytracing pipeline.

	// Create raytracing interfaces: raytracing device and commandlist.
	CreateRaytracingInterfaces();

	// Create a heap for descriptors.
	m_descriptorHeap = make_shared<DescriptorHeap>(m_deviceResources, 3);
	
	// Add rays to the scene.
	CreateRays();
	
	// Add hit groups to the scene.
	CreateHitGroups();

	// Build geometry to be used in the sample.
	BuildGeometry();

	// Create constant buffers for the geometry and the scene.
	CreateConstantBuffers();

	// Create AABB primitive attribute buffers.
	CreateInstanceBuffer();

	// Create an output 2D texture to store the raytracing result to.
	CreateRaytracingOutputResource();

	CreateAccelerationStructure();

	// Create root signatures for the shaders.
	CreateRootSignatures();

	CreateShaderTablesEntries();

	// Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
	m_rayTracingState = make_shared<RayTracingState>(m_scene, m_shaderTable->getCommonEntries(), m_dxrDevice, m_dxrCommandList, m_deviceResources, m_descriptorHeap);

	// Build shader tables, which define shaders and their local root arguments.
	m_shaderTable->getBuilded(*m_rayTracingState);
}

void ProceduralRtxEngineSample::CreateRays()
{
	m_scene->addRay(make_shared<Ray>("Radiance", L"Miss", Payload(RayPayload())));
	m_scene->addRay(make_shared<Ray>("Shadow", L"Miss_Shadow", Payload(ShadowRayPayload())));
}

void ProceduralRtxEngineSample::CreateHitGroups()
{
	// Triangle Hit Groups.
	m_scene->addHitGroup(make_shared<HitGroup>("Triangle", L"HitGroup_Triangle", L"", L"ClosestHit_Triangle", L""));
	m_scene->addHitGroup(make_shared<HitGroup>("Triangle_Shadow", L"HitGroup_Triangle_Shadow", L"", L"", L""));

	// Procedural Hit Groups.
	m_scene->addHitGroup(make_shared<HitGroup>("Pacman", L"HitGroup_Pacman", L"", L"ClosestHit_Pacman", L"Intersection_Pacman"));
	m_scene->addHitGroup(make_shared<HitGroup>("Pacman_Shadow", L"HitGroup_Pacman_Shadow", L"", L"", L"Intersection_Pacman"));

	m_scene->addHitGroup(make_shared<HitGroup>("Mandelbulb", L"HitGroup_Mandelbulb", L"", L"ClosestHit_Mandelbulb", L"Intersection_Mandelbulb"));
	m_scene->addHitGroup(make_shared<HitGroup>("Mandelbulb_Shadow", L"HitGroup_Mandelbulb_Shadow", L"", L"", L"Intersection_Mandelbulb"));
	
	m_scene->addHitGroup(make_shared<HitGroup>("Julia", L"HitGroup_Julia", L"", L"ClosestHit_Julia", L"Intersection_Julia"));
	m_scene->addHitGroup(make_shared<HitGroup>("Julia_Shadow", L"HitGroup_Julia_Shadow", L"", L"", L"Intersection_Julia"));
}

void ProceduralRtxEngineSample::CreateAccelerationStructure()
{
	m_accelerationStruct = make_shared<AccelerationStructure>(m_scene, m_dxrDevice, m_dxrCommandList, m_deviceResources);
}

void ProceduralRtxEngineSample::CreateRootSignatures()
{
	// Global root signature.
	auto globalSignature = make_shared<RootSignature>("GlobalSignature", m_deviceResources, m_descriptorHeap, false);
	
	// Global signature ranges.
	auto outputRange = globalSignature->createRange(m_raytracingOutputHandles.gpu, RootSignature::UAV, 0, 1);
	auto globalGeometry = m_scene->getGeometryMap().at("GlobalGeometry");
	auto vertexRange = globalSignature->createRange(globalGeometry->getIndexBuffer().gpuDescriptorHandle, RootSignature::SRV, 1, 2);

	// Global signature entries.
	m_raytracingOutputHandles.baseHandleIndex = globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{outputRange});
	globalSignature->addEntry(RootComponent(DontApply()), RootSignature::SRV, m_accelerationStruct->getBuilded(), 0);
	globalSignature->addEntry(RootComponent(SceneConstantBuffer()), RootSignature::CBV, m_sceneCB, 0);
	globalSignature->addEntry(RootComponent(InstanceBuffer()), RootSignature::SRV, m_instanceBuffer, 3);
	globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{vertexRange});

	m_scene->addGlobalSignature(globalSignature);

	// Triangle geometry local root signature.
	auto triangleSignature = make_shared<RootSignature>("Triangle", m_deviceResources, m_descriptorHeap, true);
	triangleSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);
	
	// Root Arguments type.
	triangleSignature->setRootArgumentsType(RootArguments(TriangleRootArguments()));
	m_scene->addLocalSignature(triangleSignature);

	// Procedural geometry local root signature.
	auto proceduralSignature = make_shared<RootSignature>("Procedural", m_deviceResources, m_descriptorHeap, true);
	proceduralSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);
	proceduralSignature->addConstant(RootComponent(PrimitiveInstanceConstantBuffer()), 2);
	
	// Root Arguments.
	proceduralSignature->setRootArgumentsType(RootArguments(ProceduralRootArguments()));
	m_scene->addLocalSignature(proceduralSignature);
}

// Create raytracing device and command list.
void ProceduralRtxEngineSample::CreateRaytracingInterfaces()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto commandList = m_deviceResources->GetCommandList();

	ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&m_dxrDevice)), L"Couldn't get DirectX Raytracing interface for the device.\n");
	ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
}

void ProceduralRtxEngineSample::CreateShaderTablesEntries()
{
	m_shaderTable = make_shared<RtxEngine::ShaderTable>(m_scene, m_deviceResources);
	
	// Ray gen.
	m_shaderTable->addRayGen(L"Raygen");

	// Miss.
	m_shaderTable->addMiss("Radiance");
	m_shaderTable->addMiss("Shadow");
	
	// Triangle Hit Groups.
	{
		TriangleRootArguments rootArgs{ m_planeMaterialCB };
		m_shaderTable->addCommonEntry(ShaderTableEntry{ "Radiance", "Triangle", "Triangle", rootArgs });
		m_shaderTable->addCommonEntry(ShaderTableEntry{ "Shadow", "Triangle_Shadow", "Triangle", rootArgs });
	}
	
	// Procedural hit groups.
	{
		UINT primitiveIndex = 0;
		UINT instanceIndex = 0;

		UINT i = 0;
		for (auto geometry : m_scene->getGeometry())
		{
			if (geometry->getType() == Geometry::Procedural)
			{
				for (auto instance : *geometry->getInstances())
				{
					ProceduralRootArguments rootArgs;
					rootArgs.materialCb = m_aabbMaterialCB[primitiveIndex];
					rootArgs.aabbCB.primitiveType = primitiveIndex;
					rootArgs.aabbCB.instanceIndex = instanceIndex;

					string radianceHitGroup;
					string shadowHitGroup;

					switch (primitiveIndex)
					{
						case SignedDistancePrimitive::Mandelbulb:
						{
							radianceHitGroup = "Mandelbulb";
							shadowHitGroup = "Mandelbulb_Shadow";
							break;
						}
						case SignedDistancePrimitive::IntersectedRoundCube:
						{
							radianceHitGroup = "Pacman";
							shadowHitGroup = "Pacman_Shadow";
							break;
						}
						case SignedDistancePrimitive::JuliaSets:
						{
							radianceHitGroup = "Julia";
							shadowHitGroup = "Julia_Shadow";
							break;
						}
					}

					m_shaderTable->addCommonEntry(ShaderTableEntry{ "Radiance", radianceHitGroup, "Procedural", rootArgs });
					m_shaderTable->addCommonEntry(ShaderTableEntry{ "Shadow", shadowHitGroup, "Procedural", rootArgs });
					
					++instanceIndex;
				}
				++primitiveIndex;
			}
		}
	}
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

void ProceduralRtxEngineSample::BuildInstancedProcedural()
{
	int N = 1;

	// Bottom-level AS with a single plane.
	Geometry::Instances mandelbulbInstances;
	Geometry::Instances pacManInstances;
	Geometry::Instances juliaInstances;
	
	//XMMATRIX scale = XMMatrixScaling(15.f, 15.f, 15.f);

	//it iterates in a nxn grid
	for (float i = 0; i < N; i+=3)
	{
		for (float j = 0; j < N; ++j)
		{
			for (float k = 0; k < N; ++k)
			{
				{
					//XMFLOAT3 float3(i + 2, j, k);
					XMFLOAT3 float3(i, j-1., k+2.3);
					XMMATRIX translation = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3));
					
					XMMATRIX mandelbulbScale = XMMatrixScaling(40.f, 40.f, 40.f);

					mandelbulbInstances.push_back(mandelbulbScale * /*rotation **/ translation);
				}
				
				{
					//XMFLOAT3 float3(i + 1, j, k);
					/*XMFLOAT3 float3_1(i, j, k);
					XMMATRIX translation1 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_1));*/

					XMFLOAT3 float3_2(i+0.7, j-0.7, k);
					XMMATRIX translation2 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_2));

					XMFLOAT3 float3_3(i-0.7, j-0.7, k);
					XMMATRIX translation3 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_3));

					XMMATRIX pacmanScale = XMMatrixScaling(50.f, 50.f, 50.f);

					//pacManInstances.push_back(pacmanScale * /* rotation **/ translation1);
					pacManInstances.push_back(pacmanScale * /* rotation **/ translation2);
					pacManInstances.push_back(pacmanScale * /* rotation **/ translation3);
				}

				{
					XMFLOAT3 float3(i, j +2.6, k+1.5);
					XMMATRIX rotation = XMMatrixRotationX(3.1421f);
					XMMATRIX translation = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3));

					XMMATRIX juliaScale = XMMatrixScaling(100.f, 30.f, 100.f);

					juliaInstances.push_back(juliaScale * rotation * translation);
				}
			}
		}
	}


	D3D12_RAYTRACING_AABB juliaAABB{ -3.5f, -3.5f, -3.5f, 3.5f, 3.5f, 3.5f };
	m_scene->addGeometry(make_shared<Geometry>("Julia", juliaAABB, *m_deviceResources, juliaInstances));

	D3D12_RAYTRACING_AABB pacmanAABB{ -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	m_scene->addGeometry(make_shared<Geometry>("Pacman", pacmanAABB, *m_deviceResources, pacManInstances));

	D3D12_RAYTRACING_AABB mandelbulbAABB{ -1.5f, -1.5f, -1.5f, 1.5f, 1.5f, 1.5f };
	m_scene->addGeometry(make_shared<Geometry>("Mandelbulb", mandelbulbAABB, *m_deviceResources, mandelbulbInstances));
}

void ProceduralRtxEngineSample::BuildInstancedParallelepipeds()
{
	XMFLOAT3 q = XMFLOAT3(-0.5f, 0.f, 0.f);
	XMFLOAT3 p = XMFLOAT3(0.5f, 0.f, 0.f);

	float d = 0.2f;

	//for each point, we creat a parallelepiped in the x-direction, thus 8 vertices
	XMFLOAT3 q01 = XMFLOAT3(q.x, q.y - d, q.z + d);
	XMFLOAT3 q00 = XMFLOAT3(q.x, q.y - d, q.z - d);
	XMFLOAT3 q10 = XMFLOAT3(q.x, q.y + d, q.z - d);
	XMFLOAT3 q11 = XMFLOAT3(q.x, q.y + d, q.z + d);

	XMFLOAT3 p01 = XMFLOAT3(p.x, p.y - d, p.z + d);
	XMFLOAT3 p00 = XMFLOAT3(p.x, p.y - d, p.z - d);
	XMFLOAT3 p10 = XMFLOAT3(p.x, p.y + d, p.z - d);
	XMFLOAT3 p11 = XMFLOAT3(p.x, p.y + d, p.z + d);

	vector<Vertex> vertices{
		{ q01, XMFLOAT3(0.0f, 0.0f, -1.0f)  },
		{ q00, XMFLOAT3(0.0f, 1.0f, 0.0f)  },
		{ q10, XMFLOAT3(1.0f, 0.0f, 0.0f)  },
		{ q11, XMFLOAT3(0.0f, -1.0f, 0.0f) },

		{ p01, XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ p00, XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ p10, XMFLOAT3(0.0f, 1.0f, 0.0f)  },
		{ p11, XMFLOAT3(0.0f, 1.0f, 0.0f)  }
	};

	vector<Index> indices{
		0, 4, 7,
		0, 7, 3,
		1, 5, 4,
		1, 4, 0,
		5, 2, 6,
		5, 1, 2,
		3, 7, 6,
		3, 6, 2,
		2, 0, 3,
		2, 1, 0,
		4, 6, 7,
		4, 5, 6
	};

	int N = 1;

	// Bottom-level AS with a single plane.
	Geometry::Instances instances;
		
	// Scale in XZ dimensions.
	XMMATRIX mScale = XMMatrixScaling(120, 15, 120);

	//it iterates in a nxn grid
	for (float i = 0; i < N; i++)
	{
		for (float j = 0; j < N; j++)
		{
			for (float k = 0; k < N; k++)
			{
				XMFLOAT3 globalTranslation(0.f, -15.f, 0.f);
				XMFLOAT3 float3(i, j+0.5f, k);

				XMMATRIX mTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&globalTranslation) + (30. * XMLoadFloat3(&float3)));

				instances.push_back(mScale * mTranslation);
			}
		}
	}

	m_scene->addGeometry(make_shared<Geometry>("GlobalGeometry", vertices, indices, *m_deviceResources, *m_descriptorHeap, instances));
}

void ProceduralRtxEngineSample::BuildPlaneGeometry(const XMFLOAT3& width)
{	
	vector<Index> indices;
	vector<Vertex> vertices;

	//build floor
	{
		int n = 256;

		//it iterates in a nxn grid
		for (int k = 0; k < n * n; k++)
		{
			//creating vertices coordinates
			int i = k % n;
			int j = k / n;

			float x = float(i) / float(n - 1);
			float z = float(j) / float(n - 1);

			vertices.push_back({ XMFLOAT3(x, 0.f, z), XMFLOAT3(0.0f, 1.0f, 0.0f) });

			//index of the two triangle inside the square
			if (i < n - 1 && j < n - 1)
			{
				indices.push_back(k);
				indices.push_back(k + n);
				indices.push_back(k + 1);

				indices.push_back(k + 1);
				indices.push_back(k + n);
				indices.push_back(k + n + 1);
			}
		}
	}

	XMFLOAT3 translation(-0.35f, 0.0f, -0.35f);
	const XMVECTOR vWidth = XMLoadFloat3(&width);
	const XMVECTOR vBasePosition = vWidth * XMLoadFloat3(&translation);
	auto wTranslation = XMMatrixTranslationFromVector(vBasePosition);
	XMMATRIX mScale = XMMatrixScaling(width.x, width.y, width.z);
	auto triangleBlasTransform = mScale * wTranslation;

	m_scene->addGeometry(make_shared<Geometry>("GlobalGeometry", vertices, indices, *m_deviceResources, *m_descriptorHeap,
		Geometry::Instances(1, triangleBlasTransform)));
}

// Build geometry used in the sample.
void ProceduralRtxEngineSample::BuildGeometry()
{
	BuildInstancedParallelepipeds();
	BuildInstancedProcedural();
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
		(*m_sceneCB)->debugFlag = !(*m_sceneCB)->debugFlag;
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

	// Rotate the second light around Y axis.
	if (m_animateLight)
	{
		float secondsToRotateAround = 8.0f;
		float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
		XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
		const XMVECTOR& prevLightPosition = (*m_sceneCB)->lightPosition;
		(*m_sceneCB)->lightPosition = XMVector3Transform(prevLightPosition, rotate);
	}

	// Transform the procedural geometry.
	if (m_animateGeometry)
	{
		m_animateGeometryTime += elapsedTime;
	}

	UpdateCameraMatrices(m_animateGeometryTime);

	UpdateAABBPrimitiveAttributes(m_animateGeometryTime);
	(*m_sceneCB)->elapsedTime = m_animateGeometryTime;
	
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
	m_scene->getGlobalSignature().updateHeapHandle(m_raytracingOutputHandles.baseHandleIndex, m_raytracingOutputHandles.gpu);
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

	m_sceneCB->Release();
	m_instanceBuffer->Release();

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

	m_rayTracingState->doRayTracing(m_shaderTable->getBuilded(*m_rayTracingState), m_width, m_height);
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