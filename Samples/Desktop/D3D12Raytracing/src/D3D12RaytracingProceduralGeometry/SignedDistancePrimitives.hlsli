//*********************************************************************************
//
// This file is based on or incorporates material from the projects listed below 
// (Third Party OSS). The original copyright notice and the license under which 
// Microsoft received such Third Party OSS, are set forth below. Such licenses 
// and notices are provided for informational purposes only. Microsoft licenses 
// the Third Party OSS to you under the licensing terms for the Microsoft product 
// or service. Microsoft reserves all other rights not expressly granted under 
// this agreement, whether by implication, estoppel or otherwise.
//
// MIT License
// Copyright(c) 2013 Inigo Quilez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files(the Software), to deal 
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is furnished 
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS 
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
// IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//*********************************************************************************

//**********************************************************************************************
//
// SignedDistanceFieldLibrary.h
//
// A list of useful distance function to simple primitives, and an example on how to 
// do some interesting boolean operations, repetition and displacement.
// More info here: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//
//**********************************************************************************************

#ifndef SIGNEDDISTANCEPRIMITIVES_H
#define SIGNEDDISTANCEPRIMITIVES_H


#include "RaytracingShaderHelper.hlsli"

//------------------------------------------------------------------
float GetDistanceFromSignedDistancePrimitive(in float3 position, in SignedDistancePrimitive::Enum sdPrimitive, in float time);

//------------------------------------------------------------------

// Subtract: Obj1 - Obj2
float opS(float d1, float d2)
{
    return max(d1, -d2);
}

// Union: Obj1 + Obj2
float opU(float d1, float d2)
{
    return min(d1, d2);
}

// Intersection: Obj1 & Obj2
float opI(float d1, float d2)
{
    return max(d1, d2);
}

// Repetitions
float3 opRep(float3 p, float3 c)
{
    return fmod(p, c) - 0.5 * c;
} 

// Polynomial smooth min/union (k = 0.1)
// Ref: http://www.iquilezles.org/www/articles/smin/smin.htm
float smin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5*(b - a) / k, 0.0, 1.0);
    return lerp(b, a, h) - k * h*(1.0 - h);
}


// Polynomial smooth min/union (k = 0.1)
float smax(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5*(b - a) / k, 0.0, 1.0);
    return lerp(a, b, h) + k * h*(1.0 - h);
}

// Smooth blend as union 
float opBlendU(float d1, float d2)
{
    return smin(d1, d2, 0.1);
}

// Smooth blend as intersect 
float opBlendI(float d1, float d2)
{
    return smax(d1, d2, 0.1);
}


// Twist
float3 opTwist(float3 p)
{
    float c = cos(3.0 * p.y);
    float s = sin(3.0 * p.y);
    float2x2 m = float2x2(c, -s, s, c);
    return float3(mul(m, p.xz), p.y);
}


//------------------------------------------------------------------

float sdPlane(float3 p)
{
    return p.y;
}

float sdSphere(float3 p, float s)
{
    return length(p) - s;
}

// Box extents: <-b,b>
float sdBox(float3 p, float3 b)
{
    float3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sdEllipsoid(in float3 p, in float3 r)
{
    return (length(p / r) - 1.0) * min(min(r.x, r.y), r.z);
}

float udRoundBox(float3 p, float3 b, float r)
{
    return length(max(abs(p) - b, 0.0)) - r;
}

// t: {radius, tube radius}
float sdTorus(float3 p, float2 t)
{
    float2 q = float2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdHexPrism(float3 p, float2 h)
{
    float3 q = abs(p);
    float d1 = q.z - h.y;
    float d2 = max((q.x * 0.866025 + q.y * 0.5), q.y) - h.x;
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdCapsule(float3 p, float3 a, float3 b, float r)
{
    float3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdEquilateralTriangle(in float2 p)
{
    const float k = 1.73205;    //sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0) p = float2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x += 2.0 - 2.0 * clamp((p.x + 2.0) / 2.0, 0.0, 1.0);
    return -length(p) * sign(p.y);
}

float sdTriPrism(float3 p, float2 h)
{
    float3 q = abs(p);
    float d1 = q.z - h.y;
#if 1
    // distance bound
    float d2 = max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5;
#else
    // correct distance
    h.x *= 0.866025;
    float d2 = sdEquilateralTriangle(p.xy / h.x) * h.x;
#endif
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdCylinder(float3 p, float2 h)
{
    float2 d = abs(float2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdCone(in float3 p, in float3 c)
{
    float2 q = float2(length(p.xz), p.y);
    float d1 = -q.y - c.z;
    float d2 = max(dot(q, c.xy), q.y);
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

float sdConeSection(in float3 p, in float h, in float r1, in float r2)
{
    float d1 = -p.y - h;
    float q = p.y - h;
    float si = 0.5 * (r1 - r2) / h;
    float d2 = max(sqrt(dot(p.xz, p.xz) * (1.0 - si * si)) + q * si - r2, q);
    return length(max(float2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}


// h = { sin a, cos a, height of a pyramid }
// a = pyramid's inner angle between its side plane and a ground plane.
// Octahedron position - ground plane intersecting in the middle.
float sdOctahedron(float3 p, float3 h)
{
    float d = 0.0;

    // Get distance against pyramid's sides going through origin.
    // Test: d = p.x * sin a + p.y * cos a
    d = dot(float2(max(abs(p.x), abs(p.z)), abs(p.y)), 
            float2(h.x, h.y));

    // Subtract distance to a side when at height h.z from the origin.
    return d - h.y * h.z;
}

// h = { sin a, cos a, height of a pyramid}
// a = pyramid's inner angle between its side plane and a ground plane.
// Pyramid position - sitting on a ground plane.
float sdPyramid(float3 p, float3 h) // h = { sin a, cos a, height }
{
    float octa = sdOctahedron(p, h);

    // Subtract bottom half
    return opS(octa, p.y);
}


float length_toPowNegative6(float2 p)
{
    p = p * p * p; 
    p = p * p;
    return pow(p.x + p.y, 1.0 / 6.0);
}

float length_toPowNegative8(float2 p)
{
    p = p * p; p = p * p; p = p * p;
    return pow(p.x + p.y, 1.0 / 8.0);
}

float sdTorus82(float3 p, float2 t)
{
    float2 q = float2(length(p.xz) - t.x, p.y);
    return length_toPowNegative8(q) - t.y;
}

float sdTorus88(float3 p, float2 t)
{
    float2 q = float2(length_toPowNegative8(p.xz) - t.x, p.y);
    return length_toPowNegative8(q) - t.y;
}

float sdCylinder6(float3 p, float2 h)
{
    return max(length_toPowNegative6(p.xz) - h.x, abs(p.y) - h.y);
}

//by tiago
float mobiusStrip(float3 p)
{
    float result = 1000000.f;//sdCapsule(position, float3(20,0,0), float3(0,1,0), float(1.5));
    
    int n_copies = 70;
    
    float R = 25.f;
    float s = 10.;
    
    for (int i = 0; i<n_copies; i++)
    {
        float theta = ((float)i/(float)(n_copies-1))*3.14159265359*2.0;
      
        float3 center = R*float3(cos(theta),0,sin(theta));//- /*float3(0,10,0) +*/ float3(cos(theta)*10.f,-2,sin(theta)*10.f);
        float3 Xv = float3((cos(theta/2.)/2.)*cos(theta),sin(theta/2.)/2.,(cos(theta/2.)/2.)*sin(theta));
      
        float3 a =  center + s*Xv;
        float3 b =  center - s*Xv;
      
        result = smin(result, sdCapsule(p, a, b, float(.6)),0.6);
    }
    
    return result;
}

//by tiago
float torusVisgraf(float3 p)
{
    float2 r1 = float2(6.f,3.f);
    float2 r2 = float2(6.f,3.f);
    
    float3 rot_p = float3(p.y, -p.x, p.z + 6.);
    
    return opU(sdTorus(p,r1), sdTorus(rot_p,r2));
}

//by tiago
float pacMan(float3 p, float t = 0.5 /*t must be between 0 and 1*/)
{
    float r1 = float(0.5f); //head radius
    float2 r2 = float2(0.45f*t+0.05f*(1.f-t), 0.5f); //mouth radius
    
    float3 rot_p = float3(p.z, p.y, -p.x); //mouth orientation
    rot_p = float3(rot_p.y, -rot_p.x + 0.5, rot_p.z); //mouth orientation
    float mouth = sdTriPrism(rot_p, r2);
   
    float eyes = opU(sdSphere(p + float3(-0.25, 0.365, -0.225), 0.075), sdSphere(p + float3(0.25, 0.365, -0.225), 0.075));
   
    return opS(opS(sdSphere(p, r1), mouth), eyes);
}

float2 isphere( in float4 sph, in float3 ro, in float3 rd )
{
    float3 oc = ro - sph.xyz;
    
	float b = dot(oc,rd);
	float c = dot(oc,oc) - sph.w*sph.w;
    float h = b*b - c;
    
    if( h<0.0 ) return float2(-1.0, -1.0);

    h = sqrt( h );

    return -b + float2(-h,h);
}


float3 sdCalculateNormal(in float3 pos, in SignedDistancePrimitive::Enum sdPrimitive, in float time)
{
    float2 e = float2(1.0, -1.0) * 0.5773 * 0.0001;
    return normalize(
        e.xyy * GetDistanceFromSignedDistancePrimitive(pos + e.xyy, sdPrimitive, time) +
        e.yyx * GetDistanceFromSignedDistancePrimitive(pos + e.yyx, sdPrimitive, time) +
        e.yxy * GetDistanceFromSignedDistancePrimitive(pos + e.yxy, sdPrimitive, time) +
        e.xxx * GetDistanceFromSignedDistancePrimitive(pos + e.xxx, sdPrimitive, time));
}

float map( in float3 p, out float4 resColor )
{
    float3 w = p;
    float m = dot(w,w);

    float4 trap = float4(abs(w),m);
	float dz = 1.;
    
    
	for( int i=0; i<4; i++ )
    {
#define PERFORMANCE
#ifdef PERFORMANCE
        float m2 = m*m;
        float m4 = m2*m2;
		dz = 8.0*sqrt(m4*m2*m)*dz + 1.0;

        float x = w.x; float x2 = x*x; float x4 = x2*x2;
        float y = w.y; float y2 = y*y; float y4 = y2*y2;
        float z = w.z; float z2 = z*z; float z4 = z2*z2;

        float k3 = x2 + z2;
        float k2 = rsqrt( k3*k3*k3*k3*k3*k3*k3 );
        float k1 = x4 + y4 + z4 - 6.0*y2*z2 - 6.0*x2*y2 + 2.0*z2*x2;
        float k4 = x2 - y2 + z2;

        w.x = p.x +  64.0*x*y*z*(x2-z2)*k4*(x4-6.0*x2*z2+z4)*k1*k2;
        w.y = p.y + -16.0*y2*k3*k4*k4 + k1*k1;
        w.z = p.z +  -8.0*y*k4*(x4*x4 - 28.0*x4*x2*z2 + 70.0*x4*z4 - 28.0*x2*z2*z4 + z4*z4)*k1*k2;
#else
        dz = 8.0*pow(sqrt(m),7.0)*dz + 1.0;
		//dz = 8.0*pow(m,3.5)*dz + 1.0;
        
        float r = length(w);
        float b = 8.0*acos( w.y/r);
        float a = 8.0*atan2( w.x, w.z );
        w = p + pow(r,8.0) * float3( sin(b)*sin(a), cos(b), sin(b)*cos(a) );
#endif        
        
        trap = min( trap, float4(abs(w),m) );

        m = dot(w,w);
		if( m > 256.0 )
            break;
    }

    resColor = float4(m,trap.yzw);

    return 0.25*log(m)*sqrt(m)/dz;
}

float3 calcNormal( in float3 pos, in float t )
{
    float4 tmp;
    float2 e = float2(1.0,-1.0)*0.5773 * 0.0001;
    return normalize( e.xyy*map( pos + e.xyy,tmp ) + 
					  e.yyx*map( pos + e.yyx,tmp ) + 
					  e.yxy*map( pos + e.yxy,tmp ) + 
					  e.xxx*map( pos + e.xxx,tmp ) );
}

bool MandelbulbDistance(in Ray ray, in float time, int instanceId, out float thit, out ProceduralPrimitiveAttributes attr,
                        in float stepScale = 1.0f)
{
    float res = -1.0;

    float3 ro = ray.origin;
    float3 rd = ray.direction;
    
    // bounding sphere
    float2 dis = isphere( float4(0.0,0.0,0.0, 1.25), ro, rd );
    if( dis.y<0.0 )
        return false;
    dis.x = max( dis.x, 0.0 );
    dis.y = min( dis.y, 1000.0 );

    // raymarch fractal distance field
	float4 trap = {0.f, 0.f, 0.f, 0.f};

	float t = dis.x;
    float3 pos = {0.f, 0.f, 0.f};
    
    // Number of iterations is used to animate the Mandelbulbs.
    int iAnimMin = 1;
    int iAnimMax = 128;
    int iMax = (0.5*pow(time,2)+instanceId) % (iAnimMax*2);
    if(iMax>iAnimMax) 
    {
        iMax = 2*iAnimMax - iMax; 
    }
   
    iMax += iAnimMin;
    //iMax = 128;
    
	for( int i=0; i<iMax; i++  )
    { 
        pos = ro + rd*t;
        float th =  0.0001*t;
		float h = map( pos, trap );
		if( t>dis.y || h<th ) break;
        t += h;
    }
    
    if( t<dis.y )
    {
        float3 hitSurfaceNormal = calcNormal(pos, t);
        thit = t;
        attr.normal = hitSurfaceNormal;
        attr.color = trap;
        return true;
    }

    return false;
}

// Test ray against a signed distance primitive.
// Ref: https://www.scratchapixel.com/lessons/advanced-rendering/rendering-distance-fields/basic-sphere-tracer
bool RaySignedDistancePrimitiveTest(in Ray ray, in SignedDistancePrimitive::Enum sdPrimitive, out float thit, out ProceduralPrimitiveAttributes attr, in float stepScale = 1.0f, in float time=1.f)
{
    const float threshold = 0.0001;
    float t = RayTMin();
    const UINT MaxSteps = 512;

    // Do sphere tracing through the AABB.
    UINT i = 0;
    while (i++ < MaxSteps && t <= RayTCurrent())
    {
        float3 position = ray.origin + t * ray.direction;
        float distance = GetDistanceFromSignedDistancePrimitive(position, sdPrimitive, time);

        // Has the ray intersected the primitive? 
        if (distance <= threshold * t)
        {
            float3 hitSurfaceNormal = sdCalculateNormal(position, sdPrimitive, time);
            if (IsAValidHit(ray, t, hitSurfaceNormal))
            {
                thit = t;
                attr.normal = hitSurfaceNormal;

                if (abs(abs(dot(hitSurfaceNormal, normalize(position))) - 1) > 0.001)
                {
                    attr.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
                }
                else 
                {
                    attr.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
                }

                return true;
            }
        }

        // Since distance is the minimum distance to the primitive, 
        // we can safely jump by that amount without intersecting the primitive.
        // We allow for scaling of steps per primitive type due to any pre-applied 
        // transformations that don't preserve true distances.
        t += stepScale * distance;
    }
    return false;
}

// Analytically integrated checkerboard grid (box filter).
// Ref: http://iquilezles.org/www/articles/filterableprocedurals/filterableprocedurals.htm
// ratio - Center fill to border ratio.
float CheckersTextureBoxFilter(in float2 uv, in float2 dpdx, in float2 dpdy, in UINT ratio)
{
    float2 w = max(abs(dpdx), abs(dpdy));   // Filter kernel
    float2 a = uv + 0.5*w;
    float2 b = uv - 0.5*w;

    // Analytical integral (box filter).
    float2 i = (floor(a) + min(frac(a)*ratio, 1.0) -
        floor(b) - min(frac(b)*ratio, 1.0)) / (ratio*w);
    return (1.0 - i.x)*(1.0 - i.y);
}


#endif // SIGNEDDISTANCEPRIMITIVES_H