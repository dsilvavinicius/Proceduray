#ifndef PACMAN_H
#define PACMAN_H

[shader("intersection")]
void Intersection_Pacman()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();
    SignedDistancePrimitive::Enum primitiveType = (SignedDistancePrimitive::Enum) l_aabbCB.primitiveType;

    float thit;
    ProceduralPrimitiveAttributes attr = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f} };
    
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        ReportHit(0, 1, attr);
        return;
    }
    
    bool primitiveTest = RaySignedDistancePrimitiveTest(localRay, primitiveType, thit, attr, l_materialCB.stepScale, g_sceneCB.elapsedTime);
    
    if (primitiveTest && thit < RayTCurrent())
    {
        InstanceBuffer aabbAttribute = g_instanceBuffer[l_aabbCB.instanceIndex];
        //attr.normal = normalize(mul(attr.normal, (float3x3) aabbAttribute.bottomLevelASToLocalSpace));
        attr.normal = normalize(mul(attr.normal, (float3x3) WorldToObject3x4()));
        
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void ClosestHit_Pacman(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
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
    float4 color = attr.color * (phongColor + reflectedColor);

    color += rayPayload.color;
    
    // Apply visibility falloff.
    rayPayload.dist+=RayTCurrent();
    float t = rayPayload.dist;
        
    //color = lerp(color, BackgroundColor + rayPayload.color, 1.0 - exp(-0.0000002*t*t*t));
   
    rayPayload.color = color;
}

#endif