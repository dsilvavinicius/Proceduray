#pragma once

#include <string>

namespace RtxEngine
{
	using namespace std;
	class Entity
	{
	public:
		Entity(const string& name) : m_name(name) {}
		virtual ~Entity() = 0 {}
		const string& getName() const { return m_name; }

	private:
			string m_name;
	};
}