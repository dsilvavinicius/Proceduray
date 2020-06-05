#include "../stdafx.h"
#include "HitGroup.h"

namespace RtxEngine
{
	HitGroup::HitGroup(const wstring& name, const wstring& anyHit, const wstring& closestHit, const wstring& intersection)
		: name(name),
		anyHit(anyHit),
		closestHit(closestHit),
		intersection(intersection)
	{}
}