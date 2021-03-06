
#ifndef GRAPH3D_H
#define GRAPH3D_H

static float amplitude = 25.5;
static float spread = 10.;
static float3 gaussianCenter =  float3(45., 20.0, 45.);

static float amplitude2 = -25.5;
static float spread2 = 10.;
static float3 gaussianCenter2 =  float3(-5., 20.0, 35.);

//Initial conditions of a geodesic
struct graphRay
{
    float3 origin;
    float3 dir;
};
//*********************************************************************************************************

// FUNCTION SETUP

//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_Gaussian(float3 p, int i, float3 p0, float a, float s)
{
    float aux = 0.f;

    aux = -(p[i - 1] - p0[i - 1]) / (s * s);

    return a * exp( -dot(p-p0,p-p0)/(2.f * s * s)) * aux;
}
//*********************************************************************************************************



//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_saddle(float3 p, int i)
{
    float result = 0.f;

    //f(x,y,z)=xyz
    if (i == 1)
    {
        result = p.y * p.z;
    }
    if (i == 2)
    {
        result = p.x * p.z;
    }
    if (i == 3)
    {
        result = p.x * p.y;
    }

    return result*0.001;
}
//*********************************************************************************************************



//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_saddle1(float3 p, int i)
{
    float result = 0.f;

    //f(x,y,z)=x^2+y^2-z^2;
    if (i == 1)
    {
        result = 2.f * p.x;
    }
    if (i == 2)
    {
        result = 2.f * p.y;
    }
    if (i == 3)
    {
        result = -2.f * p.z;
    }

    return result;
}
//*********************************************************************************************************



//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_paraboloid(float3 p, int i)
{
    float result = 0.f;
    
    //f(x,y,z)=x^2+y^2+z^2;
    if (i == 1)
    {
        result = 2.f * p.x;
    }
    if (i == 2)
    {
        result = 2.f * p.y;
    }
    if (i == 3)
    {
        result = 2.f * p.z;
    }

    return result;
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_Gaussian(float3 p, int i, int j, float3 p0, float a, float s)
{
    float d = 1.f;

    if (i != j)
    {
        d = 0.f;
    }

    float aux = ((p[j - 1] - p0[j - 1])*(p[i - 1] - p0[i - 1])) / (s * s * s * s) - d / (s * s);
    
    return a * exp( - dot(p-p0,p-p0) / (2.f * s * s)) * aux;
}
//*********************************************************************************************************



//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_saddle(float3 p, int i, int j)
{
    float result = 0.f;

    //f(x,y,z)=xyz
    if (i == j)
    {
        result = 0.f;
    }

    if ((i == 1 && j == 2) || (i == 2 && j == 1))
    {
        result = p.z;
    }

    if ((i == 1 && j == 3) || (i == 3 && j == 1))
    {
        result = p.y;
    }

    if ((i == 2 && j == 3) || (i == 3 && j == 2))
    {
        result = p.x;
    }

    return result*0.001;
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_saddle1(float3 p, int i, int j)
{
    float result = 0.f;

    //f(x,y,z)=x^2+y^2-z^2;
    if (i == j)
    {
        result = 2.f;
        if (i == 3)
        {
            result = -2.f;
        }
    }

    return result;
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
float diff_paraboloid(float3 p, int i, int j)
{
    float result = 0.f;

    //f(x,y,z)=x^2+y^2+z^2;
    if (i == j)
    {
        result = 2.f;
    }
    
    return result;
}
//*********************************************************************************************************



//*********************************************************************************************************
//first order partial derivative of the choosed function
//*********************************************************************************************************
float diff(float3 p, int i)
{
    //return diff_saddle(p, i);
    //return diff_saddle1(p, i);
   // return diff_Gaussian(p, i, -gLeftHandPosW, gScale, gSphericalScale) + diff_Gaussian(p, i, -gRightHandPosW, gScale, gSphericalScale);
     // return diff_Gaussian(p, i, gaussianCenter, amplitude, spread);
      return diff_Gaussian(p, i, gaussianCenter, amplitude, spread)+
             diff_Gaussian(p, i, gaussianCenter2, amplitude2, spread2);
   // return diff_paraboloid(p, i);
}
//*********************************************************************************************************

//*********************************************************************************************************
//second order partial derivative of the choosed function
//*********************************************************************************************************
float diff(float3 p, int i, int j)
{
   //return diff_saddle(p, i,j);
   // return diff_saddle1(p, i, j);
   // return diff_Gaussian(p, i, j, -gLeftHandPosW, gScale, gSphericalScale) + diff_Gaussian(p, i, j, -gRightHandPosW, gScale, gSphericalScale);
   // return diff_Gaussian(p, i, j, gaussianCenter, amplitude, spread);
    return diff_Gaussian(p, i, j, gaussianCenter, amplitude, spread)+
           diff_Gaussian(p, i, j, gaussianCenter2, amplitude2, spread2);
  //  return diff_paraboloid(p, i, j);
}
//*********************************************************************************************************



//*********************************************************************************************************
//gradient of the choosed function
//*********************************************************************************************************
float3 grad(float3 p)
{
    return float3(diff(p, 1), diff(p, 2), diff(p, 3));
}
//*********************************************************************************************************



//*********************************************************************************************************
//
//*********************************************************************************************************
graphRay sumGraphRay(graphRay ray1, graphRay ray2)
{
    ray1.origin += ray2.origin;
    ray1.dir += ray2.dir;

    return ray1;
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
graphRay mulGraphRay(graphRay ray, float t)
{
    ray.origin *= t;
    ray.dir *= t;

    return ray;
}
//*********************************************************************************************************


//*********************************************************************************************************
//Inner product
//*********************************************************************************************************
float graphDot(float3 u, float3 v, float3 p)
{
    float f1 = diff(p, 1);
    float f2 = diff(p, 2);
    float f3 = diff(p, 3);

    return  (u.x * v.x) * (1.f + f1 * f1) 
          + (u.y * v.y) * (1.f + f2 * f2) 
          + (u.z * v.z) * (1.f + f3 * f3)
          + (u.x * v.y + u.y * v.x) * f1 * f2 
          + (u.x * v.z + u.z * v.x) * f1 * f3
          + (u.y * v.z + u.z * v.y) * f2 * f3;
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
float graphNorm(float3 u, float3 p)
{
    return sqrt(graphDot(u, u, p));
}
//*********************************************************************************************************


//*********************************************************************************************************
//
//*********************************************************************************************************
float3 graphNormalize(float3 u, float3 p)
{
    return u / graphNorm(u, p);
}
//*********************************************************************************************************


//*********************************************************************************************************
// Rijks coefficient of the curvature tensor
//*********************************************************************************************************
float coefCurvTensor(float3 p, int i, int j, int k, int s)
{
    float result = diff(p, s, j)*diff(p, i, k) - diff(p, s, i)*diff(p, j, k);

    float3 gradf = grad(p);
    
    result /= (1.f + dot(gradf, gradf));
    
    return result;
}
//*********************************************************************************************************


//*********************************************************************************************************
// sectional curvature
//*********************************************************************************************************
float secCurv(float3 p, float3 u, float3 v)
{
    float result = 0.f;

    result += coefCurvTensor(p, 1, 2, 1, 2)*(u.x*v.y*u.x*v.y - u.x*v.y*u.y*v.x - u.y*v.x*u.x*v.y + u.y*v.x*u.y*v.x);
    result += coefCurvTensor(p, 1, 2, 1, 3)*(u.x*v.y*u.x*v.z - u.x*v.y*u.z*v.x - u.y*v.x*u.x*v.z + u.y*v.x*u.z*v.x);
    result += coefCurvTensor(p, 1, 2, 2, 3)*(u.x*v.y*u.y*v.z - u.x*v.y*u.z*v.y - u.y*v.x*u.y*v.z + u.y*v.x*u.z*v.y);
    
    result += coefCurvTensor(p, 1, 3, 1, 2)*(u.x*v.z*u.x*v.y - u.x*v.z*u.y*v.x - u.z*v.x*u.x*v.y + u.z*v.x*u.y*v.x);
    result += coefCurvTensor(p, 1, 3, 1, 3)*(u.x*v.z*u.x*v.z - u.x*v.z*u.z*v.x - u.z*v.x*u.x*v.z + u.z*v.x*u.z*v.x);
    result += coefCurvTensor(p, 1, 3, 2, 3)*(u.x*v.z*u.y*v.z - u.x*v.z*u.z*v.y - u.z*v.x*u.y*v.z + u.z*v.x*u.z*v.y);
    
    result += coefCurvTensor(p, 2, 3, 1, 2)*(u.y*v.z*u.x*v.y - u.y*v.z*u.y*v.x - u.z*v.y*u.x*v.y + u.z*v.y*u.y*v.x);
    result += coefCurvTensor(p, 2, 3, 1, 3)*(u.y*v.z*u.x*v.z - u.y*v.z*u.z*v.x - u.z*v.y*u.x*v.z + u.z*v.y*u.z*v.x);
    result += coefCurvTensor(p, 2, 3, 2, 3)*(u.y*v.z*u.y*v.z - u.y*v.z*u.z*v.y - u.z*v.y*u.y*v.z + u.z*v.y*u.z*v.y);
    
    return result;
}
//*********************************************************************************************************

//*********************************************************************************************************
// sectional scalar
//*********************************************************************************************************
float scalarCurv(float3 p)
{
    float result = 0.f;
    
    float f1 = diff(p, 1);
    float f2 = diff(p, 2);
    float f3 = diff(p, 3);

    result += 2.f*coefCurvTensor(p, 1, 2, 1, 2)*(1.f + f3*f3);
    result -= 4.f*coefCurvTensor(p, 1, 2, 1, 3)*(f2*f3);
    result += 4.f*coefCurvTensor(p, 1, 2, 2, 3)*(f1*f3);
    
    result += 2.f*coefCurvTensor(p, 1, 3, 1, 3)*(1.f + f2*f2);
    result -= 4.f*coefCurvTensor(p, 1, 3, 2, 3)*(f1*f2);
    
    result += 2.f*coefCurvTensor(p, 2, 3, 2, 3)*(1.f + f1*f1);
    
    float3 gradf = grad(p);
    
    result /= (1.f + dot(gradf, gradf));
    
    return result;
}
//*********************************************************************************************************


//*********************************************************************************************************
//  Derivative of the geodesic tangent vector
//*********************************************************************************************************
float3 yLine(graphRay ray)
{
    float3 p = ray.origin;
    float3 y = ray.dir;
    
    float3 gradient = grad(p);
    
    float D = 1.f + dot(gradient, gradient);
    
    float b = - (diff(p, 1, 1) * y.x * y.x + 
                 diff(p, 2, 2) * y.y * y.y + 
                 diff(p, 3, 3) * y.z * y.z + 
          2.f * (diff(p, 1, 2) * y.x * y.y + 
                 diff(p, 1, 3) * y.x * y.z + 
                 diff(p, 2, 3) * y.y * y.z))/D;
   
    float3 result = float3(0.f, 0.f, 0.f);

    result.x = diff(p, 1) * b;
    result.y = diff(p, 2) * b;
    result.x = diff(p, 3) * b;

    return result;
}
//*********************************************************************************************************


//*********************************************************************************************************
//Evaluate the derivative of the geodesic leaving p in the direction of v
//*********************************************************************************************************
graphRay evalGraphRay(graphRay ray, float t)
{
    ray.dir = graphNormalize(ray.dir, ray.origin);
    //ray.dir = normalize(ray.dir);
    
    //differential ray
    graphRay dRay;
    dRay.origin = ray.dir;
    dRay.dir = yLine(ray);

    //Euler methold
    ray = sumGraphRay(ray, mulGraphRay(dRay, t));

    //ray.dir = graphNormalize(ray.dir, ray.origin);
    //ray.dir = normalize(ray.dir);
    
    return ray;
}
//*********************************************************************************************************

void evalGraphRay(inout float3 p, inout float3 v, float t)
{
   graphRay nextRay;
   nextRay.origin = p;
   nextRay.dir = v;
   
   nextRay = evalGraphRay(nextRay,t);
   
   p = nextRay.origin;
   v = nextRay.dir;
}

//*********************************************************************************************************
#endif 