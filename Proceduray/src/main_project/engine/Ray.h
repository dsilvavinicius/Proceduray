#pragma once

#include "Entity.h"
#include "ShaderCompatUtils.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace RtxEngine
{
	using namespace std;

	class Ray : public Entity
	{
	public:
		/**
		* @param missShader: miss shader entry point.
		*/
		Ray(const string& name, const wstring& missShader, Payload) : Entity(name), missShader(missShader) {}
	
		wstring missShader;
	};

	using RayPtr = shared_ptr<Ray>;
	using RayMap = unordered_map<string, RayPtr>;
}
