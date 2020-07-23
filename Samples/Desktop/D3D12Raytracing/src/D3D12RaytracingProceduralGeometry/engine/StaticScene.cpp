#include "../stdafx.h"
#include "StaticScene.h"

namespace RtxEngine
{
	void StaticScene::addHitGroup(const string& name, const HitGroupPtr& hitGroup)
	{
		m_hitGroups.push_back(hitGroup);
		m_hitGroupMap[name] = hitGroup;
	}
}