#include "HitGroup.h"

namespace RtxEngine
{
	HitGroup::HitGroup(const string& anyHit, const string& closestHit, const string& intersection)
		: m_anyHit(anyHit),
		m_closestHit(closestHit),
		m_intersection(intersection)
	{}
}