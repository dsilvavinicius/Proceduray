#pragma once

#include "Entity.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;

	class HitGroup : public Entity
	{
	public:
		HitGroup(const string& name, const wstring& internalName, const wstring& anyHit, const wstring& closestHit = L"",
			const wstring& intersection = L"");
		wstring wName;
		wstring anyHit;
		wstring closestHit;
		wstring intersection;
	};

	using HitGroupPtr = shared_ptr<HitGroup>;
	using HitGroupMap = unordered_map<string, HitGroupPtr>;
	using HitGroupVector = vector<HitGroupPtr>;
}