#ifndef MANDELBULB_H
#define MANDELBULB_H

[shader("intersection")]
void Intersection_Mandelbulb()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();

    float thit;
    ProceduralPrimitiveAttributes attr = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f} };
    
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        ReportHit(0, 1, attr);
        return;
    }
    
    bool primitiveTest = MandelbulbDistance(localRay, g_sceneCB.elapsedTime, l_aabbCB.instanceIndex, thit, attr, l_materialCB.stepScale);
    
    if (primitiveTest && thit < RayTCurrent())
    {
        InstanceBuffer aabbAttribute = g_instanceBuffer[l_aabbCB.instanceIndex];
        attr.normal = normalize(mul(attr.normal, (float3x3) WorldToObject3x4()));
        
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void ClosestHit_Mandelbulb(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
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

#endif