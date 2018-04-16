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
#include "D3D12RaytracingProceduralGeometry.h"
#include "CompiledShaders\Raytracing.hlsl.h"

using namespace std;
using namespace DX;

// Shader entry points.
const wchar_t* D3D12RaytracingProceduralGeometry::c_raygenShaderName = L"MyRaygenShader";
const wchar_t* D3D12RaytracingProceduralGeometry::c_intersectionShaderNames[] =
{
    L"MyIntersectionShader_AABB", L"MyIntersectionShader_Sphere", L"MyIntersectionShader_Spheres"
};
const wchar_t* D3D12RaytracingProceduralGeometry::c_closestHitShaderNames[][RayType::Count] =
{
    { L"MyClosestHitShader_Triangle", L"MyClosestHitShader_ShadowRayTriangle" },
    { L"MyClosestHitShader_AABB", L"MyClosestHitShader_ShadowRayAABB" },
};
const wchar_t* D3D12RaytracingProceduralGeometry::c_missShaderNames[] =
{
    L"MyMissShader", L"MyMissShader_Shadow"
};

// Hit groups.
const wchar_t* D3D12RaytracingProceduralGeometry::c_hitGroupNames_TriangleGeometry[] = 
{ 
    L"MyHitGroup_Triangle", L"MyHitGroup_ShadowRayTriangle" 
};
const wchar_t* D3D12RaytracingProceduralGeometry::c_hitGroupNames_AABBGeometry[][RayType::Count] = 
{
    { L"MyHitGroup_AABB_AABB", L"MyHitGroup_ShadowRayAABB_AABB" },
    { L"MyHitGroup_AABB_Sphere", L"MyHitGroup_ShadowRayAABB_Sphere" },
    { L"MyHitGroup_AABB_Spheres", L"MyHitGroup_ShadowRayAABB_Spheres" },
};

D3D12RaytracingProceduralGeometry::D3D12RaytracingProceduralGeometry(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_raytracingOutputResourceUAVDescriptorHeapIndex(UINT_MAX),
    m_curRotationAngleRad(0.0f)
{
    // DXR is an experimental feature and needs to be enabled before creating a D3D12 device.
    m_isDxrSupported = EnableRaytracing();

    if (!m_isDxrSupported)
    {
        OutputDebugString(
            L"Could not enable raytracing driver (D3D12EnableExperimentalFeatures() failed).\n" \
            L"Possible reasons:\n" \
            L"  1) your OS is not in developer mode.\n" \
            L"  2) your GPU driver doesn't match the D3D12 runtime loaded by the app (d3d12.dll and friends).\n" \
            L"  3) your D3D12 runtime doesn't match the D3D12 headers used by your app (in particular, the GUID passed to D3D12EnableExperimentalFeatures).\n\n");

        OutputDebugString(L"Enabling compute based fallback raytracing support.\n");
        ThrowIfFalse(EnableComputeRaytracingFallback(), L"Could not enable compute based fallback raytracing support (D3D12EnableExperimentalFeatures() failed).\n");
    }

    m_forceComputeFallback = false;
    SelectRaytracingAPI(RaytracingAPI::FallbackLayer);

    m_deviceResources = std::make_unique<DeviceResources>(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_UNKNOWN,
        FrameCount,
        D3D_FEATURE_LEVEL_11_0,
        DeviceResources::c_AllowTearing
        );
    m_deviceResources->RegisterDeviceNotify(this);

    // Sample shows handling of use cases with tearing support, which is OS dependent and has been supported since Threshold II.
    // Since the Fallback Layer requires Fall Creator's update (RS3), we don't need to handle non-tearing cases.
    if (!m_deviceResources->IsTearingSupported())
    {
        OutputDebugString(L"Sample must be run on an OS with tearing support.\n");
        exit(EXIT_FAILURE);
    }

    UpdateForSizeChange(width, height);
}

void D3D12RaytracingProceduralGeometry::OnInit()
{
    m_deviceResources->SetWindow(Win32Application::GetHwnd(), m_width, m_height);

    m_deviceResources->CreateDeviceResources();
    m_deviceResources->CreateWindowSizeDependentResources();

    InitializeScene();

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Update camera matrices passed into the shader.
void D3D12RaytracingProceduralGeometry::UpdateCameraMatrices()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    m_sceneCB->cameraPosition = m_eye;
    float fovAngleY = 45.0f;
    XMMATRIX view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), m_aspectRatio, 0.01f, 125.0f);
    XMMATRIX viewProj = view * proj;
    m_sceneCB->projectionToWorld = XMMatrixInverse(nullptr, viewProj);
}

// Update AABB primite attributes buffers passed into the shader.
void D3D12RaytracingProceduralGeometry::UpdateAABBPrimitiveAttributes()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    const float aabbDefaultWidth = 2;    // Default AABB is <-1,1>^3
    //const float c_aabbWidth  = 2;// 1 / sqrt(2.0f);// Width of each AABB - scaled down to fit any AABB rotation within the default width
    const float aabbDistanceStride = c_aabbWidth  + c_aabbDistance;
    const XMVECTOR vAABBstride = XMLoadFloat3(&XMFLOAT3(aabbDistanceStride, aabbDistanceStride, aabbDistanceStride));

    // ToDo scale for transformation to fit within <-1,1>
    float scaleRatio = c_aabbWidth  / aabbDefaultWidth;
    XMMATRIX mScale = XMMatrixScaling(scaleRatio, scaleRatio, scaleRatio);
   
    const XMVECTOR vBasePosition = XMLoadFloat3(&XMFLOAT3(
        -((NUM_AABB_X-1) * c_aabbWidth  + (NUM_AABB_X - 1) * c_aabbDistance) / 2.0f,
        -((NUM_AABB_Y-1) * c_aabbWidth  + (NUM_AABB_Y - 1) * c_aabbDistance) / 2.0f,
        -((NUM_AABB_Z-1) * c_aabbWidth  + (NUM_AABB_Z - 1) * c_aabbDistance) / 2.0f));

    const float elapsedTime = static_cast<float>(m_timer.GetTotalSeconds());
    for (UINT z = 0, i = 0; z < NUM_AABB_Z; z++)
    {
        for (UINT y = 0; y < NUM_AABB_Y; y++)
        {
            for (UINT x = 0; x < NUM_AABB_X; x++, i++)
            {
                auto& aabbAttributes = m_aabbPrimitiveAttributeBuffer[i];
                XMVECTOR vIndex = XMLoadUInt3(&XMUINT3(x, y, z));
                XMVECTOR vTranslation = vBasePosition + vIndex * vAABBstride;
                // ToDo TotalSeconds may run out of precision after some time
                XMMATRIX mRotation =  XMMatrixRotationZ(elapsedTime/2.0f*(x + y + z) * XM_2PI / NUM_AABB);// XMConvertToRadians(XMVectorGetX(XMVector3Length(vTranslation))));
                XMMATRIX mTranslation = XMMatrixTranslationFromVector(vTranslation);
                XMMATRIX mTransform = mScale * mRotation * mTranslation;

                aabbAttributes.localSpaceToBottomLevelAS = mTransform;
                aabbAttributes.bottomLevelASToLocalSpace = XMMatrixInverse(nullptr, mTransform);
           }
        }
    }
}

// Initialize scene rendering parameters.
void D3D12RaytracingProceduralGeometry::InitializeScene()
{
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    // Setup materials.
    {
        m_planeMaterialCB.albedo = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
        m_aabbMaterialCB[IntersectionShaderType::AABB].albedo = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
        m_aabbMaterialCB[IntersectionShaderType::Sphere].albedo = XMFLOAT4(0.8f, 0.8f, 0.5f, 1.0f);
        m_aabbMaterialCB[IntersectionShaderType::Spheres].albedo = XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
    }

    // Setup camera.
    {
        // Initialize the view and projection inverse matrices.
        m_eye = { 0.0f, 7.0f, -18.0f, 1.0f };
        m_at = { 0.0f, 0.0f, 0.0f, 1.0f };
        XMVECTOR right = { 1.0f, 0.0f, 0.0f, 0.0f };

        XMVECTOR direction = XMVector4Normalize(m_at - m_eye);
        m_up = XMVector3Normalize(XMVector3Cross(direction, right));

        // Rotate camera around Y axis.
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(45.0f));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);

        UpdateCameraMatrices();
    }

    // Setup lights.
    {
        // Initialize the lighting parameters.
        XMFLOAT4 lightPosition;
        XMFLOAT4 lightAmbientColor;
        XMFLOAT4 lightDiffuseColor;

        lightPosition = XMFLOAT4(0.0f, 18.0f, -30.0f, 0.0f);
        m_sceneCB->lightPosition = XMLoadFloat4(&lightPosition);
        m_sceneCB->lightPosition = XMLoadFloat4(&lightPosition);

        lightAmbientColor = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
        m_sceneCB->lightAmbientColor = XMLoadFloat4(&lightAmbientColor);

        lightDiffuseColor = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        m_sceneCB->lightDiffuseColor = XMLoadFloat4(&lightDiffuseColor);
    }
}

// Create constant buffers.
void D3D12RaytracingProceduralGeometry::CreateConstantBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();

    m_sceneCB.Create(device, frameCount, L"Scene Constant Buffer");
}

// Create AABB primitive attributes buffers.
void D3D12RaytracingProceduralGeometry::CreateAABBPrimitiveAttributesBuffers()
{
    // ToDo move this out
    auto device = m_deviceResources->GetD3DDevice();
    auto frameCount = m_deviceResources->GetBackBufferCount();
    m_aabbPrimitiveAttributeBuffer.Create(device, NUM_AABB, frameCount, L"AABB primitive attributes");
}

// Create resources that depend on the device.
void D3D12RaytracingProceduralGeometry::CreateDeviceDependentResources()
{
    // Initialize raytracing pipeline.

    // Create raytracing interfaces: raytracing device and commandlist.
    CreateRaytracingInterfaces();

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    // Create a heap for descriptors.
    CreateDescriptorHeap();

    // Build geometry to be used in the sample.
    BuildGeometry();

    // Build raytracing acceleration structures from the generated geometry.
    BuildAccelerationStructures();

    // Create constant buffers for the geometry and the scene.
    CreateConstantBuffers();

    // Create AABB primitive attribute buffers.
    CreateAABBPrimitiveAttributesBuffers();

    // Build shader tables, which define shaders and their local root arguments.
    BuildShaderTables();

    // Create an output 2D texture to store the raytracing result to.
    CreateRaytracingOutputResource();
}

void D3D12RaytracingProceduralGeometry::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
    auto device = m_deviceResources->GetD3DDevice();
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        ThrowIfFailed(m_fallbackDevice->D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
        ThrowIfFailed(m_fallbackDevice->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
        ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
    }
}

void D3D12RaytracingProceduralGeometry::CreateRootSignatures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE ranges[2]; // Perfomance TIP: Order from most frequent to least frequent.
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // 1 output texture
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);  // 2 static index and vertex buffers.

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignature::Slot::Count];
        rootParameters[GlobalRootSignature::Slot::OutputView].InitAsDescriptorTable(1, &ranges[0]);
        rootParameters[GlobalRootSignature::Slot::AccelerationStructure].InitAsShaderResourceView(0);
        rootParameters[GlobalRootSignature::Slot::SceneConstant].InitAsConstantBufferView(0);
        rootParameters[GlobalRootSignature::Slot::AABBattributeBuffer].InitAsShaderResourceView(3);
        rootParameters[GlobalRootSignature::Slot::VertexBuffers].InitAsDescriptorTable(1, &ranges[1]);
        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // ToDo check if FL can run without Local Root Sig for raygen and miss

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
#if USE_NON_NULL_LOCAL_ROOT_SIG 
        // Empty root signature
        {
            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(D3D12_DEFAULT);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Empty]);
        }
#endif
        // Triangle geometry
        {
            namespace RootSignatureSlots = LocalRootSignature::Triangle::Slot;
            CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
            rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(MaterialConstantBuffer), 1);

            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle]);
        }

        // AABB geometry
        {
            namespace RootSignatureSlots = LocalRootSignature::AABB::Slot;
            CD3DX12_ROOT_PARAMETER rootParameters[RootSignatureSlots::Count];
            rootParameters[RootSignatureSlots::MaterialConstant].InitAsConstants(SizeOfInUint32(MaterialConstantBuffer), 1);
            rootParameters[RootSignatureSlots::GeometryIndex].InitAsConstants(SizeOfInUint32(AABBConstantBuffer), 2);

            CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
            SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB]);
        }
    }
}

// Create raytracing device and command list.
void D3D12RaytracingProceduralGeometry::CreateRaytracingInterfaces()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();

    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        CreateRaytracingFallbackDeviceFlags createDeviceFlags = m_forceComputeFallback ?
            CreateRaytracingFallbackDeviceFlags::ForceComputeFallback :
            CreateRaytracingFallbackDeviceFlags::None;
        ThrowIfFailed(D3D12CreateRaytracingFallbackDevice(device, createDeviceFlags, 0, IID_PPV_ARGS(&m_fallbackDevice)));
        m_fallbackDevice->QueryRaytracingCommandList(commandList, IID_PPV_ARGS(&m_fallbackCommandList));
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(device->QueryInterface(__uuidof(ID3D12DeviceRaytracingPrototype), &m_dxrDevice), L"Couldn't get DirectX Raytracing interface for the device.\n");
        ThrowIfFailed(commandList->QueryInterface(__uuidof(ID3D12CommandListRaytracingPrototype), &m_dxrCommandList), L"Couldn't get DirectX Raytracing interface for the command list.\n");
    }
}

// DXIL library
// This contains the shaders and their entrypoints for the state object.
// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
void D3D12RaytracingProceduralGeometry::CreateDxilLibrarySubobject(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
    auto lib = raytracingPipeline->CreateSubobject<CD3D12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void *)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
#if DEFINE_EXPLICIT_SHADER_EXPORTS
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be ommited for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(c_raygenShaderName);
        DefineExports(lib, c_intersectionShaderNames);
        DefineExports(lib, c_closestHitShaderNames);
        DefineExports(lib, c_missShaderNames);
    }
#else
    // Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
#endif
}

// Hit groups
// A hit group specifies closest hit, any hit and intersection shaders 
// to be executed when a ray intersects the geometry.
void D3D12RaytracingProceduralGeometry::CreateHitGroupSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Triangle geometry hit groups
    {
        for (UINT rayType = 0; rayType < RayType::Count; rayType++)
        {
            auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
            hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::Triangle][rayType]);
            hitGroup->SetHitGroupExport(c_hitGroupNames_TriangleGeometry[rayType]);
        }
    }

    // AABB geometry hit groups
    {
        // Create hit groups for each intersection shader.
        for (UINT aabbType = 0; aabbType < IntersectionShaderType::Count; aabbType++)
            for (UINT rayType = 0; rayType < RayType::Count; rayType++)
            {
                auto hitGroup = raytracingPipeline->CreateSubobject<CD3D12_HIT_GROUP_SUBOBJECT>();
                hitGroup->SetIntersectionShaderImport(c_intersectionShaderNames[aabbType]);
                hitGroup->SetClosestHitShaderImport(c_closestHitShaderNames[GeometryType::AABB][rayType]);
                hitGroup->SetHitGroupExport(c_hitGroupNames_AABBGeometry[aabbType][rayType]);
            }
    }
}

// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void D3D12RaytracingProceduralGeometry::CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline)
{
#if USE_NON_NULL_LOCAL_ROOT_SIG 
    // Empty
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Empty].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
        rootSignatureAssociation->AddExports(c_missShaderNames);
    }
#endif
    // Triangle geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::Triangle].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExports(c_hitGroupNames_TriangleGeometry);
    }

    // AABB geometry
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3D12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature[LocalRootSignature::Type::AABB].Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        for (auto& hitGroupsForIntersectionShaderType : c_hitGroupNames_AABBGeometry)
        {
            rootSignatureAssociation->AddExports(hitGroupsForIntersectionShaderType);
        }
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void D3D12RaytracingProceduralGeometry::CreateRaytracingPipelineStateObject()
{
    // Create 17 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 8 - Hit groups - 4 geometries (1 triangle 3 aabb) x 2 ray types (ray, shadowRay)
    // 1 - Shader config
    // 6 - 3 x Local root signature and association
    // 1 - Global root signature
    // 1 - Pipeline config
#if USE_NON_NULL_LOCAL_ROOT_SIG
    const UINT NUM_SUBOBJECTS = 18;
#else
    const UINT NUM_SUBOBJECTS = 15;
#endif
    CD3D12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    // DXIL library
    CreateDxilLibrarySubobject(&raytracingPipeline);

    // Hit groups
    CreateHitGroupSubobjects(&raytracingPipeline);

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = max(sizeof(RayPayload), sizeof(ShadowRayPayload));
    UINT attributeSize = sizeof(MyAttributes);
    shaderConfig->Config(payloadSize, attributeSize);

    // Local root signature and shader association
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    CreateLocalRootSignatureSubobjects(&raytracingPipeline);

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3D12_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3D12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed
    // as drivers may apply optimization strategies for low recursion depths.
    UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
    pipelineConfig->Config(maxRecursionDepth);

    // Debug
    assert(raytracingPipeline.NumSubbojects() == NUM_SUBOBJECTS && L"Checking expeted num subobjects here. Num subobjects doesn't match. Update RTPSO description.");
    PrintStateObjectDesc(raytracingPipeline);

    // Create the state object.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        ThrowIfFailed(m_fallbackDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_fallbackStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
    else // DirectX Raytracing
    {
        ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
    }
}

// Create a 2D output texture for raytracing.
void D3D12RaytracingProceduralGeometry::CreateRaytracingOutputResource()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto backbufferFormat = m_deviceResources->GetBackBufferFormat();

    // Create the output resource. The dimensions and format should match the swap-chain.
    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_raytracingOutput)));
    NAME_D3D12_OBJECT(m_raytracingOutput);

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;
    m_raytracingOutputResourceUAVDescriptorHeapIndex = AllocateDescriptor(&uavDescriptorHandle, m_raytracingOutputResourceUAVDescriptorHeapIndex);
    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    device->CreateUnorderedAccessView(m_raytracingOutput.Get(), nullptr, &UAVDesc, uavDescriptorHandle);
    m_raytracingOutputResourceUAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), m_raytracingOutputResourceUAVDescriptorHeapIndex, m_descriptorSize);
}

void D3D12RaytracingProceduralGeometry::CreateDescriptorHeap()
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    // Allocate a heap for 6 descriptors:
    // 2 - vertex and index  buffer SRVs
    // 1 - raytracing output texture SRV
    // 3 - 2x bottom and a top level acceleration structure fallback wrapped pointer UAVs
    descriptorHeapDesc.NumDescriptors = 6;
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    descriptorHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_descriptorHeap));
    NAME_D3D12_OBJECT(m_descriptorHeap);

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

// Build AABBs for procedural geometry within a bottom-level acceleration structure.
void D3D12RaytracingProceduralGeometry::BuildProceduralGeometryAABBs()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Create a grid of AABBs.
    {
        const float aabbBaseWidth = 2;                  // Width of each AABB
        const float aabbRotationBloat = 1;// sqrt(2.0f);     // Size bloat for AABB to encapsule any rotations of base AABB
        const float c_aabbWidth  = aabbBaseWidth * aabbRotationBloat;

        const XMFLOAT3 basePosition =
        {
            -(NUM_AABB_X * c_aabbWidth  + (NUM_AABB_X - 1) * c_aabbDistance) / 2.0f,
            -(NUM_AABB_Y * c_aabbWidth  + (NUM_AABB_Y - 1) * c_aabbDistance) / 2.0f,
            -(NUM_AABB_Z * c_aabbWidth  + (NUM_AABB_Z - 1) * c_aabbDistance) / 2.0f,
        };

        // ToDo This is calculated twice - here and in update AABB
        D3D12_RAYTRACING_AABB aabb[NUM_AABB_Z][NUM_AABB_Y][NUM_AABB_X];
        for (UINT z = 0; z < NUM_AABB_Z; z++)
        {
            FLOAT minZ = basePosition.z + z * (c_aabbWidth  + c_aabbDistance);
            for (UINT y = 0; y < NUM_AABB_Y; y++)
            {
                FLOAT minY = basePosition.y + y * (c_aabbWidth  + c_aabbDistance);
                for (UINT x = 0; x < NUM_AABB_X; x++)
                {
                    FLOAT minX = basePosition.x + x * (c_aabbWidth  + c_aabbDistance);
                    aabb[z][y][x] =
                    {
                        minX, minY, minZ, minX + c_aabbWidth , minY + c_aabbWidth , minZ + c_aabbWidth 
                    };
                }
            }
        }
        AllocateUploadBuffer(device, &aabb, sizeof(aabb), &m_aabbBuffer.resource);
    }
}

void D3D12RaytracingProceduralGeometry::BuildPlaneGeometry()
{
    auto device = m_deviceResources->GetD3DDevice();
    // Plane indices.
    Index indices[] =
    {
        3,1,0,
        2,1,3,

    };

    // Cube vertices positions and corresponding triangle normals.
    // ToDo use scale transformation
    Vertex vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
    };

    AllocateUploadBuffer(device, indices, sizeof(indices), &m_indexBuffer.resource);
    AllocateUploadBuffer(device, vertices, sizeof(vertices), &m_vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = CreateBufferSRV(&m_indexBuffer, sizeof(indices) / 4, sizeof(UINT));
    UINT descriptorIndexVB = CreateBufferSRV(&m_vertexBuffer, ARRAYSIZE(vertices), sizeof(vertices[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
}

// Build geometry used in the sample.
void D3D12RaytracingProceduralGeometry::BuildGeometry()
{
    BuildProceduralGeometryAABBs();
    BuildPlaneGeometry();
}

// Build geometry descs for bottom-level AS.
void D3D12RaytracingProceduralGeometry::BuildGeometryDescsForBottomLevelAS(array<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count>& geometryDescs)
{
    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    // Triangle geometry desc
    {
        // Triangle bottom-level AS contains a single plane geometry.
        geometryDescs[BottomLevelASType::Triangle].resize(1);
        
        // Plane geometry
        auto& geometryDesc = geometryDescs[BottomLevelASType::Triangle][0];
        geometryDesc = {};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Triangles.IndexBuffer = m_indexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.IndexCount = static_cast<UINT>(m_indexBuffer.resource->GetDesc().Width) / sizeof(Index);
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = static_cast<UINT>(m_vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
        geometryDesc.Triangles.VertexBuffer.StartAddress = m_vertexBuffer.resource->GetGPUVirtualAddress();
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
        geometryDesc.Flags = geometryFlags;
    }

    // AABB geometry desc
    {
        D3D12_RAYTRACING_GEOMETRY_DESC aabbDescTemplate = {};
        aabbDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
        aabbDescTemplate.AABBs.AABBCount = 1;
        aabbDescTemplate.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
        aabbDescTemplate.Flags = geometryFlags;

        // AABB bottom-level AS contains one geometry per intersection shader type.
        geometryDescs[BottomLevelASType::AABB].resize(IntersectionShaderType::Count, aabbDescTemplate);

        // Create AABB geometries. 
        // Having separate geometries allows of separate shader record binding per geometry.
        // In this sample, this lets sample specify custom hit groups per AABB geometry.
        for (UINT i = 0; i < IntersectionShaderType::Count; i++)
        {
            auto& geometryDesc = geometryDescs[BottomLevelASType::AABB][i];
            geometryDesc.AABBs.AABBs.StartAddress = m_aabbBuffer.resource->GetGPUVirtualAddress() + i * sizeof(D3D12_RAYTRACING_AABB);
        }
    }
}

AccelerationStructureBuffers D3D12RaytracingProceduralGeometry::BuildBottomLevelAS(const vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> bottomLevelAS;

    // Get the size requirements for the scratch and AS buffers.
    D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
    prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildInfoDesc.Flags = buildFlags;
    prebuildInfoDesc.NumDescs = static_cast<UINT>(geometryDescs.size());
    prebuildInfoDesc.pGeometryDescs = geometryDescs.data();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
    }
    else // DirectX Raytracing
    {
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &bottomLevelPrebuildInfo);
    }
    ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    // Create a scratch buffer.
    AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn�t need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState;
        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
        }
        else // DirectX Raytracing
        {
            initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAS, initialResourceState, L"BottomLevelAccelerationStructure");
    }

    // Bottom-level AS desc.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    {
        bottomLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelBuildDesc.Flags = buildFlags;
        bottomLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
        bottomLevelBuildDesc.DestAccelerationStructureData = { bottomLevelAS->GetGPUVirtualAddress(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes };
        bottomLevelBuildDesc.NumDescs = static_cast<UINT>(geometryDescs.size());
        bottomLevelBuildDesc.pGeometryDescs = geometryDescs.data();
    }
    
    // Build the acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        // Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
        ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
        m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
        m_fallbackCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
    }
    else // DirectX Raytracing
    {
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc);
    }

    AccelerationStructureBuffers bottomLevelASBuffers;
    bottomLevelASBuffers.accelerationStructure = bottomLevelAS;
    bottomLevelASBuffers.scratch = scratch;
    bottomLevelASBuffers.ResultDataMaxSizeInBytes = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
    return bottomLevelASBuffers;
}

// ToDo should the comptr be passed by value?
template <class InstanceDescType, class BLASPtrType>
void D3D12RaytracingProceduralGeometry::BuildBotomLevelASInstanceDescs(BLASPtrType *bottomLevelASaddresses, ComPtr<ID3D12Resource>* instanceDescsResource)
{
    auto device = m_deviceResources->GetD3DDevice();
    
    vector<InstanceDescType> instanceDescs;
    instanceDescs.resize(NUM_BLAS);

    // Width of a bottom-level AS geometry
    const XMFLOAT3 fWidth = XMFLOAT3(
        NUM_AABB_X * c_aabbWidth + (NUM_AABB_X - 1) * c_aabbDistance,
        NUM_AABB_Y * c_aabbWidth + (NUM_AABB_Y - 1) * c_aabbDistance,
        NUM_AABB_Z * c_aabbWidth + (NUM_AABB_Z - 1) * c_aabbDistance);
    const XMVECTOR vWidth = XMLoadFloat3(&fWidth);
    const XMVECTOR vStride = vWidth + XMLoadFloat3(&XMFLOAT3(c_aabbDistance, c_aabbDistance, c_aabbDistance));


    // Bottom-level AS with a single plane.
    {
        auto& instanceDesc = instanceDescs[0];
        instanceDesc = {};
        instanceDesc.InstanceMask = 1;
        instanceDesc.InstanceContributionToHitGroupIndex = BottomLevelASType::Triangle * RayType::Count * IntersectionShaderType::Count;
        instanceDesc.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::Triangle];

        // Calculate transformation matrix.
        const XMVECTOR vInstancesScale = XMLoadUInt3(&XMUINT3(NUM_INSTANCE_X, NUM_INSTANCE_Y, NUM_INSTANCE_Z));
        const XMVECTOR vBasePosition = vStride * vInstancesScale * XMLoadFloat3(&XMFLOAT3(-0.5f, 0.0f, -0.5f));
        
        // Scale in XZ dimensions.
        XMMATRIX mScale = XMMatrixScaling(XMVectorGetByIndex(vStride*vInstancesScale, 0), 1.0f, XMVectorGetByIndex(vStride*vInstancesScale, 2));
        
        XMMATRIX mTranslation = XMMatrixTranslationFromVector(vBasePosition);
        XMMATRIX mTransform = mScale * mTranslation;         
        StoreXMMatrixAsTransform3x4(instanceDesc.Transform, mTransform);
    }

    // Create instanced bottom-level AS with procedural geometry AABBs.
    // Instances share all the data, except for a transform.
    {
        const XMVECTOR vBasePosition = XMLoadFloat3(&XMFLOAT3(
            -((NUM_INSTANCE_X - 1) * (fWidth.x + c_aabbDistance) / 2.0f),
            1.0f,
            -((NUM_INSTANCE_Z - 1) * (fWidth.z + c_aabbDistance) / 2.0f)));

        InstanceDescType instanceDescTemplate = {};
        instanceDescTemplate.InstanceMask = 1;
        // ToDo explain the hitgroupindex offset
        instanceDescTemplate.InstanceContributionToHitGroupIndex = BottomLevelASType::AABB * RayType::Count;
        instanceDescTemplate.AccelerationStructure = bottomLevelASaddresses[BottomLevelASType::AABB];

        UINT blasIndex = 1;
        for (UINT x = 0; x < NUM_INSTANCE_X; x++)
            for (UINT y = 0; y < NUM_INSTANCE_Y; y++)
                for (UINT z = 0; z < NUM_INSTANCE_Z; z++, blasIndex++)
                {
                    auto& instanceDesc = instanceDescs[blasIndex];
                    instanceDesc = instanceDescTemplate;

                    XMVECTOR vIndex = XMLoadUInt3(&XMUINT3(x, y, z));
                    XMVECTOR vTranslation = vBasePosition + vIndex * vStride;
                    XMMATRIX mTranslation = XMMatrixTranslationFromVector(vTranslation);
                    StoreXMMatrixAsTransform3x4(instanceDesc.Transform, mTranslation);
                }
    }
    UINT64 bufferSize = static_cast<UINT64>(instanceDescs.size() * sizeof(instanceDescs[0]));
    AllocateUploadBuffer(device, instanceDescs.data(), bufferSize, &(*instanceDescsResource), L"InstanceDescs");
};


AccelerationStructureBuffers D3D12RaytracingProceduralGeometry::BuildTopLevelAS(AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count], D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    ComPtr<ID3D12Resource> scratch;
    ComPtr<ID3D12Resource> topLevelAS;

    // Get required sizes for an acceleration structure.
    D3D12_GET_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO_DESC prebuildInfoDesc = {};
    prebuildInfoDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    prebuildInfoDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildInfoDesc.Flags = buildFlags;
    prebuildInfoDesc.NumDescs = NUM_BLAS;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
    }
    else // DirectX Raytracing
    {
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildInfoDesc, &topLevelPrebuildInfo);
    }
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratch, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn�t need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState;
        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
        }
        else // DirectX Raytracing
        {
            initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAS, initialResourceState, L"TopLevelAccelerationStructure");
    }

    // Note on Emulated GPU pointers (AKA Wrapped pointers) requirement in Fallback Layer:
    // The primary point of divergence between the DXR API and the compute-based Fallback layer is the handling of GPU pointers. 
    // DXR fundamentally requires that GPUs be able to dynamically read from arbitrary addresses in GPU memory. 
    // The existing Direct Compute API today is more rigid than DXR and requires apps to explicitly inform the GPU what blocks of memory it will access with SRVs/UAVs.
    // In order to handle the requirements of DXR, the Fallback Layer uses the concept of Emulated GPU pointers, 
    // which requires apps to create views around all memory they will access for raytracing, 
    // but retains the DXR-like flexibility of only needing to bind the top level acceleration structure at DispatchRays.
    //
    // The Fallback Layer interface uses WRAPPED_GPU_POINTER to encapsulate the underlying pointer
    // which will either be an emulated GPU pointer for the compute - based path or a GPU_VIRTUAL_ADDRESS for the DXR path.

    // Create instance descs for the bottom-level acceleration structures.
    ComPtr<ID3D12Resource> instanceDescsResource;
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
        WRAPPED_GPU_POINTER bottomLevelASaddresses[BottomLevelASType::Count] = 
        {
            CreateFallbackWrappedPointer(bottomLevelAS[0].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[0].ResultDataMaxSizeInBytes) / sizeof(UINT32)),
            CreateFallbackWrappedPointer(bottomLevelAS[1].accelerationStructure.Get(), static_cast<UINT>(bottomLevelAS[1].ResultDataMaxSizeInBytes) / sizeof(UINT32))
        };
        BuildBotomLevelASInstanceDescs<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC>(bottomLevelASaddresses, &instanceDescsResource);
    }
    else // DirectX Raytracing
    {
        D3D12_RAYTRACING_INSTANCE_DESC instanceDescs[BottomLevelASType::Count] = {};
        D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASaddresses[BottomLevelASType::Count] =
        {
            bottomLevelAS[0].accelerationStructure->GetGPUVirtualAddress(),
            bottomLevelAS[1].accelerationStructure->GetGPUVirtualAddress()
        };
        BuildBotomLevelASInstanceDescs<D3D12_RAYTRACING_INSTANCE_DESC>(bottomLevelASaddresses, &instanceDescsResource);
    }

    // Create a wrapped pointer to the acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        UINT numBufferElements = static_cast<UINT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
        m_fallbackTopLevelAccelerationStructurePointer = CreateFallbackWrappedPointer(topLevelAS.Get(), numBufferElements);
    }

    // Top-level AS desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    {
        topLevelBuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelBuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelBuildDesc.Flags = buildFlags;
        topLevelBuildDesc.DestAccelerationStructureData = { topLevelAS->GetGPUVirtualAddress(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes };
        topLevelBuildDesc.NumDescs = NUM_BLAS;
        topLevelBuildDesc.pGeometryDescs = nullptr;
        topLevelBuildDesc.InstanceDescs = instanceDescsResource->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = { scratch->GetGPUVirtualAddress(), scratch->GetDesc().Width };
    }

    // Build acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        // Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
        ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
        m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
        m_fallbackCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
    }
    else // DirectX Raytracing
    {
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc);
    }

    AccelerationStructureBuffers topLevelASBuffers;
    topLevelASBuffers.accelerationStructure = topLevelAS;
    topLevelASBuffers.instanceDesc = instanceDescsResource;
    topLevelASBuffers.scratch = scratch;
    topLevelASBuffers.ResultDataMaxSizeInBytes = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
    return topLevelASBuffers;
}

// Build acceleration structure needed for raytracing.
void D3D12RaytracingProceduralGeometry::BuildAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    // Reset the command list for the acceleration structure construction.
    commandList->Reset(commandAllocator, nullptr);

    // Build bottom-level AS.
    AccelerationStructureBuffers bottomLevelAS[BottomLevelASType::Count];
    array<vector<D3D12_RAYTRACING_GEOMETRY_DESC>, BottomLevelASType::Count> geometryDescs;
    {
        BuildGeometryDescsForBottomLevelAS(geometryDescs);

        // Build all bottom-level AS.
        for (UINT i = 0; i < BottomLevelASType::Count; i++)
        {
            bottomLevelAS[i] = BuildBottomLevelAS(geometryDescs[i]);
        }
    }

    // Batch all resource barriers for bottom-level AS builds.
    D3D12_RESOURCE_BARRIER resourceBarriers[BottomLevelASType::Count];
    for (UINT i = 0; i < BottomLevelASType::Count; i++)
    {
        resourceBarriers[i] = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS[i].accelerationStructure.Get());
    }
    commandList->ResourceBarrier(BottomLevelASType::Count, resourceBarriers);

    // Build top-level AS.
    AccelerationStructureBuffers topLevelAS = BuildTopLevelAS(bottomLevelAS);
    
    // Kick off acceleration structure construction.
    m_deviceResources->ExecuteCommandList();

    // Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
    m_deviceResources->WaitForGpu();

    // Store the AS buffers. The rest of the buffers will be released once we exit the function.
    for (UINT i = 0; i < BottomLevelASType::Count; i++)
    {
        m_bottomLevelAS[i] = bottomLevelAS[i].accelerationStructure;
    }
    m_topLevelAS = topLevelAS.accelerationStructure;
}

// Build shader tables.
// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void D3D12RaytracingProceduralGeometry::BuildShaderTables()
{
    auto device = m_deviceResources->GetD3DDevice();

    void* rayGenShaderID;
    void* missShaderIDs[RayType::Count];
    void* hitGroupShaderIDs_TriangleGeometry[RayType::Count];
    void* hitGroupShaderIDs_AABBGeometry[IntersectionShaderType::Count][RayType::Count];

    // A shader name look-up table for shader table debug print out.
    unordered_map<void*, wstring> shaderIdToStringMap;

    auto GetShaderIDs = [&](auto* stateObjectProperties)
    {
        rayGenShaderID = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
        shaderIdToStringMap[rayGenShaderID] = c_raygenShaderName;

        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[i]);
            shaderIdToStringMap[missShaderIDs[i]] = c_missShaderNames[i];
        }
        for (UINT i = 0; i < RayType::Count; i++)
        {
            hitGroupShaderIDs_TriangleGeometry[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_TriangleGeometry[i]);
            shaderIdToStringMap[hitGroupShaderIDs_TriangleGeometry[i]] = c_hitGroupNames_TriangleGeometry[i];
        }
        for (UINT r = 0; r < IntersectionShaderType::Count; r++)
            for (UINT c = 0; c < RayType::Count; c++)        
            {
                hitGroupShaderIDs_AABBGeometry[r][c] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_AABBGeometry[r][c]); 
                shaderIdToStringMap[hitGroupShaderIDs_AABBGeometry[r][c]] = c_hitGroupNames_AABBGeometry[r][c];
            }
    };

    // Get shader identifiers.
    UINT shaderIDSize;
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        GetShaderIDs(m_fallbackStateObject.Get());
        shaderIDSize = m_fallbackDevice->GetShaderIdentifierSize();
    }
    else // DirectX Raytracing
    {
        ComPtr<ID3D12StateObjectPropertiesPrototype> stateObjectProperties;
        ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
        GetShaderIDs(stateObjectProperties.Get());
        shaderIDSize = m_dxrDevice->GetShaderIdentifierSize();
    }

    // Initialize shader tables.
    // ToDo apply the same to other samples.

    // RayGen shader table.
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIDSize; // No root arguments
        
        ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable" );
        rayGenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderRecordSize, nullptr, 0));
        rayGenShaderTable.DebugPrint(shaderIdToStringMap);
        m_rayGenShaderTable = rayGenShaderTable.GetResource();
    }
    
    // Miss shader table.
    {
        UINT numShaderRecords = RayType::Count;
        UINT shaderRecordSize = shaderIDSize; // No root arguments

        ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderTable.push_back(ShaderRecord(missShaderIDs[i], shaderIDSize, nullptr, 0));
        }
        missShaderTable.DebugPrint(shaderIdToStringMap);
        m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
        m_missShaderTable = missShaderTable.GetResource();
    }

    // Hit group shader table.
    {
        UINT numShaderRecords = RayType::Count + IntersectionShaderType::Count * RayType::Count;
        UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();
        ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

        // Triangle geometry hit groups.
        {
            LocalRootSignature::Triangle::RootArguments rootArgs;
            rootArgs.materialCb = m_planeMaterialCB;

            // ToDo describe layout
            for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
            {
                hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
            }
        }
      
        // AABB geometry hit groups.
        {
            LocalRootSignature::AABB::RootArguments rootArgs;
            for (UINT r = 0; r < IntersectionShaderType::Count; r++)
            {
                rootArgs.materialCb = m_aabbMaterialCB[r];
                rootArgs.aabbCB.geometryIndex = r;
                for (UINT c = 0; c < RayType::Count; c++)
                {
                    auto& hitGroupShaderID = hitGroupShaderIDs_AABBGeometry[r][c];
                    hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
                }
            }
        }
        hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
        m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
        m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
    }
}

void D3D12RaytracingProceduralGeometry::SelectRaytracingAPI(RaytracingAPI type)
{
    if (type == RaytracingAPI::FallbackLayer)
    {
        m_raytracingAPI = type;
    }
    else // DirectX Raytracing
    {
        if (m_isDxrSupported)
        {
            m_raytracingAPI = type;
        }
        else
        {
            OutputDebugString(L"Invalid selection - DXR is not available.\n");
        }
    }
}

void D3D12RaytracingProceduralGeometry::OnKeyDown(UINT8 key)
{
    // Store previous values.
    RaytracingAPI previousRaytracingAPI = m_raytracingAPI;
    bool previousForceComputeFallback = m_forceComputeFallback;

    switch (key)
    {
    case '1': // Fallback Layer
        m_forceComputeFallback = false;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case '2': // Fallback Layer + force compute path
        m_forceComputeFallback = true;
        SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        break;
    case '3': // DirectX Raytracing
        SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        break;
    default:
        break;
    }

    if (m_raytracingAPI != previousRaytracingAPI ||
        m_forceComputeFallback != previousForceComputeFallback)
    {
        // Raytracing API selection changed, recreate everything.
        RecreateD3D();
    }
}

// Update frame-based values.
void D3D12RaytracingProceduralGeometry::OnUpdate()
{
    m_timer.Tick();
    CalculateFrameStats();
    float elapsedTime = static_cast<float>(m_timer.GetElapsedSeconds());
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
    auto prevFrameIndex = m_deviceResources->GetPreviousFrameIndex();

    // Rotate the camera around Y axis.
    if (1)
    {
        float secondsToRotateAround = 48.0f;
        float angleToRotateBy = 360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        m_eye = XMVector3Transform(m_eye, rotate);
        m_up = XMVector3Transform(m_up, rotate);
        m_at = XMVector3Transform(m_at, rotate);
        UpdateCameraMatrices();
    }

    // Rotate the second light around Y axis.
    {
        float secondsToRotateAround = 8.0f;
        float angleToRotateBy = -360.0f * (elapsedTime / secondsToRotateAround);
        XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angleToRotateBy));
        const XMVECTOR& prevLightPosition = m_sceneCB->lightPosition;
        m_sceneCB->lightPosition = XMVector3Transform(prevLightPosition, rotate);
    }

    UpdateAABBPrimitiveAttributes();
}


// Parse supplied command line args.
void D3D12RaytracingProceduralGeometry::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    if (argc > 1)
    {
        if (_wcsnicmp(argv[1], L"-FL", wcslen(argv[1])) == 0)
        {
            m_forceComputeFallback = true;
            SelectRaytracingAPI(RaytracingAPI::FallbackLayer);
        }
        else if (_wcsnicmp(argv[1], L"-DXR", wcslen(argv[1])) == 0)
        {
            SelectRaytracingAPI(RaytracingAPI::DirectXRaytracing);
        }
    }
}

void D3D12RaytracingProceduralGeometry::DoRaytracing()
{
    auto commandList = m_deviceResources->GetCommandList();
    auto frameIndex = m_deviceResources->GetCurrentFrameIndex();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        dispatchDesc->HitGroupTable.StartAddress = m_hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = m_hitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
        dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = m_width;
        dispatchDesc->Height = m_height;
        commandList->DispatchRays(stateObject, dispatchDesc);
    };

    auto SetCommonPipelineState = [&](auto* descriptorSetCommandList)
    {
        descriptorSetCommandList->SetDescriptorHeaps(1, m_descriptorHeap.GetAddressOf());
        // Set index and successive vertex buffer decriptor tables
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::VertexBuffers, m_indexBuffer.gpuDescriptorHandle);
        commandList->SetComputeRootDescriptorTable(GlobalRootSignature::Slot::OutputView, m_raytracingOutputResourceUAVGpuDescriptor);
    };

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy dynamic buffers to GPU.
    {
        m_sceneCB.CopyStagingToGpu(frameIndex);
        commandList->SetComputeRootConstantBufferView(GlobalRootSignature::Slot::SceneConstant, m_sceneCB.GpuVirtualAddress(frameIndex));

        m_aabbPrimitiveAttributeBuffer.CopyStagingToGpu(frameIndex);
        // ToDo Set this in local root signature 
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AABBattributeBuffer, m_aabbPrimitiveAttributeBuffer.GpuVirtualAddress(frameIndex));
    }

    // Bind the heaps, acceleration structure and dispatch rays.    
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        D3D12_FALLBACK_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(m_fallbackCommandList.Get());
        m_fallbackCommandList->SetTopLevelAccelerationStructure(GlobalRootSignature::Slot::AccelerationStructure, m_fallbackTopLevelAccelerationStructurePointer);
        DispatchRays(m_fallbackCommandList.Get(), m_fallbackStateObject.Get(), &dispatchDesc);
    }
    else // DirectX Raytracing
    {
        D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
        SetCommonPipelineState(commandList);
        commandList->SetComputeRootShaderResourceView(GlobalRootSignature::Slot::AccelerationStructure, m_topLevelAS->GetGPUVirtualAddress());
        DispatchRays(m_dxrCommandList.Get(), m_dxrStateObject.Get(), &dispatchDesc);
    }
}

// Update the application state with the new resolution.
void D3D12RaytracingProceduralGeometry::UpdateForSizeChange(UINT width, UINT height)
{
    DXSample::UpdateForSizeChange(width, height);
}

// Copy the raytracing output to the backbuffer.
void D3D12RaytracingProceduralGeometry::CopyRaytracingOutputToBackbuffer()
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
void D3D12RaytracingProceduralGeometry::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource();
    UpdateCameraMatrices();
}

// Release resources that are dependent on the size of the main window.
void D3D12RaytracingProceduralGeometry::ReleaseWindowSizeDependentResources()
{
    m_raytracingOutput.Reset();
}

// Release all resources that depend on the device.
void D3D12RaytracingProceduralGeometry::ReleaseDeviceDependentResources()
{
    m_fallbackDevice.Reset();
    m_fallbackCommandList.Reset();
    m_fallbackStateObject.Reset();
    m_raytracingGlobalRootSignature.Reset();
    ResetComPtrArray(&m_raytracingLocalRootSignature);

    m_dxrDevice.Reset();
    m_dxrCommandList.Reset();
    m_dxrStateObject.Reset();

    m_descriptorHeap.Reset();
    m_descriptorsAllocated = 0;
    m_raytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;
    m_indexBuffer.resource.Reset();
    m_vertexBuffer.resource.Reset();
    m_rayGenShaderTable.Reset();
    m_missShaderTable.Reset();
    m_hitGroupShaderTable.Reset();

    ResetComPtrArray(&m_bottomLevelAS);
    m_topLevelAS.Reset();
}

void D3D12RaytracingProceduralGeometry::RecreateD3D()
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
void D3D12RaytracingProceduralGeometry::OnRender()
{
    if (!m_deviceResources->IsWindowVisible())
    {
        return;
    }

    m_deviceResources->Prepare();

    DoRaytracing();

    CopyRaytracingOutputToBackbuffer();

    m_deviceResources->Present(D3D12_RESOURCE_STATE_PRESENT);
}

void D3D12RaytracingProceduralGeometry::OnDestroy()
{
    OnDeviceLost();
}

// Release all device dependent resouces when a device is lost.
void D3D12RaytracingProceduralGeometry::OnDeviceLost()
{
    ReleaseWindowSizeDependentResources();
    ReleaseDeviceDependentResources();
}

// Create all device dependent resources when a device is restored.
void D3D12RaytracingProceduralGeometry::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

// Compute the average frames per second and million rays per second.
void D3D12RaytracingProceduralGeometry::CalculateFrameStats()
{
    static int frameCnt = 0;
    static double elapsedTime = 0.0f;
    double totalTime = m_timer.GetTotalSeconds();
    frameCnt++;

    // Compute averages over one second period.
    if ((totalTime - elapsedTime) >= 1.0f)
    {
        float diff = static_cast<float>(totalTime - elapsedTime);
        float fps = static_cast<float>(frameCnt) / diff; // Normalize to an exact second.

        frameCnt = 0;
        elapsedTime = totalTime;

        float MRaysPerSecond = (m_width * m_height * fps) / static_cast<float>(1e6);

        wstringstream windowText;

        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            if (m_fallbackDevice->UsingRaytracingDriver())
            {
                windowText << L"(FL-DXR)";
            }
            else
            {
                windowText << L"(FL)";
            }
        }
        else
        {
            windowText << L"(DXR)";
        }
        windowText << setprecision(2) << fixed
            << L"    fps: " << fps << L"     ~Million Primary Rays/s: " << MRaysPerSecond;
        SetCustomWindowText(windowText.str().c_str());
    }
}

// Handle OnSizeChanged message event.
void D3D12RaytracingProceduralGeometry::OnSizeChanged(UINT width, UINT height, bool minimized)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, minimized))
    {
        return;
    }

    UpdateForSizeChange(width, height);

    ReleaseWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

// Create a wrapped pointer for the Fallback Layer path.
WRAPPED_GPU_POINTER D3D12RaytracingProceduralGeometry::CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
    rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

    D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;

    // Only compute fallback requires a valid descriptor index when creating a wrapped pointer.
    UINT descriptorHeapIndex = 0;
    if (!m_fallbackDevice->UsingRaytracingDriver())
    {
        descriptorHeapIndex = AllocateDescriptor(&bottomLevelDescriptor);
        device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
    }
    return m_fallbackDevice->GetWrappedPointerSimple(descriptorHeapIndex, resource->GetGPUVirtualAddress());
}

// Allocate a descriptor and return its index. 
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT D3D12RaytracingProceduralGeometry::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
    {
        ThrowIfFalse(m_descriptorsAllocated < m_descriptorHeap->GetDesc().NumDescriptors, L"Ran out of descriptors on the heap!" );
        descriptorIndexToUse = m_descriptorsAllocated++;
    }
    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
    return descriptorIndexToUse;
}

// Create SRV for a buffer.
UINT D3D12RaytracingProceduralGeometry::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
{
    auto device = m_deviceResources->GetD3DDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements = numElements;
    srvDesc.Buffer.StructureByteStride = elementSize;
    UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    return descriptorIndex;
};
