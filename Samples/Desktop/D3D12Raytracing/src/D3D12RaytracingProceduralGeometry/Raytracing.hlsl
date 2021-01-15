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
//#define NONLINEAR_RAYTRACING
#define RAYTRACING

//#define SECTIONALCURVATURE

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "engine/ShaderCompat.h"
#include "ProceduralPrimitivesLibrary.hlsli"
#include "RaytracingShaderHelper.hlsli"

#include "Graph3D.hlsli"

static float gStep = 4.;
static float gMaxLenght = 600.;

//***************************************************************************
//*****------ Shader resources bound via root signatures -------*************
//***************************************************************************

// Scene wide resources.
//  g_* - bound via a global root signature.
//  l_* - bound via a local root signature.
RaytracingAccelerationStructure g_scene : register(t0, space0);
RWTexture2D<float4> g_renderTarget : register(u0);
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

// Triangle resources
ByteAddressBuffer g_indices : register(t1, space0);
StructuredBuffer<Vertex> g_vertices : register(t2, space0);

// Procedural geometry resources
StructuredBuffer<InstanceBuffer> g_instanceBuffer : register(t3, space0);
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1);
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2);

#include "Helpers.hlsli"

//***************************************************************************
//********************------ Ray gen shader.. -------************************
//***************************************************************************

[shader("raygeneration")]
void Raygen()
{
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    Ray ray = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
 
    // Cast a ray into the scene and retrieve a shaded color.
    UINT currentRecursionDepth = 0;
    float4 color = TraceRadianceRay(ray, currentRecursionDepth);

    // Write the raytraced color to the output texture.
    g_renderTarget[DispatchRaysIndex().xy] = color;
}

void traceRaySegment(inout RayPayload payload)
{
    RayDesc rayDesc;
    rayDesc.Direction = WorldRayDirection();
    rayDesc.Origin = WorldRayOrigin() + (gStep - 0.000001) * rayDesc.Direction;
    rayDesc.TMin = 0.f;
    rayDesc.TMax = gStep;

   //update the ray
   //evalGraphRay(rayDesc.Origin, rayDesc.Direction, gStep-0.08); //Riemannian metric induced by a graph of a function
    
    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Radiance],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Radiance],
        rayDesc, payload);
}

//***************************************************************************
//**********************------ Miss shaders -------**************************
//***************************************************************************

[shader("miss")]
void Miss(inout RayPayload rayPayload)
{
#ifdef RAYTRACING    
    float4 backgroundColor = float4(BackgroundColor);
    rayPayload.color = backgroundColor;
    rayPayload.hit = false;
#endif    
    
#ifdef NONLINEAR_RAYTRACING  
    rayPayload.dist += (gStep - 0.000001);
    rayPayload.count += 1;
   
    if (rayPayload.dist < gMaxLenght)
    {
        //computing coefficients of the curvature tensor 
        float3 p = WorldRayOrigin() + (gStep - 0.000001) * WorldRayDirection();
    
        //float curvature = coefCurvTensor(p, 1, 2, 1, 2);
        
        //transfer function
        float3 Csrc = float3(0.f,0.f,0.f);
        
        //scalar curvature
        {
            float curvature = scalarCurv(p);
            
            if (curvature < 0)
            {
                Csrc.x += -curvature * 10.;
            }
            else
            {
                Csrc.z += curvature * 10.;
            }
        }
        
        //coefficient curvature tensor
        //{
        //    float curv1 = coefCurvTensor(p, 1, 2, 1, 2);
        //    float curv2 = coefCurvTensor(p, 1, 2, 1, 3);
        //    float curv3 = coefCurvTensor(p, 1, 2, 2, 3);
        //    float curv4 = coefCurvTensor(p, 1, 3, 1, 3);
        //    float curv5 = coefCurvTensor(p, 1, 3, 2, 3);
        //    float curv6 = coefCurvTensor(p, 2, 3, 2, 3);
            
        //    float t = 1.f;
            
        //    rayPayload.color.x += curv1*t;//red
        //    rayPayload.color.y += curv2*t;//green
        //    rayPayload.color.z += curv3*t;//blue
                                        
        //    rayPayload.color.y += curv4*t;//cyan
        //    rayPayload.color.z += curv4*t;//
                                        
        //    rayPayload.color.x += curv5*t;//magenta
        //    rayPayload.color.z += curv5*t;//
                                        
        //    rayPayload.color.x += curv6*t;//yellow
        //    rayPayload.color.y += curv6*t;//            
        //}
        
        rayPayload.color += float4(Csrc,1);
        
    }
    else
    {
        float4 backgroundColor = float4(BackgroundColor);
            
        //float t = rayPayload.dist;
        //rayPayload.color = lerp(rayPayload.color, BackgroundColor, 1.0 - exp(-0.0000002*t*t*t));
        rayPayload.color += backgroundColor;
    }
    rayPayload.hit = false;
#endif 
}

[shader("miss")]
void Miss_Shadow(inout ShadowRayPayload rayPayload)
{
   rayPayload.dist += gStep;
   rayPayload.hit = false;
}

//***************************************************************************
//*****************------ Intersection shaders-------************************
//***************************************************************************

// Get ray in AABB's local space.
Ray GetRayInAABBPrimitiveLocalSpace()
{
    InstanceBuffer attr = g_instanceBuffer[l_aabbCB.instanceIndex];

    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(WorldRayOrigin(), 1), attr.localSpaceToBottomLevelAS).xyz;
    ray.direction = mul(WorldRayDirection(), (float3x3) attr.localSpaceToBottomLevelAS);
    ray.direction = normalize(ray.direction);
    return ray;
}

#include "Pacman.hlsli"

#include "JuliaSets.hlsli"

#include "Mandelbulb.hlsli"

#include "Triangles.hlsli"

#endif // RAYTRACING_HLSL