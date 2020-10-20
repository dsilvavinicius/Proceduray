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
	m_aabbPrimitiveAttributeBuffer = make_shared<StructuredBuffer<PrimitiveInstancePerFrameBuffer>>();
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
	float fovRadiansY = 3.14159265359/4.;

	//stringstream ss;
	//ss << "Aspect: " << m_aspectRatio << endl << endl;
	//OutputDebugStringA(ss.str().c_str());

	m_cam->SetPerspectiveMatrix(fovRadiansY, 1.f / m_aspectRatio, 0.01f, 125.0f);
	m_camController.Update(deltaT, m_input);
	//m_cam->Update();

	(*m_sceneCB)->cameraPosition = m_cam->GetPosition();
	//XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
	
	//XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
	//XMMATRIX viewProj = view * proj;
	//(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
	(*m_sceneCB)->projectionToWorld = XMMatrixInverse(nullptr, m_cam->GetViewProjMatrix());
}

// Update AABB primite attributes buffers passed into the shader.
void ProceduralRtxEngineSample::UpdateAABBPrimitiveAttributes(float animationTime)
{
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

	XMMATRIX mIdentity = XMMatrixIdentity();

	XMMATRIX mScale15y = XMMatrixScaling(1, 1.5, 1);
	XMMATRIX mScale15 = XMMatrixScaling(1.5, 1.5, 1.5);
	XMMATRIX mScale2 = XMMatrixScaling(2, 2, 2);
	XMMATRIX mScale3 = XMMatrixScaling(3, 3, 3);

	XMMATRIX mRotation = XMMatrixRotationY(-2 * animationTime);

	// Apply scale, rotation and translation transforms.
	// The intersection shader tests in this sample work with local space, so here
	// we apply the BLAS object space translation that was passed to geometry descs.
	auto SetTransformForAABB = [&](UINT instanceIndex, XMMATRIX& mTransform)
	{
		(*m_aabbPrimitiveAttributeBuffer)[instanceIndex].localSpaceToBottomLevelAS = XMMatrixInverse(nullptr, mTransform);
		(*m_aabbPrimitiveAttributeBuffer)[instanceIndex].bottomLevelASToLocalSpace = mTransform;
	};
	
	UINT i = 0;
	for (auto geometry : m_scene->getGeometry())
	{
		if (geometry->getType() == Geometry::Procedural)
		{
			SetTransformForAABB(i, (*geometry->getInstances())[0]);
			++i;
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
		XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);

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
			SetAttributes(Mandelbulb, yellow, 0.2, 0.6);
			SetAttributes(IntersectedRoundCube, green, 0.2, 0.6);
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
}

// Create constant buffers.
void ProceduralRtxEngineSample::CreateConstantBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	m_sceneCB->Create(device, frameCount, L"Scene Constant Buffer");
}

// Create AABB primitive attributes buffers.
void ProceduralRtxEngineSample::CreateAABBPrimitiveAttributesBuffers()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto frameCount = m_deviceResources->GetBackBufferCount();

	UINT nProceduralInstances = 0;
	for (auto geometry : m_scene->getGeometry())
	{
		if(geometry->getType() == Geometry::Procedural)
		{
			++nProceduralInstances;
		}
	}

	m_aabbPrimitiveAttributeBuffer->Create(device, nProceduralInstances, frameCount, L"AABB primitive attributes");
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
	CreateAABBPrimitiveAttributesBuffers();

	// Create an output 2D texture to store the raytracing result to.
	CreateRaytracingOutputResource();

	CreateAccelerationStructures();

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
	m_scene->addRay("Radiance", make_shared<Ray>(L"MyMissShader", Payload(RayPayload())));
	m_scene->addRay("Shadow", make_shared<Ray>(L"MyMissShader_ShadowRay", Payload(ShadowRayPayload())));
}

void ProceduralRtxEngineSample::CreateHitGroups()
{
	// Triangle Hit Groups.
	m_scene->addHitGroup("Triangle", make_shared<HitGroup>(L"MyHitGroup_Triangle", L"", L"MyClosestHitShader_Triangle", L""));
	m_scene->addHitGroup("Triangle_Shadow", make_shared<HitGroup>(L"MyHitGroup_Triangle_ShadowRay", L"", L"", L""));

	// Procedural Hit Groups.
	// Analytic.
	//m_scene->addHitGroup("Analytic", make_shared<HitGroup>(L"MyHitGroup_AABB_AnalyticPrimitive", L"", L"MyClosestHitShader_AABB", L"MyIntersectionShader_AnalyticPrimitive"));
	//m_scene->addHitGroup("Analytic_Shadow", make_shared<HitGroup>(L"MyHitGroup_AABB_AnalyticPrimitive_ShadowRay", L"", L"", L"MyIntersectionShader_AnalyticPrimitive"));
	// Volumetric.
	//m_scene->addHitGroup("Volumetric", make_shared<HitGroup>(L"MyHitGroup_AABB_VolumetricPrimitive", L"", L"MyClosestHitShader_AABB", L"MyIntersectionShader_VolumetricPrimitive"));
	//m_scene->addHitGroup("Volumetric_Shadow", make_shared<HitGroup>(L"MyHitGroup_AABB_VolumetricPrimitive_ShadowRay", L"", L"", L"MyIntersectionShader_VolumetricPrimitive"));
	//// Signed Distance.
	m_scene->addHitGroup("SignedDist", make_shared<HitGroup>(L"MyHitGroup_AABB_SignedDistancePrimitive", L"", L"MyClosestHitShader_AABB", L"MyIntersectionShader_SignedDistancePrimitive"));
	m_scene->addHitGroup("SignedDist_Shadow", make_shared<HitGroup>(L"MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay", L"", L"", L"MyIntersectionShader_SignedDistancePrimitive"));
}

void ProceduralRtxEngineSample::CreateAccelerationStructures()
{
	m_accelerationStruct = make_shared<AccelerationStructure>(m_scene, m_dxrDevice, m_dxrCommandList, m_deviceResources);
}

void ProceduralRtxEngineSample::CreateRootSignatures()
{
	// Global root signature.
	auto globalSignature = make_shared<RootSignature>(m_deviceResources, m_descriptorHeap, false);
	
	// Global signature ranges.
	auto outputRange = globalSignature->createRange(m_raytracingOutputHandles.gpu, RootSignature::UAV, 0, 1);
	auto globalGeometry = m_scene->getGeometryMap().at("GlobalGeometry");
	auto vertexRange = globalSignature->createRange(globalGeometry->getIndexBuffer().gpuDescriptorHandle, RootSignature::SRV, 1, 2);

	// Global signature entries.
	m_raytracingOutputHandles.baseHandleIndex = globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{outputRange});
	globalSignature->addEntry(RootComponent(DontApply()), RootSignature::SRV, m_accelerationStruct->getBuilded(), 0);
	globalSignature->addEntry(RootComponent(SceneConstantBuffer()), RootSignature::CBV, m_sceneCB, 0);
	globalSignature->addEntry(RootComponent(PrimitiveInstancePerFrameBuffer()), RootSignature::SRV, m_aabbPrimitiveAttributeBuffer, 3);
	globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{vertexRange});

	m_scene->addGlobalSignature(globalSignature);

	// Triangle geometry local root signature.
	auto triangleSignature = make_shared<RootSignature>(m_deviceResources, m_descriptorHeap, true);
	triangleSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);
	
	// Root Arguments type.
	triangleSignature->setRootArgumentsType(RootArguments(TriangleRootArguments()));
	m_scene->addLocalSignature("Triangle", triangleSignature);

	// Procedural geometry local root signature.
	auto proceduralSignature = make_shared<RootSignature>(m_deviceResources, m_descriptorHeap, true);
	proceduralSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);
	proceduralSignature->addConstant(RootComponent(PrimitiveInstanceConstantBuffer()), 2);
	
	// Root Arguments.
	proceduralSignature->setRootArgumentsType(RootArguments(ProceduralRootArguments()));
	m_scene->addLocalSignature("Procedural", proceduralSignature);
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
	m_shaderTable->addRayGen(L"MyRaygenShader");

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
		const string hitGroupIds[][2] =
		{
			{ "Analytic", "Analytic_Shadow" },
			{ "Volumetric", "Volumetric_Shadow" },
			{ "SignedDist", "SignedDist_Shadow" },
		};

		ProceduralRootArguments rootArgs;
		UINT instanceIndex = 0;

		// Create a shader record for each primitive.
		for (UINT iShader = 0, instanceIndex = 0; iShader < IntersectionShaderType::Count; iShader++)
		{
			UINT numPrimitiveTypes = IntersectionShaderType::PerPrimitiveTypeCount(static_cast<IntersectionShaderType::Enum>(iShader));

			// Primitives for each intersection shader.
			for (UINT primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++, instanceIndex++)
			{
				rootArgs.materialCb = m_aabbMaterialCB[instanceIndex];
				rootArgs.aabbCB.instanceIndex = instanceIndex;
				rootArgs.aabbCB.primitiveType = primitiveIndex;

				m_shaderTable->addCommonEntry(ShaderTableEntry{ "Radiance", hitGroupIds[iShader][RayType::Radiance], "Procedural", rootArgs });
				m_shaderTable->addCommonEntry(ShaderTableEntry{ "Shadow", hitGroupIds[iShader][RayType::Shadow], "Procedural", rootArgs });
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

// Build AABBs for procedural geometry within a bottom-level acceleration structure.
void ProceduralRtxEngineSample::BuildProceduralGeometryAABBs(const XMMATRIX& proceduralBlasTransform)
{
	// Set up AABBs on a grid.
	{
		/*XMINT3 aabbGrid = XMINT3(4, 1, 4);
		const XMFLOAT3 basePosition =
		{
			-(aabbGrid.x * c_aabbWidth + (aabbGrid.x - 1) * c_aabbDistance) / 2.0f,
			-(aabbGrid.y * c_aabbWidth + (aabbGrid.y - 1) * c_aabbDistance) / 2.0f,
			-(aabbGrid.z * c_aabbWidth + (aabbGrid.z - 1) * c_aabbDistance) / 2.0f,
		};

		XMFLOAT3 stride = XMFLOAT3(c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance, c_aabbWidth + c_aabbDistance);
		auto InitializeAABB = [&](auto& offsetIndex, auto& size)
		{
			return D3D12_RAYTRACING_AABB{
				basePosition.x + offsetIndex.x * stride.x,
				basePosition.y + offsetIndex.y * stride.y,
				basePosition.z + offsetIndex.z * stride.z,
				basePosition.x + offsetIndex.x * stride.x + size.x,
				basePosition.y + offsetIndex.y * stride.y + size.y,
				basePosition.z + offsetIndex.z * stride.z + size.z,
			};
		};*/
		m_aabbs.resize(IntersectionShaderType::TotalPrimitiveCount);
		//UINT offset = 0;

		// Analytic primitives.
		//{
		//	using namespace AnalyticPrimitive;
		//	/*m_aabbs[offset + AABB] = InitializeAABB(XMINT3(3, 0, 0), XMFLOAT3(2, 3, 2));
		//	m_scene->addGeometry("AABB", make_shared<Geometry>(m_aabbs[offset + AABB], *m_deviceResources));*/

		//	m_aabbs[Spheres] = InitializeAABB(XMFLOAT3(2.25f, 0, 0.75f), XMFLOAT3(10, 10, 10));
		//	m_scene->addGeometry("Spheres", make_shared<Geometry>(m_aabbs[Spheres], *m_deviceResources));
		//	//offset += AnalyticPrimitive::Count;
		//}

		// Volumetric primitives.
		//{
		//	using namespace VolumetricPrimitive;
		//	m_aabbs[/*offset +*/ Metaballs] = InitializeAABB(XMINT3(0, 0, 0), XMFLOAT3(15, 15, 15));
		//	m_scene->addGeometry("Metaballs", make_shared<Geometry>(m_aabbs[/*offset +*/ Metaballs], *m_deviceResources));
		//	//offset += VolumetricPrimitive::Count;
		//}

		//// Signed distance primitives.
		//{
			using namespace SignedDistancePrimitive;
		//	m_aabbs[offset + MiniSpheres] = InitializeAABB(XMINT3(2, 0, 0), XMFLOAT3(2, 2, 2));
		//	m_scene->addGeometry("MiniSpheres", make_shared<Geometry>(m_aabbs[offset + MiniSpheres], *m_deviceResources));
		//	
		//	m_aabbs[offset + TwistedTorus] = InitializeAABB(XMINT3(0, 0, 1), XMFLOAT3(2, 2, 2));
		//	m_scene->addGeometry("TwistedTorus", make_shared<Geometry>(m_aabbs[offset + TwistedTorus], *m_deviceResources));

			//m_aabbs[/*offset +*/ IntersectedRoundCube] = InitializeAABB(XMINT3(0, 0, 2), XMFLOAT3(100, 20, 100));
			int N = 30;

			// Bottom-level AS with a single plane.
			Geometry::Instances instances;

			//it iterates in a nxn grid
			for (int m = 0; m < N * N * N; m++)
			{
				//creating vertices coordinates
				int k = m / (N * N);
				int l = m % (N * N);
				int i = l % N;
				int j = l / N;

				/*float x = float(i) / float(N - 1);
				float x1 = float(i + 1) / float(N - 1);
				float z = float(j) / float(N - 1);
				float y = float(k) / float(N - 1);*/

				float x = float(i);
				float x1 = float(i + 1);
				float z = float(j);
				float y = float(k);

				XMFLOAT3 float3((x1 + x) * 0.5, y, z);

				XMMATRIX mTranslation = XMMatrixTranslationFromVector(1. * XMLoadFloat3(&float3));

				instances.push_back(mTranslation);
			}

			m_aabbs[Mandelbulb] = D3D12_RAYTRACING_AABB{ 0.f, 0.f, 0.f, 1.f, 1.f, 1.f };
			m_scene->addGeometry("Mandelbulb", make_shared<Geometry>(m_aabbs[Mandelbulb], *m_deviceResources,
				instances));

		//	m_aabbs[offset + SquareTorus] = InitializeAABB(XMFLOAT3(0.75f, -0.1f, 2.25f), XMFLOAT3(3, 3, 3));
		//	m_scene->addGeometry("SquareTorus", make_shared<Geometry>(m_aabbs[offset + SquareTorus], *m_deviceResources));

		//	m_aabbs[offset + Cog] = InitializeAABB(XMINT3(1, 0, 0), XMFLOAT3(2, 2, 2));
		//	m_scene->addGeometry("Cog", make_shared<Geometry>(m_aabbs[offset + Cog], *m_deviceResources));

		//	m_aabbs[offset + Cylinder] = InitializeAABB(XMINT3(0, 0, 3), XMFLOAT3(2, 3, 2));
		//	m_scene->addGeometry("Cylinder", make_shared<Geometry>(m_aabbs[offset + Cylinder], *m_deviceResources));

		//	m_aabbs[offset + FractalPyramid] = InitializeAABB(XMINT3(2, 0, 2), XMFLOAT3(6, 6, 6));
		//	m_scene->addGeometry("FractalPyramid", make_shared<Geometry>(m_aabbs[offset + FractalPyramid], *m_deviceResources));
		//}
	}
}

void ProceduralRtxEngineSample::BuildInstancedProcedural()
{
	int N = 4;

	// Bottom-level AS with a single plane.
	Geometry::Instances instances;
	
	XMMATRIX mScale = XMMatrixScaling(20.f, 20.f, 20.f);

	//it iterates in a nxn grid
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; ++j)
		{
			for (int k = 0; k < N; ++k)
			{
				XMFLOAT3 float3(i, j, k);
				XMMATRIX mTranslation = XMMatrixTranslationFromVector(30.f * XMLoadFloat3(&float3));

				instances.push_back(mScale * mTranslation);
			}
		}
	}

	m_aabbs.push_back(D3D12_RAYTRACING_AABB{ -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f });
	m_scene->addGeometry("Mandelbulb", make_shared<Geometry>(m_aabbs[SignedDistancePrimitive::Mandelbulb], *m_deviceResources, instances));

	m_aabbs.push_back(D3D12_RAYTRACING_AABB{ -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f });
	m_scene->addGeometry("Pacman", make_shared<Geometry>(m_aabbs[SignedDistancePrimitive::IntersectedRoundCube], *m_deviceResources, instances));
}

void ProceduralRtxEngineSample::BuildInstancedParallelepipeds()
{
	XMFLOAT3 q = XMFLOAT3(-0.5f, 0.f, 0.f);
	XMFLOAT3 p = XMFLOAT3(0.5f, 0.f, 0.f);

	float d = 0.02;

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
		{ q01, XMFLOAT3(0.0f, 0.0f, 1.0f)  },
		{ q00, XMFLOAT3(0.0f, 1.0f, 0.0f)  },
		{ q10, XMFLOAT3(1.0f, 0.0f, 0.0f)  },
		{ q11, XMFLOAT3(0.0f, -1.0f, 0.0f) },

		{ p01, XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ p00, XMFLOAT3(0.0f, 0.0f, -1.0f) },
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

	int N = 4;

	// Bottom-level AS with a single plane.
	Geometry::Instances instances;
		
	// Scale in XZ dimensions.
	XMMATRIX mScale = XMMatrixScaling(20, 20, 20);

	//it iterates in a nxn grid
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			for (int k = 0; k < N; k++)
			{
				XMFLOAT3 globalTranslation(0.f, 0.f, 0.f);
				XMFLOAT3 float3(i, j, k);

				XMMATRIX mTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&globalTranslation) + (30. * XMLoadFloat3(&float3)));

				instances.push_back(mScale * mTranslation);
			}
		}
	}

	m_scene->addGeometry("GlobalGeometry", make_shared<Geometry>(vertices, indices, *m_deviceResources, *m_descriptorHeap, instances));
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

	m_scene->addGeometry("GlobalGeometry", make_shared<Geometry>(vertices, indices, *m_deviceResources, *m_descriptorHeap,
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
	m_input.newMousePos(XMFLOAT2(x, y));
}

void ProceduralRtxEngineSample::OnLeftButtonDown(UINT x, UINT y)
{
	m_input.setMouseButton(InputManager::LEFT, true);
	m_input.newMousePos(XMFLOAT2(x, y));
	m_input.newMousePos(XMFLOAT2(x, y));
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
}

// Update frame-based values.
void ProceduralRtxEngineSample::OnUpdate()
{
	m_timer.Tick();
	CalculateFrameStats();
	float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
	
	auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
	auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

	UpdateCameraMatrices(elapsedTime);

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
	UpdateAABBPrimitiveAttributes(m_animateGeometryTime);
	(*m_sceneCB)->elapsedTime = m_animateGeometryTime;
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
	m_aabbPrimitiveAttributeBuffer->Release();

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
		//float raytracingTime = static_cast<float>(m_rayTracingState->getGpuTimer().GetElapsedMS());
		float raytracingTime = 0.f;
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