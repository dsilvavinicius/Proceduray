#pragma once

#include "DeviceHelper.h"
#include "StaticScene.h"

namespace RtxEngine
{
	class AccelerationStructure
	{
	public:
		AccelerationStructure(DevicePtr& device, const StaticScenePtr& scene);
	private:
		void build();

		DevicePtr m_device;
		StaticScenePtr m_scene;
	};

	using AccelerationStructurePtr = shared_ptr<AccelerationStructure>;
}