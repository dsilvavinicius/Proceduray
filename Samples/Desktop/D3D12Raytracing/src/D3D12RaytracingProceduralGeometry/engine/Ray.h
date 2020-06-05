#pragma once

#include "ShaderCompatUtils.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace RtxEngine
{
	using namespace std;

	struct Ray
	{
	public:
		/**
		* @param missShader: miss shader entry point.
		*/
		Ray(const wstring& missShader, Payload) : missShader(missShader) {}
	
		wstring missShader;
	};

	using RayPtr = shared_ptr<Ray>;
	using RayMap = unordered_map<string, RayPtr>;
}
