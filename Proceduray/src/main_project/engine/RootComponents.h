#ifndef ROOT_COMPONENTS_H
#define ROOT_COMPONENTS_H

#ifdef HLSL
#include "..\util\HlslCompat.h"
#else
using namespace DirectX;
#endif

// Root components are the components of Root arguments.

// Attributes per primitive type.
struct PrimitiveConstantBuffer
{
	XMFLOAT4 albedo;
	float reflectanceCoef;
	float diffuseCoef;
	float specularCoef;
	float specularPower;
	float stepScale;                      // Step scale for ray marching of signed distance primitives. 
										  // - Some object transformations don't preserve the distances and 
										  //   thus require shorter steps.
	XMFLOAT3 padding;
};

// Attributes per primitive instance.
struct PrimitiveInstanceConstantBuffer
{
	UINT instanceIndex;
	UINT primitiveType; // Procedural primitive type
};

struct SceneConstantBuffer
{
	XMMATRIX projectionToWorld;
	XMVECTOR cameraPosition;
	XMVECTOR lightPosition;
	XMVECTOR lightAmbientColor;
	XMVECTOR lightDiffuseColor;
	float    reflectance;
	float    elapsedTime;                 // Elapsed application time.
	int		 debugFlag;
};

// Dynamic attributes per primitive instance.
struct InstanceBuffer
{
	XMMATRIX localSpaceToBottomLevelAS;   // Matrix from local primitive space to bottom-level object space.
	XMMATRIX bottomLevelASToLocalSpace;   // Matrix from bottom-level object space to local primitive space.
};

#endif