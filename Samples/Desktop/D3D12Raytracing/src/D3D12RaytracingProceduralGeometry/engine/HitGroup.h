#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;

	struct HitGroup
	{
	public:
		HitGroup(const wstring& name, const wstring& anyHit, const wstring& closestHit = L"", const wstring& intersection = L"");

		wstring name;
		wstring anyHit;
		wstring closestHit;
		wstring intersection;
	};

	using HitGroupPtr = shared_ptr<HitGroup>;
	using HitGroupMap = unordered_map<string, HitGroupPtr>;
}