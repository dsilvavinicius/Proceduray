#include "../stdafx.h"
#include "StaticScene.h"

namespace RtxEngine
{
	void StaticScene::addGeometry(const string& name, const GeometryPtr& geometry)
	{
		m_geometry.push_back(geometry);
		m_geometryMap[name] = geometry;
	}

	void StaticScene::addHitGroup(const string& name, const HitGroupPtr& hitGroup)
	{
		m_hitGroups.push_back(hitGroup);
		m_hitGroupMap[name] = hitGroup;
	}
}