#pragma once

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