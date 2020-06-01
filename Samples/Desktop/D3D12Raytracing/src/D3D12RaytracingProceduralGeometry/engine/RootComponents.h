#pragma once

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