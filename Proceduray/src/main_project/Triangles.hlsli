#ifndef TRIANGLES_H
#define TRIANGLES_H

[shader("closesthit")]
void ClosestHit_Triangle(inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr)
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
    float3 triangleNormal = /*normalize(grad(HitWorldPosition()));*/-g_vertices[indices[0]].normal;


    // PERFORMANCE TIP: it is recommended to avoid values carry over across TraceRay() calls. 
    // Therefore, in cases like retrieving HitWorldPosition(), it is recomputed every time.

    // Shadow component.
    // Trace a shadow ray.
    float3 hitPosition = HitWorldPosition();
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);

    //float checkers = AnalyticalCheckersTexture(HitWorldPosition(), triangleNormal, g_sceneCB.cameraPosition.xyz, g_sceneCB.projectionToWorld);
    //float checkers = 0;//indices[0]%2;
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
    //float4 color = (checkers + abs(checkers-1.)* 0.8) * (phongColor + reflectedColor);

   // float4 phongColor = float4(l_materialCB.albedo*dot(triangleNormal,WorldRayDirection()));
    float4 color = phongColor + reflectedColor;
    
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

#endif