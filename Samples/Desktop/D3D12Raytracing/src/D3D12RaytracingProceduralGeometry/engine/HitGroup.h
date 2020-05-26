#pragma one

#include <memory>
#include <string>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;

	class HitGroup
	{
	public:
		HitGroup(const string& anyHit, const string& closestHit, const string& intersection);
	private:
		string m_anyHit;
		string m_closestHit;
		string m_intersection;
	};

	using HitGroupPtr = shared_ptr<HitGroup>;
	using HitGroupMap = unordered_map<string, HitGroupPtr>;
}