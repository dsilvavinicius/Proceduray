#ifndef ATTRIB_STRUCTS_H
#define ATTRIB_STRUCTS_H

#ifdef HLSL
#include "..\util\HlslCompat.h"
#else
using namespace DirectX;
#endif

// Attribute structures. The attribute structure is the type of the output of an intersection shader. Consequentialy, it is the type
// of one of the inputs to the any/closest hit shaders.

struct ProceduralPrimitiveAttributes
{
	XMFLOAT3 normal;
	XMFLOAT4 color;
};

#endif