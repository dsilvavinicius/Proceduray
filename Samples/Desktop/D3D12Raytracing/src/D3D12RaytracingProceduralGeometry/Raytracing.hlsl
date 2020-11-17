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
#include "JuliaSets.hlsli"

#include "Graph3D.hlsli"

static float gStep = 0.5;
static float gMaxLenght = 200.;

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
StructuredBuffer<PrimitiveInstancePerFrameBuffer> g_AABBPrimitiveAttributes : register(t3, space0);
ConstantBuffer<PrimitiveConstantBuffer> l_materialCB : register(b1);
ConstantBuffer<PrimitiveInstanceConstantBuffer> l_aabbCB: register(b2);


//***************************************************************************
//****************------ Utility functions -------***************************
//***************************************************************************

// Diffuse lighting calculation.
float CalculateDiffuseCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal)
{
    float fNDotL = saturate(dot(-incidentLightRay, normal));
    return fNDotL;
}

// Phong lighting specular component
float4 CalculateSpecularCoefficient(in float3 hitPosition, in float3 incidentLightRay, in float3 normal, in float specularPower)
{
    float3 reflectedLightRay = normalize(reflect(incidentLightRay, normal));
    return pow(saturate(dot(reflectedLightRay, normalize (-WorldRayDirection()))), specularPower);
}


// Phong lighting model = ambient + diffuse + specular components.
float4 CalculatePhongLighting(in float4 albedo, in float3 normal, in bool isInShadow, in float diffuseCoef = 1.0, in float specularCoef = 1.0, in float specularPower = 50)
{
    float3 hitPosition = HitWorldPosition();
    float3 lightPosition = g_sceneCB.lightPosition.xyz;
    float shadowFactor = isInShadow ? InShadowRadiance : 1.0;
    float3 incidentLightRay = normalize(hitPosition - lightPosition);

    // Diffuse component.
    float4 lightDiffuseColor = g_sceneCB.lightDiffuseColor;
    float Kd = CalculateDiffuseCoefficient(hitPosition, incidentLightRay, normal);
    float4 diffuseColor = shadowFactor * diffuseCoef * Kd * lightDiffuseColor * albedo;

    // Specular component.
    float4 specularColor = float4(0, 0, 0, 0);
    if (!isInShadow)
    {
        float4 lightSpecularColor = float4(1, 1, 1, 1);
        float4 Ks = CalculateSpecularCoefficient(hitPosition, incidentLightRay, normal, specularPower);
        specularColor = specularCoef * Ks * lightSpecularColor;
    }

    // Ambient component.
    // Fake AO: Darken faces with normal facing downwards/away from the sky a little bit.
    float4 ambientColor = g_sceneCB.lightAmbientColor;
    float4 ambientColorMin = g_sceneCB.lightAmbientColor - 0.1;
    float4 ambientColorMax = g_sceneCB.lightAmbientColor;
    float a = 1 - saturate(dot(normal, float3(0, -1, 0)));
    ambientColor = albedo * lerp(ambientColorMin, ambientColorMax, a);
    
    return ambientColor + diffuseColor + specularColor;
}


//***************************************************************************
//*****------ TraceRay wrappers for radiance and shadow rays. -------********
//***************************************************************************

// Trace a radiance ray into the scene and returns a shaded color.
float4 TraceRadianceRay(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return float4(0, 0, 0, 0);
    }
#ifdef RAYTRACING
    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;
    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1, 0.f, 0, false};
    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Radiance],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Radiance],
        rayDesc, rayPayload);

    return rayPayload.color;
    
#endif
    
#ifdef NONLINEAR_RAYTRACING 
    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifacts along contact areas.
    // Note: make sure to enable face culling so as to avoid surface face fighting.
    
    rayDesc.TMin = 0.;
    rayDesc.TMax = gStep;
   
    RayPayload rayPayload = { float4(0, 0, 0, 0), currentRayRecursionDepth + 1, 0.f, 0, false};
    
    while (rayPayload.dist < gMaxLenght && !rayPayload.hit)
    {
        rayPayload.hit = true;
        TraceRay(g_scene,
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
            TraceRayParameters::InstanceMask,
            TraceRayParameters::HitGroup::Offset[RayType::Radiance],
            TraceRayParameters::HitGroup::GeometryStride,
            TraceRayParameters::MissShader::Offset[RayType::Radiance],
            rayDesc, rayPayload);

        //update the ray
        evalGraphRay(rayDesc.Origin, rayDesc.Direction, gStep - 0.08); //Riemannian metric induced by a graph of a function
        //rayDesc.Origin += (gStep-0.000001)*rayDesc.Direction;
    }
    return rayPayload.color;
#endif    
}

// Trace a shadow ray and return true if it hits any geometry.
bool TraceShadowRayAndReportIfHit(in Ray ray, in UINT currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH)
    {
        return false;
    }

 #ifdef RAYTRACING
// Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifcats along contact areas.
    // Note: make sure to enable back-face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    // Initialize shadow ray payload.
    // Set the initial value to true since closest and any hit shaders are skipped. 
    // Shadow miss shader, if called, will set it to false.
    ShadowRayPayload shadowPayload = { 0.f,true };
    TraceRay(g_scene,
        RAY_FLAG_CULL_BACK_FACING_TRIANGLES
        | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders,
        TraceRayParameters::InstanceMask,
        TraceRayParameters::HitGroup::Offset[RayType::Shadow],
        TraceRayParameters::HitGroup::GeometryStride,
        TraceRayParameters::MissShader::Offset[RayType::Shadow],
        rayDesc, shadowPayload);

    return shadowPayload.hit;
#endif
    
#ifdef NONLINEAR_RAYTRACING   
    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = ray.origin;
    rayDesc.Direction = ray.direction;
    // Set TMin to a zero value to avoid aliasing artifcats along contact areas.
    // Note: make sure to enable back-face culling so as to avoid surface face fighting.
    rayDesc.TMin = 0;
    rayDesc.TMax = gStep; //why is it not working for rayDesc.TMax = step
    
    // Initialize shadow ray payload.
    // Set the initial value to true since closest and any hit shaders are skipped. 
    // Shadow miss shader, if called, will set it to false.
    
    ShadowRayPayload shadowPayload = { 0.f, false };
    
    while(shadowPayload.dist < gMaxLenght && !shadowPayload.hit)
    { 
        shadowPayload.hit = true;
        TraceRay(g_scene,
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES
            | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
            | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
            | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, // ~skip closest hit shaders,
            TraceRayParameters::InstanceMask,
            TraceRayParameters::HitGroup::Offset[RayType::Shadow],
            TraceRayParameters::HitGroup::GeometryStride,
            TraceRayParameters::MissShader::Offset[RayType::Shadow],
            rayDesc, shadowPayload);
         
            //update the ray
         evalGraphRay(rayDesc.Origin, rayDesc.Direction, gStep); //Riemannian metric induced by a graph of a function
         //rayDesc.Origin += gStep*rayDesc.Direction;
    }
    return shadowPayload.hit;
#endif    
}

//***************************************************************************
//********************------ Ray gen shader.. -------************************
//***************************************************************************

[shader("raygeneration")]
void MyRaygenShader()
{
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    Ray ray = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
 
    // Cast a ray into the scene and retrieve a shaded color.
    UINT currentRecursionDepth = 0;
    float4 color = TraceRadianceRay(ray, currentRecursionDepth);

    // Write the raytraced color to the output texture.
    g_renderTarget[DispatchRaysIndex().xy] = color;
}

//[shader("raygeneration")]
//void MyRaygenShader()
//{
//    float4 col = {0.0, 0.0, 0.0, 1.0};
//    f_mainImage_float4(col, DispatchRaysIndex().xy, DispatchRaysDimensions().xy, g_sceneCB.elapsedTime);

//    // Write the raytraced color to the output texture.
//    g_renderTarget[DispatchRaysIndex().xy] = col;
//}

//***************************************************************************
//******************------ Closest hit shaders -------***********************
//***************************************************************************

[shader("closesthit")]
void MyClosestHitShader_Triangle(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
{
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        rayPayload.color = float4(0.f, 0.f, 1.0f, 0.f);
        return;
    }

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;
    
    // Load up three 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex, g_indices);
    
    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 triangleNormal = /*normalize(grad(HitWorldPosition()));*/g_vertices[indices[0]].normal;

    // PERFORMANCE TIP: it is recommended to avoid values carry over across TraceRay() calls. 
    // Therefore, in cases like retrieving HitWorldPosition(), it is recomputed every time.

    // Shadow component.
    // Trace a shadow ray.
    float3 hitPosition = HitWorldPosition();
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    //float checkers = AnalyticalCheckersTexture(HitWorldPosition(), triangleNormal, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
    float checkers = 0;//indices[0]%2;
    // Reflected component.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001 )
    {
        // Trace a reflection ray.
        Ray reflectionRay = { HitWorldPosition(), reflect(WorldRayDirection(), triangleNormal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), triangleNormal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, triangleNormal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = (checkers + abs(checkers-1.)* 0.8) * (phongColor + reflectedColor);
    
    #ifdef SECTIONALCURVATURE
    //computing sectional curvature
    float3 p1 = g_vertices[indices[0]].position;
    float3 p2 = g_vertices[indices[1]].position;
    
    float3 u = normalize(p1 - p2);
    float3 v = cross(triangleNormal, u);
    
    float curvature = secCurv(hitPosition, u, v);
    
    if(curvature < 0 )
    {
        color.x += -curvature*10000.;
    }
    else
    {
        color.z += curvature*10000.;
    }
    #endif
   
    color += rayPayload.color;
    
    // Apply visibility falloff.
    rayPayload.dist+=RayTCurrent();
    float t = rayPayload.dist;
        
    //color = lerp(color, BackgroundColor + rayPayload.color, 1.0 - exp(-0.0000002*t*t*t));
   
    rayPayload.color = color;
}

[shader("closesthit")]
void MyClosestHitShader_AABB(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    // DEBUG
    if (HitKind() == 1)
    {
        rayPayload.color = float4(0.f, 0.f, 1.f, 0.f);
        return;
    }

    // PERFORMANCE TIP: it is recommended to minimize values carry over across TraceRay() calls. 
    // Therefore, in cases like retrieving HitWorldPosition(), it is recomputed every time.

    // Shadow component.
    // Trace a shadow ray.
    float3 hitPosition = HitWorldPosition();
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    // Reflected component.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001)
    {
        // Trace a reflection ray.
        Ray reflectionRay = { HitWorldPosition(), reflect(WorldRayDirection(), attr.normal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(l_materialCB.albedo, attr.normal, shadowRayHit, l_materialCB.diffuseCoef, l_materialCB.specularCoef, l_materialCB.specularPower);
    float4 color = phongColor + reflectedColor;

    color += rayPayload.color;
    
    // Apply visibility falloff.
    rayPayload.dist+=RayTCurrent();
    float t = rayPayload.dist;
        
    //color = lerp(color, BackgroundColor + rayPayload.color, 1.0 - exp(-0.0000002*t*t*t));
   
    rayPayload.color = color;
}

[shader("closesthit")]
void MandelbulbClosestHit(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    if (HitKind() == 1)
    {
        rayPayload.color = float4(0.f, 1.f, 0.f, 0.f);
        return;
    }

    // color
    float3 col = float3(0.01, 0.01, 0.01);
	col = lerp( col, float3(0.10,0.20,0.30), clamp(attr.color.y,0.0,1.0) );
	col = lerp( col, float3(0.02,0.10,0.30), clamp(attr.color.z*attr.color.z,0.0,1.0) );
    col = lerp( col, float3(0.30,0.10,0.02), clamp(pow(attr.color.w,6.0),0.0,1.0) );
    col *= 0.5;
	//col = float3(0.1);
        
    // lighting terms
    float3 light1 = float3(0.577, 0.577, -0.577);
    float3 light2 = float3(-0.707, 0.000,  0.707);
            
    float3 pos = HitWorldPosition();
    float3 nor = attr.normal;
    float3 hal = normalize( light1-WorldRayDirection());
    float3 ref = reflect( WorldRayDirection(), nor );
    float occ = clamp(0.05*log(attr.color.x),0.0,1.0);
    float fac = clamp(1.0+dot(WorldRayDirection(),nor),0.0,1.0);
            
    // sun
    float sha1 = 1.f;//softshadow( pos+0.001*nor, light1, 32.0 );
    float dif1 = clamp( dot( light1, nor ), 0.0, 1.0 )*sha1;
    float spe1 = pow( clamp(dot(nor,hal),0.0,1.0), 32.0 )*dif1*(0.04+0.96*pow(clamp(1.0-dot(hal,light1),0.0,1.0),5.0));
    // bounce
    float dif2 = clamp( 0.5 + 0.5*dot( light2, nor ), 0.0, 1.0 )*occ;
    // sky
    float dif3 = (0.7+0.3*nor.y)*(0.2+0.8*occ);
        
	float3 lin = float3(0.0, 0.0, 0.0); 
		    lin += 7.0*float3(1.50,1.10,0.70)*dif1;
		    lin += 4.0*float3(0.25,0.20,0.15)*dif2;
        	lin += 1.5*float3(0.10,0.20,0.30)*dif3;
            lin += 2.5*float3(0.35,0.30,0.25)*(0.05+0.95*occ); // ambient
        	lin += 4.0*fac*occ;                          // fake SSS
	col *= lin;
	col = pow( col, float3(0.7,0.9,1.0) );                  // fake SSS
    col += spe1*15.0;
    //col += 8.0*float3(0.8,0.9,1.0)*(0.2+0.8*occ)*(0.03+0.97*pow(fac,5.0))*smoothstep(0.0,0.1,ref.y )*softshadow( pos+0.01*nor, ref, 2.0 );
    //col = float3(occ*occ);
        
    // gamma
	col = sqrt( col );
            
    float4 color = float4(col, 1.f);
    
    // Reflected component.
    float4 reflectedColor = float4(0, 0, 0, 0);
    if (l_materialCB.reflectanceCoef > 0.001)
    {
        // Trace a reflection ray.
        Ray reflectionRay = {pos, ref};
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);

        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, l_materialCB.albedo.xyz);
        reflectedColor = l_materialCB.reflectanceCoef * float4(fresnelR, 1) * reflectionColor;
    }

    // Calculate final color.
    color = color + reflectedColor;
    color += rayPayload.color;
    
    // Apply visibility falloff.
    rayPayload.dist+=RayTCurrent();
    //float t = rayPayload.dist;
        
    //color = lerp(color, BackgroundColor + rayPayload.color, 1.0 - exp(-0.0000002*t*t*t));
   
    rayPayload.color = color;
}

[shader("closesthit")]
void JuliaClosestHit(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    // DEBUG
    if (HitKind() == 1)
    {
        rayPayload.color = float4(1.f, 0.f, 0.f, 0.f);
        return;
    }

    float3 pos = HitWorldPosition();

    //rayPayload.color = float4(colorSurface(pos, attr.normal, attr.color.xy), 1.f);
    //rd = cosineDirection(attr.normal);
    //ro = pos + attr.normal * kPrecis;
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
void MyMissShader(inout RayPayload rayPayload)
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
        
        //scalar curvature
        //{
        //    float curvature = scalarCurv(p);
        //    if (curvature < 0)
        //    {
        //        rayPayload.color.x += -curvature * 10.;
        //    }
        //    else
        //    {
        //        rayPayload.color.z += curvature * 10.;
        //    }       
        //}
        
        //coefficient curvature tensor
        {
            float curv1 = coefCurvTensor(p, 1, 2, 1, 2);
            float curv2 = coefCurvTensor(p, 1, 2, 1, 3);
            float curv3 = coefCurvTensor(p, 1, 2, 2, 3);
            float curv4 = coefCurvTensor(p, 1, 3, 1, 3);
            float curv5 = coefCurvTensor(p, 1, 3, 2, 3);
            float curv6 = coefCurvTensor(p, 2, 3, 2, 3);
            
            float t = 1.f;
            
            rayPayload.color.x += curv1*t;//red
            rayPayload.color.y += curv2*t;//green
            rayPayload.color.z += curv3*t;//blue
                                        
            rayPayload.color.y += curv4*t;//cyan
            rayPayload.color.z += curv4*t;//
                                        
            rayPayload.color.x += curv5*t;//magenta
            rayPayload.color.z += curv5*t;//
                                        
            rayPayload.color.x += curv6*t;//yellow
            rayPayload.color.y += curv6*t;//
            
            
            
            
        }
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
void MyMissShader_ShadowRay(inout ShadowRayPayload rayPayload)
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
    PrimitiveInstancePerFrameBuffer attr = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];

    // Retrieve a ray origin position and direction in bottom level AS space 
    // and transform them into the AABB primitive's local space.
    Ray ray;
    ray.origin = mul(float4(WorldRayOrigin(), 1), attr.localSpaceToBottomLevelAS).xyz;
    ray.direction = mul(WorldRayDirection(), (float3x3) attr.localSpaceToBottomLevelAS);
    ray.direction = normalize(ray.direction);
    return ray;
}

//[shader("intersection")]
void MyIntersectionShader_AnalyticPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    AnalyticPrimitive::Enum primitiveType = (AnalyticPrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayAnalyticGeometryIntersectionTest(localRay, primitiveType, thit, attr))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

//[shader("intersection")]
void MyIntersectionShader_VolumetricPrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    VolumetricPrimitive::Enum primitiveType = (VolumetricPrimitive::Enum) l_aabbCB.primitiveType;
    
    float thit;
    ProceduralPrimitiveAttributes attr;
    if (RayVolumetricGeometryIntersectionTest(localRay, primitiveType, thit, attr, g_sceneCB.elapsedTime))
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = mul(attr.normal, (float3x3) aabbAttribute.localSpaceToBottomLevelAS);
        attr.normal = normalize(mul((float3x3) ObjectToWorld3x4(), attr.normal));

        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MyIntersectionShader_SignedDistancePrimitive()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    SignedDistancePrimitive::Enum primitiveType = (SignedDistancePrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr;
    
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        ReportHit(0, 1, attr);
        return;
    }
    
    bool primitiveTest = RaySignedDistancePrimitiveTest(localRay, primitiveType, thit, attr, l_materialCB.stepScale);
    
    if (primitiveTest)
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        //attr.normal = normalize(mul(attr.normal, (float3x3) aabbAttribute.bottomLevelASToLocalSpace));
        attr.normal = normalize(mul(attr.normal, (float3x3) WorldToObject3x4()));
        
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void MandelbulbIntersection()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();

    float thit;
    ProceduralPrimitiveAttributes attr;
    
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        ReportHit(0, 1, attr);
        return;
    }
    
    bool primitiveTest = MandelbulbDistance(localRay, g_sceneCB.elapsedTime, l_aabbCB.instanceIndex, thit, attr, l_materialCB.stepScale);
    
    if (primitiveTest)
    {
        PrimitiveInstancePerFrameBuffer aabbAttribute = g_AABBPrimitiveAttributes[l_aabbCB.instanceIndex];
        attr.normal = normalize(mul(attr.normal, (float3x3) WorldToObject3x4()));
        
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("intersection")]
void JuliaIntersection()
{
}

#endif // RAYTRACING_HLSL