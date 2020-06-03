#ifndef PAYLOADS_H
#define PAYLOADS_H

// The type of the ray payloads that are passed through the shaders.

struct RayPayload
{
	XMFLOAT4 color;
	UINT   recursionDepth;
};

struct ShadowRayPayload
{
	bool hit;
};

#endif