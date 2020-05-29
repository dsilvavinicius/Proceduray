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
		HitGroup(const wstring& anyHit, const wstring& closestHit = L"", const wstring& intersection = L"");

		wstring m_anyHit;
		wstring m_closestHit;
		wstring m_intersection;
	};

	using HitGroupPtr = shared_ptr<HitGroup>;
	using HitGroupMap = unordered_map<wstring, HitGroupPtr>;
}