#include "stdafx.h"
#include "MandelJuliaPacSceneBuilder.h"

MandelJuliaPacSceneBuilder::MandelJuliaPacSceneBuilder()
{
	m_scene = make_shared<StaticScene>();
	m_sceneCB = make_shared<ConstantBuffer<SceneConstantBuffer>>();
	m_instanceBuffer = make_shared<StructuredBuffer<InstanceBuffer>>();
}

// Initialize scene rendering parameters.
void MandelJuliaPacSceneBuilder::init(DeviceResourcesPtr deviceResources)
{
	auto frameIndex = deviceResources->GetCurrentFrameIndex();

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

void MandelJuliaPacSceneBuilder::build(DxrDevicePtr device, DeviceResourcesPtr deviceResources, DxrCommandListPtr commandList,
	DescriptorHeapPtr descriptorHeap, DescriptorHeap::DescriptorHandles& descriptorHandles)
{
	// Add rays to the scene.
	CreateRays();

	// Add hit groups to the scene.
	CreateHitGroups();

	// Build geometry to be used in the sample.
	BuildGeometry(deviceResources, descriptorHeap);

	// Create constant buffers for the geometry and the scene.
	CreateConstantBuffers(deviceResources);

	// Create AABB primitive attribute buffers.
	CreateInstanceBuffer(deviceResources);

	CreateAccelerationStructure(device, deviceResources, commandList);

	// Create root signatures for the shaders.
	CreateRootSignatures(deviceResources, descriptorHeap, descriptorHandles);

	CreateShaderTablesEntries(deviceResources);
}

// Create constant buffers.
void MandelJuliaPacSceneBuilder::CreateConstantBuffers(DeviceResourcesPtr deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	auto frameCount = deviceResources->GetBackBufferCount();

	m_sceneCB->Create(device, frameCount, L"Scene Constant Buffer");
}

// Create AABB primitive attributes buffers.
void MandelJuliaPacSceneBuilder::CreateInstanceBuffer(DeviceResourcesPtr deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	auto frameCount = deviceResources->GetBackBufferCount();

	UINT nProceduralInstances = 0;
	for (auto geometry : m_scene->getGeometry())
	{
		if (geometry->getType() == Geometry::Procedural)
		{
			for (auto instance : *geometry->getInstances())
			{
				++nProceduralInstances;
			}
		}
	}

	m_instanceBuffer->Create(device, nProceduralInstances, frameCount, L"AABB primitive attributes");
}

void MandelJuliaPacSceneBuilder::CreateRays()
{
	m_scene->addRay(make_shared<Ray>("Radiance", L"Miss", Payload(RayPayload())));
	m_scene->addRay(make_shared<Ray>("Shadow", L"Miss_Shadow", Payload(ShadowRayPayload())));
}

void MandelJuliaPacSceneBuilder::CreateHitGroups()
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

void MandelJuliaPacSceneBuilder::CreateAccelerationStructure(DxrDevicePtr device, DeviceResourcesPtr deviceResources, DxrCommandListPtr commandList)
{
	m_accelerationStruct = make_shared<AccelerationStructure>(m_scene, device, commandList, deviceResources);
}

void MandelJuliaPacSceneBuilder::CreateRootSignatures(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap,
	DescriptorHeap::DescriptorHandles& descriptorHandles)
{
	// Global root signature.
	auto globalSignature = make_shared<RootSignature>("GlobalSignature", deviceResources, descriptorHeap, false);

	// Global signature ranges.
	auto outputRange = globalSignature->createRange(descriptorHandles.gpu, RootSignature::UAV, 0, 1);
	auto globalGeometry = m_scene->getGeometryMap().at("GlobalGeometry");
	auto vertexRange = globalSignature->createRange(globalGeometry->getIndexBuffer().gpuDescriptorHandle, RootSignature::SRV, 1, 2);

	// Global signature entries.
	descriptorHandles.baseHandleIndex = globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{outputRange});
	globalSignature->addEntry(RootComponent(DontApply()), RootSignature::SRV, m_accelerationStruct->getBuilded(), 0);
	globalSignature->addEntry(RootComponent(SceneConstantBuffer()), RootSignature::CBV, m_sceneCB, 0);
	globalSignature->addEntry(RootComponent(InstanceBuffer()), RootSignature::SRV, m_instanceBuffer, 3);
	globalSignature->addDescriptorTable(vector<RootSignature::DescriptorRange>{vertexRange});

	m_scene->addGlobalSignature(globalSignature);

	// Triangle geometry local root signature.
	auto triangleSignature = make_shared<RootSignature>("Triangle", deviceResources, descriptorHeap, true);
	triangleSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);

	// Root Arguments type.
	triangleSignature->setRootArgumentsType(RootArguments(TriangleRootArguments()));
	m_scene->addLocalSignature(triangleSignature);

	// Procedural geometry local root signature.
	auto proceduralSignature = make_shared<RootSignature>("Procedural", deviceResources, descriptorHeap, true);
	proceduralSignature->addConstant(RootComponent(PrimitiveConstantBuffer()), 1);
	proceduralSignature->addConstant(RootComponent(PrimitiveInstanceConstantBuffer()), 2);

	// Root Arguments.
	proceduralSignature->setRootArgumentsType(RootArguments(ProceduralRootArguments()));
	m_scene->addLocalSignature(proceduralSignature);
}

void MandelJuliaPacSceneBuilder::CreateShaderTablesEntries(DeviceResourcesPtr deviceResources)
{
	m_shaderTable = make_shared<RtxEngine::ShaderTable>(m_scene, deviceResources);

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

void MandelJuliaPacSceneBuilder::BuildInstancedProcedural(DeviceResourcesPtr deviceResources)
{
	int N = 1;

	// Bottom-level AS with a single plane.
	Geometry::Instances mandelbulbInstances;
	Geometry::Instances pacManInstances;
	Geometry::Instances juliaInstances;

	//XMMATRIX scale = XMMatrixScaling(15.f, 15.f, 15.f);

	//it iterates in a nxn grid
	for (float i = 0; i < N; i += 3)
	{
		for (float j = 0; j < N; ++j)
		{
			for (float k = 0; k < N; ++k)
			{
				{
					//XMFLOAT3 float3(i + 2, j, k);
					XMFLOAT3 float3(i, j - 1., k + 2.3);
					XMMATRIX translation = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3));

					XMMATRIX mandelbulbScale = XMMatrixScaling(40.f, 40.f, 40.f);

					mandelbulbInstances.push_back(mandelbulbScale * /*rotation **/ translation);
				}

				{
					//XMFLOAT3 float3(i + 1, j, k);
					/*XMFLOAT3 float3_1(i, j, k);
					XMMATRIX translation1 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_1));*/

					XMFLOAT3 float3_2(i + 0.7, j - 0.7, k);
					XMMATRIX translation2 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_2));

					XMFLOAT3 float3_3(i - 0.7, j - 0.7, k);
					XMMATRIX translation3 = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3_3));

					XMMATRIX pacmanScale = XMMatrixScaling(50.f, 50.f, 50.f);

					//pacManInstances.push_back(pacmanScale * /* rotation **/ translation1);
					pacManInstances.push_back(pacmanScale * /* rotation **/ translation2);
					pacManInstances.push_back(pacmanScale * /* rotation **/ translation3);
				}

				{
					XMFLOAT3 float3(i, j + 2.6, k + 1.5);
					XMMATRIX rotation = XMMatrixRotationX(3.1421f);
					XMMATRIX translation = XMMatrixTranslationFromVector(50.f * XMLoadFloat3(&float3));

					XMMATRIX juliaScale = XMMatrixScaling(100.f, 30.f, 100.f);

					juliaInstances.push_back(juliaScale * rotation * translation);
				}
			}
		}
	}


	D3D12_RAYTRACING_AABB juliaAABB{ -3.5f, -3.5f, -3.5f, 3.5f, 3.5f, 3.5f };
	m_scene->addGeometry(make_shared<Geometry>("Julia", juliaAABB, *deviceResources, juliaInstances));

	D3D12_RAYTRACING_AABB pacmanAABB{ -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	m_scene->addGeometry(make_shared<Geometry>("Pacman", pacmanAABB, *deviceResources, pacManInstances));

	D3D12_RAYTRACING_AABB mandelbulbAABB{ -1.5f, -1.5f, -1.5f, 1.5f, 1.5f, 1.5f };
	m_scene->addGeometry(make_shared<Geometry>("Mandelbulb", mandelbulbAABB, *deviceResources, mandelbulbInstances));
}

void MandelJuliaPacSceneBuilder::BuildInstancedParallelepipeds(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap)
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
				XMFLOAT3 float3(i, j + 0.5f, k);

				XMMATRIX mTranslation = XMMatrixTranslationFromVector(XMLoadFloat3(&globalTranslation) + (30. * XMLoadFloat3(&float3)));

				instances.push_back(mScale * mTranslation);
			}
		}
	}

	m_scene->addGeometry(make_shared<Geometry>("GlobalGeometry", vertices, indices, *deviceResources, *descriptorHeap, instances));
}

void MandelJuliaPacSceneBuilder::BuildPlaneGeometry(const XMFLOAT3& width, DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap)
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

	m_scene->addGeometry(make_shared<Geometry>("GlobalGeometry", vertices, indices, *deviceResources, *descriptorHeap,
		Geometry::Instances(1, triangleBlasTransform)));
}

// Build geometry used in the sample.
void MandelJuliaPacSceneBuilder::BuildGeometry(DeviceResourcesPtr deviceResources, DescriptorHeapPtr descriptorHeap)
{
	BuildInstancedParallelepipeds(deviceResources, descriptorHeap);
	BuildInstancedProcedural(deviceResources);
}

void MandelJuliaPacSceneBuilder::release()
{
	m_sceneCB->Release();
	m_instanceBuffer->Release();
}