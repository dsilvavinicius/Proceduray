#ifndef JULIASETS_H
#define JULIASETS_H

float2 vec2_ctor(float x0, float x1)
{
    return float2(x0, x1);
}
float3 vec3_ctor(float x0)
{
    return float3(x0, x0, x0);
}
float3 vec3_ctor(float x0, float x1, float x2)
{
    return float3(x0, x1, x2);
}
float3 vec3_ctor(float2 x0, float x1)
{
    return float3(x0, x1);
}
float3x3 mat3_ctor(float x0, float x1, float x2, float x3, float x4, float x5, float x6, float x7, float x8)
{
    return float3x3(x0, x1, x2, x3, x4, x5, x6, x7, x8);
}
float4 vec4_ctor(float x0, float3 x1)
{
    return float4(x0, x1);
}
float4 vec4_ctor(float3 x0, float x1)
{
    return float4(x0, x1);
}

float4 f_qSquare_float4(in float4 _q)
{
    return vec4_ctor(((((_q.x * _q.x) - (_q.y * _q.y)) - (_q.z * _q.z)) - (_q.w * _q.w)), ((2.0 * _q.x) * _q.yzw));
}
float4 f_qCube_float4(in float4 _q)
{
    float4 _q22359 = (_q * _q);
    return vec4_ctor((_q.x * (((_q22359.x - (3.0 * _q22359.y)) - (3.0 * _q22359.z)) - (3.0 * _q22359.w))), (_q.yzw * ((((3.0 * _q22359.x) - _q22359.y) - _q22359.z) - _q22359.w)));
}
float f_qLength2_float4(in float4 _q)
{
    return dot(_q, _q);
}
float2 f_iSphere(in float3 _ro, in float3 _rd, in float _rad)
{
    float _b2366 = dot(_ro, _rd);
    float _c2367 = (dot(_ro, _ro) - (_rad * _rad));
    float _h2368 = ((_b2366 * _b2366) - _c2367);
    if ((_h2368 < 0.0))
    {
        return float2(-1.0, -1.0);
    }
    (_h2368 = sqrt(_h2368));
    return vec2_ctor(((-_b2366) - _h2368), ((-_b2366) + _h2368));
}
float3x3 f_setCamera(in float3 _ro, in float3 _ta, in float _cr)
{
    float3 _cw2373 = normalize((_ta - _ro));
    float3 _cp2374 = vec3_ctor(sin(_cr), cos(_cr), 0.0);
    float3 _cu2375 = normalize(cross(_cw2373, _cp2374));
    float3 _cv2376 = normalize(cross(_cu2375, _cw2373));
    float3 s981 = _cu2375;
    float3 s982 = _cv2376;
    float3 s983 = _cw2373;
    return mat3_ctor(s981[0], s981[1], s981[2], s982[0], s982[1], s982[2], s983[0], s983[1], s983[2]);
}
float2 f_map(in float3 _p, in float4 c, in int iMax)
{
    float4 _z2379 = vec4_ctor(_p, 0.0);
    float _dz22380 = { 1.0 };
    float _m22381 = { 0.0 };
    float _n2382 = { 0.0 };
    //float _o2383 = { 10000000000.0 }; //used to render the traps
{
        for (int _i2384 = { 0 }; (_i2384 < iMax); (_i2384++))
        {
            (_dz22380 *= (9.0 * f_qLength2_float4(f_qSquare_float4(_z2379))));
            (_z2379 = (f_qCube_float4(_z2379) + c));
            (_m22381 = f_qLength2_float4(_z2379));
            //(_o2383 = min(_o2383, (length((_z2379.xz - float2(0.44999999, 0.55000001))) - 0.1))); //used to render the traps
            if ((_m22381 > 256.0))
            {
                break;
            }
            (_n2382 += 1.0);
        }
    }
    float _d2385 = ((0.25 * log(_m22381)) * sqrt((_m22381 / _dz22380)));
    //(_d2385 = min(_o2383, _d2385));//used to render the traps
    //(_d2385 = max(_d2385, _p.y));
    return vec2_ctor(_d2385, _n2382);
}
float3 f_calcNormal(in float3 _pos, in float4 c = float4(-0.090909094, 0.27272728, 0.68181819, -0.27272728), in int iMax = 200)
{
    return normalize(((((float3(0.00014432501, -0.00014432501, -0.00014432501) * f_map((_pos + float3(0.00014432501, -0.00014432501, -0.00014432501)), c, iMax).x) + (float3(-0.00014432501, -0.00014432501, 0.00014432501) * f_map((_pos + float3(-0.00014432501, -0.00014432501, 0.00014432501)), c, iMax).x)) + (float3(-0.00014432501, 0.00014432501, -0.00014432501) * f_map((_pos + float3(-0.00014432501, 0.00014432501, -0.00014432501)), c, iMax).x)) + (float3(0.00014432501, 0.00014432501, 0.00014432501) * f_map((_pos + float3(0.00014432501, 0.00014432501, 0.00014432501)), c, iMax).x)));
}
float2 f_raycast(in float3 _ro, in float3 _rd, in float4 c = float4(-0.090909094, 0.27272728, 0.68181819, -0.27272728), in float deltaT = 0.0, in int iMax = 200)
{
    float _tmax2392 = { 7000.f };
    float _tmin2393 = { 0.00025000001f };
 
    float upperPlane = deltaT;
    
    float _tpS2395 = (( upperPlane - _ro.y) / _rd.y);
    
    bool isCamAboveUpper = (_ro.y > upperPlane);
    
    if ((_tpS2395 > 0.0))
    {
        if (isCamAboveUpper)
        {
            (_tmin2393 = max(_tmin2393, _tpS2395));
        }
        else
        {
            (_tmax2392 = min(_tmax2392, _tpS2395));
        }
    }
    else
    {
        if (isCamAboveUpper)
        {
            return float2(-2.0, 0.0);
        }
    }
    
    float lowerPlane = -1.10000001;
    float _tpF2396 = ((lowerPlane - _ro.y) / _rd.y);
    
    bool isCamBellowLower = (_ro.y < lowerPlane);
    
    if ((_tpF2396 > 0.0))
    {
        if (isCamBellowLower)
        {
            (_tmin2393 = max(_tmin2393, _tpF2396));
        }
        else
        {
            (_tmax2392 = min(_tmax2392, _tpF2396));
        }
    }
    else
    {
        if (isCamBellowLower)
        {
            return float2(-2.0, 0.0);
        }
    }
    
    float2 _bv2397 = f_iSphere(_ro, _rd, 1.2);
    if ((_bv2397.y < 0.0))
    {
        return float2(-2.0, 0.0);
    }
    (_tmin2393 = max(_tmin2393, _bv2397.x));
    (_tmax2392 = min(_tmax2392, _bv2397.y));
    float2 _res2398 = { -1.0, -1.0 };
    float _t2399 = _tmin2393;
    float _lt2400 = { 0.0 };
    float _lh2401 = { 0.0 };
{
        for (int _i2402 = { 0 }; (_i2402 < 1024); (_i2402++))
        {
            (_res2398 = f_map((_ro + (_rd * _t2399)), c, iMax));
            if ((_res2398.x < 0.00025000001))
            {
                break;
            }
            (_lt2400 = _t2399);
            (_lh2401 = _res2398.x);
            //(_t2399 += (min(_res2398.x, 0.0099999998) * 1.0));//used to render the traps
            (_t2399 += min(_res2398.x, 0.2));
            if ((_t2399 > _tmax2392))
            {
                break;
            }
        }
    }
    if (((_lt2400 > 9.9999997e-05) && (_res2398.x < 0.0)))
    {
        (_t2399 = (_lt2400 - ((_lh2401 * (_t2399 - _lt2400)) / (_res2398.x - _lh2401))));
    }
    float s984 = { 0 };
    if ((_t2399 < _tmax2392))
    {
        (s984 = _t2399);
    }
    else
    {
        (s984 = -1.0);
    }
    (_res2398.x = s984);
    return _res2398;
}
float3 f_colorSurface(in float3 _pos, in float2 _tn)
{
    float3 _col2407 = (0.5 + (0.5 * cos((((log2(_tn.y) * 0.89999998) + 3.5) + float3(0.0, 0.60000002, 1.0)))));
    
    if ((_pos.y > 0.0))
    {
        (_col2407 = lerp(_col2407, float3(1.0, 1.0, 1.0), 0.2));
    }
    float _inside2408 = smoothstep(14.,15., _tn.y);
    
   //sss return float3(_inside2408,0.,0.);
    
    (_col2407 *= (float3(0.44999999, 0.41999999, 0.40000001) + (float3(0.55000001, 0.57999998, 0.60000002) * _inside2408)));
    (_col2407 = lerp(((_col2407 * _col2407) * (3.0 - (2.0 * _col2407))), _col2407, _inside2408));
    (_col2407 = lerp(lerp(_col2407, vec3_ctor(dot(_col2407, float3(0.33329999, 0.33329999, 0.33329999))), -0.40000001), _col2407, _inside2408));
    
    float3 surfaceColor = clamp((_col2407 * 0.64999998), 0.0, 1.0);
    
    return surfaceColor;
}

bool JuliaDistance(in float3 _ro, in float3 _rd, inout float3 normal, inout float2 _resT, in float time=0.3f)
{
    (_resT = 100000002004087734272.0);
    
    _ro *= 0.4;
    
    _ro.y += -1.f;//for the scene with multiples objects

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
    float4 c = float4(-0.090909094, 0.27272728, 0.68181819, -0.27272728);
    
    //float2 tn = f_raycast(_ro, _rd, c, deltaT, iMax+2);
    //float2 tn = f_raycast(_ro, _rd, c, deltaT, 200);
    //float2 tn = f_raycast(_ro, _rd, c);
    float2 tn = f_raycast(_ro, _rd);
    
    bool cond = (tn.x >= 0.0);
    if (cond)
    {
        float3 _pos2419 = (_ro + (tn.x * _rd));
        //normal = f_calcNormal(_pos2419, c, iMax+2);
        normal = f_calcNormal(_pos2419);
        (_resT = tn);
    }
    return cond;
}

#endif