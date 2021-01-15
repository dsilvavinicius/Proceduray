#ifndef JULIASETS_H
#define JULIASETS_H

#define kPrecis 0.00025f
#define kc float4(-2.f, 6.f, 15.f, -6.f) / 22.0f

float3 float3_ctor(float x0)
{
    return float3(x0, x0, x0);
}

float4 qSquare_float4(in float4 q)
{
    return float4(((((q.x * q.x) - (q.y * q.y)) - (q.z * q.z)) - (q.w * q.w)), ((2.0 * q.x) * q.yzw));
}
float4 qCube_float4(in float4 q)
{
    float4 q2 = (q * q);
    return float4((q.x * (((q2.x - (3.0 * q2.y)) - (3.0 * q2.z)) - (3.0 * q2.w))), (q.yzw * ((((3.0 * q2.x) - q2.y) - q2.z) - q2.w)));
}
float qLength2_float4(in float4 q)
{
    return dot(q, q);
}
float2 iSphere(in float3 ro, in float3 rd, in float rad)
{
    float b = dot(ro, rd);
    float c = dot(ro, ro) - (rad * rad);
    float h = (b * b) - c;
    if (h < 0.0)
    {
        return float2(-1.0, -1.0);
    }
    h = sqrt(h);
    return float2(-b - h, -b + h);
}
float3x3 setCamera(in float3 ro, in float3 ta, in float cr)
{
    float3 cw = normalize((ta - ro));
    float3 cp = float3(sin(cr), cos(cr), 0.0);
    float3 cu = normalize(cross(cw, cp));
    float3 cv = normalize(cross(cu, cw));
    float3 s1 = cu;
    float3 s2 = cv;
    float3 s3 = cw;
    return float3x3(s1[0], s1[1], s1[2], s2[0], s2[1], s2[2], s3[0], s3[1], s3[2]);
}
float2 jdist(in float3 p, in float4 c, in int iMax)
{
    float4 z = float4(p, 0.0);
    float dz2 = 1.0;
    float m2 = 0.0;
    float n = 0.0;
    //float _o2383 = { 10000000000.0 }; //used to render the traps
    
    for (int i = 0; i < iMax; i++)
    {
        dz2 *= 9.0 * qLength2_float4(qSquare_float4(z));
        z = qCube_float4(z) + c;
        m2 = qLength2_float4(z);
        //(_o2383 = min(_o2383, (length((_z2379.xz - float2(0.44999999, 0.55000001))) - 0.1))); //used to render the traps
        if (m2 > 256.0)
        {
            break;
        }
        n += 1.0;
    }
    
    float d = (0.25 * log(m2)) * sqrt((m2 / dz2));
    //(_d2385 = min(_o2383, _d2385));//used to render the traps
    //(_d2385 = max(_d2385, _p.y));
    return float2(d, n);
}
float3 calcNormal(in float3 pos, in float4 c = kc, in int iMax = 200)
{
    const float2 e = float2(1.0f, -1.0f) * 0.5773f * kPrecis;
    
    return normalize(
        e.xyy * jdist(pos + e.xyy, c, iMax).x +
        e.yyx * jdist(pos + e.yyx, c, iMax).x +
        e.yxy * jdist(pos + e.yxy, c, iMax).x +
        e.xxx * jdist(pos + e.xxx, c, iMax).x
    );
}
float2 raycast(in float3 ro, in float3 rd, in float4 c = kc, in float deltaT = 0.0, in int iMax = 200)
{
    float tmax = 7000.f;
    float tmin = kPrecis;
 
    float upperPlane = deltaT;
    
    float tpS = ( upperPlane - ro.y) / rd.y;
    
    bool isCamAboveUpper = (ro.y > upperPlane);
    
    if (tpS > 0.0)
    {
        if (isCamAboveUpper)
        {
            tmin = max(tmin, tpS);
        }
        else
        {
            tmax = min(tmax, tpS);
        }
    }
    else
    {
        if (isCamAboveUpper)
        {
            return float2(-2.0, 0.0);
        }
    }
    
    float lowerPlane = -1.1;
    float tpF = (lowerPlane - ro.y) / rd.y;
    
    bool isCamBellowLower = (ro.y < lowerPlane);
    
    if (tpF > 0.0)
    {
        if (isCamBellowLower)
        {
            tmin = max(tmin, tpF);
        }
        else
        {
            tmax = min(tmax, tpF);
        }
    }
    else
    {
        if (isCamBellowLower)
        {
            return float2(-2.0, 0.0);
        }
    }
    
    float2 bv = iSphere(ro, rd, 1.2);
    if (bv.y < 0.0)
    {
        return float2(-2.0, 0.0);
    }
    tmin = max(tmin, bv.x);
    tmax = min(tmax, bv.y);
    float2 res = { -1.0, -1.0 };
    float t = tmin;
    float lt = { 0.0 };
    float lh = { 0.0 };
    {
        for (int i = 0; i < 1024; i++)
        {
            res = jdist(ro + (rd * t), c, iMax);
            if (res.x < kPrecis)
            {
                break;
            }
            lt = t;
            lh = res.x;
            //(_t2399 += (min(_res2398.x, 0.0099999998) * 1.0));//used to render the traps
            t += min(res.x, 0.2);
            if (t > tmax)
            {
                break;
            }
        }
    }
    if ((lt > 9.9999997e-05) && (res.x < 0.0))
    {
        t = lt - ((lh * (t - lt)) / (res.x - lh));
    }
    float s = 0 ;
    if (t < tmax)
    {
        s = t;
    }
    else
    {
        s = -1.0;
    }
    res.x = s;
    return res;
}
float3 colorSurface(in float3 pos, in float2 tn)
{
    float3 col = 0.5 + 0.5 * cos(log2(tn.y) * 0.9 + 3.5 + float3(0.0, 0.6, 1.0));
    
    if (pos.y > 0.0)
    {
        col = lerp(col, float3(1.0, 1.0, 1.0), 0.2);
    }
    float inside = smoothstep(14., 15., tn.y);
    
   //sss return float3(_inside2408,0.,0.);
    
    col *= float3(0.45, 0.42, 0.40) + float3(0.55, 0.58, 0.60) * inside;
    col = lerp(col * col * (3.0 - 2.0 * col), col, inside);
    col = lerp(lerp(col, float3_ctor(dot(col, float3(0.3333, 0.333, 0.333))), -0.4), col, inside);
    
    float3 surfaceColor = clamp(col * 0.65, 0.0, 1.0);
    
    return surfaceColor;
}

bool JuliaDistance(in float3 ro, in float3 rd, inout float3 normal, inout float2 resT, in float time=0.3f)
{
    resT = 1e20;
    
    ro *= 0.4;
    
    ro.y += -1.f;//for the scene with multiples objects

    // cut animation
    int iAnimMin = 0;
    int iAnimMax = 600;
    
    float minCut = -0.3f;
    float maxCut = 0.6f;
    
    int iMax = (10.5*time) % (iAnimMax*2);
    if(iMax>iAnimMax) 
    {
        iMax = 2*iAnimMax - iMax; 
    }
    iMax += iAnimMin;
    
    float deltaT = minCut + maxCut*float(iMax)/float(iAnimMax);
    //float deltaT = 0.3f;//minCut + maxCut*float(iMax)/float(iAnimMax);
    
    // julia animation
    //float4 c = 0.45*cos( float4(0.5,3.9,1.4,1.1) + 0.01*(time+100.)*float4(1.2,1.7,1.3,2.5) ) - float4(0.3,0.0,0.0,0.0);
    float4 c = kc;
    
    //float2 tn = f_raycast(ro, rd, c, deltaT, iMax+2);
    //float2 tn = f_raycast(ro, rd, c, deltaT, 200);
    //float2 tn = f_raycast(ro, rd, c);
    float2 tn = raycast(ro, rd);
    
    bool cond = (tn.x >= 0.0);
    if (cond)
    {
        float3 pos = (ro + (tn.x * rd));
        //normal = f_calcNormal(_pos2419, c, iMax+2);
        normal = calcNormal(pos);
        resT = tn;
    }
    return cond;
}

[shader("intersection")]
void Intersection_Julia()
{
    Ray localRay = GetRayInAABBPrimitiveLocalSpace();

    float2 thit;
    ProceduralPrimitiveAttributes attr = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f} };
    
    // DEBUG
    if(g_sceneCB.debugFlag)
    {
        ReportHit(0, 1, attr);
        return;
    }
    
    float3 pos;
    bool primitiveTest = JuliaDistance(localRay.origin, localRay.direction, attr.normal, thit, g_sceneCB.elapsedTime);
   
    if (primitiveTest && thit.x < RayTCurrent())
    {
        InstanceBuffer aabbAttribute = g_instanceBuffer[l_aabbCB.instanceIndex];
        attr.normal = normalize(mul(attr.normal, (float3x3) WorldToObject3x4()));
        attr.color = float4(thit, 0.f, 0.f); // Using the color parameter to send intersection min and max t.
        
        ReportHit(thit.x, /*hitKind*/ 0, attr);
    }
}

[shader("closesthit")]
void ClosestHit_Julia(inout RayPayload rayPayload, in ProceduralPrimitiveAttributes attr)
{
    // DEBUG
    if (g_sceneCB.debugFlag)
    {
        rayPayload.color = float4(1.f, 0.f, 0.0f, 0.f);
        return;
    }

    // Shadow component.
    // Trace a shadow ray.
    float3 hitPosition = HitWorldPosition();
    Ray shadowRay = { hitPosition, normalize(g_sceneCB.lightPosition.xyz - hitPosition) };
    bool shadowRayHit = false;//TraceShadowRayAndReportIfHit(shadowRay, rayPayload.recursionDepth);
    
    float3 pos = ObjectRayOrigin() + RayTCurrent() * ObjectRayDirection();
    float3 dir = WorldRayDirection();
    
    float4 albedo = float4(3.5*colorSurface(pos, attr.color.xy), 1.f);
    
    if (rayPayload.recursionDepth == MAX_RAY_RECURSION_DEPTH - 1)
    {
       albedo += 1.65 * step(0.0, abs(pos.y));
    }
    
    // Reflected component.
    float4 reflectedColor = float4(0, 0, 0, 0);
    
    float reflecCoef = 0.1;
    
    if (reflecCoef > 0.001)
    {
        // Trace a reflection ray.
        Ray reflectionRay = { hitPosition, reflect(WorldRayDirection(), attr.normal) };
        float4 reflectionColor = TraceRadianceRay(reflectionRay, rayPayload.recursionDepth);
        
        float3 fresnelR = FresnelReflectanceSchlick(WorldRayDirection(), attr.normal, albedo.xyz);
        reflectedColor = reflecCoef * float4(fresnelR, 1) * reflectionColor;
    }

    float diffuseCoef = 0.6;
    float specularCoef = 0.08;
    float specularPower = 0.2;
    // Calculate final color.
    float4 phongColor = CalculatePhongLighting(albedo, attr.normal, shadowRayHit, diffuseCoef, specularCoef, specularPower);
    float4 color = phongColor + reflectedColor;

    color += rayPayload.color;
    
    //// Apply visibility falloff.
    //rayPayload.dist+=RayTCurrent();
    //float t = rayPayload.dist;
        
    ////color = lerp(color, BackgroundColor + rayPayload.color, 1.0 - exp(-0.0000002*t*t*t));
   
    rayPayload.color = color;
}

#endif