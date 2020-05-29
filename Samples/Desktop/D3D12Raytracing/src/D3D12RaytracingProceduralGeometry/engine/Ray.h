#pragma once

#include "ShaderCompatUtils.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace RtxEngine
{
	using namespace std;

	class Ray
	{
	public:
		/**
		* @param missShader: miss shader entry point.
		*/
		Ray(const string& missShader, Payload) : m_missShader(missShader) {}
	
	private:
		string m_missShader;
	};

	using RayPtr = shared_ptr<Ray>;
	using RayMap = unordered_map<string, RayPtr>;
}
