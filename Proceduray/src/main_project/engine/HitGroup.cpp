#include "../stdafx.h"
#include "HitGroup.h"

#include <codecvt>
#include <string>

namespace RtxEngine
{
	HitGroup::HitGroup(const string& name, const wstring& internalName, const wstring& anyHit, const wstring& closestHit,
		const wstring& intersection)
		: Entity(name),
		wName(internalName),
		anyHit(anyHit),
		closestHit(closestHit),
		intersection(intersection)
	{}
}