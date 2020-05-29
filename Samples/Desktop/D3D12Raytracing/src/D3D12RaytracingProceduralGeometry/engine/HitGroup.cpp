#include "../stdafx.h"
#include "HitGroup.h"

namespace RtxEngine
{
	HitGroup::HitGroup(const wstring& anyHit, const wstring& closestHit, const wstring& intersection)
		: m_anyHit(anyHit),
		m_closestHit(closestHit),
		m_intersection(intersection)
	{}
}