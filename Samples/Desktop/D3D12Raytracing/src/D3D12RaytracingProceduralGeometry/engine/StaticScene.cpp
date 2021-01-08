#include "../stdafx.h"
#include "StaticScene.h"

namespace RtxEngine
{
	void StaticScene::addGeometry(const GeometryPtr& geometry)
	{
		m_geometry.push_back(geometry);
		m_geometryMap[geometry->getName()] = geometry;
	}

	void StaticScene::addHitGroup(const HitGroupPtr& hitGroup)
	{
		m_hitGroups.push_back(hitGroup);
		m_hitGroupMap[hitGroup->getName()] = hitGroup;
	}
}