#pragma once

// Attribute structures. The attribute structure is the type of the output of an intersection shader. Consequentialy, it is the type
// of one of the inputs to the any/closest hit shaders.

struct ProceduralPrimitiveAttributes
{
	XMFLOAT3 normal;
};