#ifndef PAYLOADS_H
#define PAYLOADS_H

#ifdef HLSL
#include "..\util\HlslCompat.h"
#else
using namespace DirectX;
#endif

// The type of the ray payloads that are passed through the shaders.

struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
	bool hit;
};

struct ShadowRayPayload
{
	bool hit;
};

#endif