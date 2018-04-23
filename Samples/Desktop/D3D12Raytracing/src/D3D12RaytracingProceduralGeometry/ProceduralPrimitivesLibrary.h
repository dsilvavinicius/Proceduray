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

#ifndef PROCEDURALPRIMITIVESLIBRARY_H
#define PROCEDURALPRIMITIVESLIBRARY_H

#include "RaytracingShaderHelper.h"

#include "AnalyticPrimitives.h"
#include "VolumetricPrimitives.h"
#include "SignedDistancePrimitives.h"
#include "SignedDistanceFractals.h"

// Analytic geometry intersection test.
bool RayAnalyticGeometryIntersectionTest(in Ray ray, in AnalyticPrimitive::Enum analyticPrimitive, out float thit, out ProceduralPrimitiveAttributes attr)
{
    switch (analyticPrimitive)
    {
    case AnalyticPrimitive::AABB: return RayAABBIntersectionTest(ray, thit, attr);
    case AnalyticPrimitive::Sphere: return RaySphereIntersectionTest(ray, thit, attr);
    case AnalyticPrimitive::Spheres: return RaySpheresIntersectionTest(ray, thit, attr);
    default: return false;
    }
}

// Analytic geometry intersection test.
bool RayVolumetricGeometryIntersectionTest(in Ray ray, in VolumetricPrimitive::Enum volumetricPrimitive, out float thit, out ProceduralPrimitiveAttributes attr, in float elapsedTime)
{
    switch (volumetricPrimitive)
    {
    case VolumetricPrimitive::Metaballs: return RayMetaballsIntersectionTest(ray, thit, attr, elapsedTime);
    default: return false;
    }
}

// Signed distance functions use a shared ray signed distance test.
// The test, instead, calls into this function to retrieve a distance for a primitive.
// Ref: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float GetDistanceFromSignedDistancePrimitive(in float3 position, in SignedDistancePrimitive::Enum signedDistancePrimitive)
{
    switch (signedDistancePrimitive)
    {
    case SignedDistancePrimitive::Cone: return sdCone(position + float3(0, -0.7, 0), float3(0.8, 0.6, 1.2));
    case SignedDistancePrimitive::MiniSpheres:
        return opI(sdSphere(opRep(position + 1, (float3)2 / 4), 0.65 / 4),
                   sdBox(position, (float3)1));
    case SignedDistancePrimitive::IntersectedRoundCube:
        return opS(udRoundBox(position, (float3)0.75, 0.2),
                   sdSphere(position, 1.20));
    case SignedDistancePrimitive::Torus: return sdTorus(position, float2(0.7, 0.25));
    case SignedDistancePrimitive::TwistedTorus: return sdTorus(opTwist(position), float2(0.6, 0.2));
        
    case SignedDistancePrimitive::Pyramid: 
        // Pyramid: 63.435 degree, 2 height
        return sdPyramid(position, float3(0.894, 0.447, 1.0));
    case SignedDistancePrimitive::Cog:
        return opS( sdTorus82(position, float2(0.60, 0.3)),
                    sdCylinder(opRep(float3(atan2(position.z, position.x) / 6.2831, 
                                            1, 
                                            0.015 + 0.25 * length(position)) + 1,
                                     float3(0.05, 1, 0.075)),
                               float2(0.02, 0.8)));
    case SignedDistancePrimitive::Cylinder: return sdCylinder(position, float2(0.4, 1.0));
    case SignedDistancePrimitive::SquareTorus: return sdTorus88(position, float2(0.75, 0.15));
        //return sdFractalPyramid(position, 2);
    default: return 0;
    }
}

#endif // PROCEDURALPRIMITIVESLIBRARY_H