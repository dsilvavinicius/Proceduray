#pragma once

#include "ShaderCompat.h"
#include "../util/HlslCompat.h"
#include "../DirectXRaytracingHelper.h"
#include <unordered_map>
#include <algorithm>
#include <variant>

namespace RtxEngine
{
	using namespace std;

	// Ray payloads
	using Payload = variant<RadiancePayload, ShadowPayload>;
	using PayloadMap = unordered_map<string, Payload>;
	
	// Root components
	using RootComponent = variant<RootComponent0, RootComponent1>;
	using RootComponentMap = unordered_map<string, RootComponent>;
	
	// Root arguments
	using RootArguments = variant<RootArguments0>;
	using RootArgumentsMap = unordered_map<string, RootArguments>;
	
	// Attrib structs
	using AttribStruct = variant<AttribStruct0>;
	using AttribStructMap = unordered_map<string, AttribStruct>;

	class ShaderCompatUtils
	{
	public:
		static UINT getMaxPayloadSize()
		{
			return m_maxPayloadSize;
		}

		static UINT getMaxAttribStructSize()
		{
			return m_maxAttribStructSize;
		}

		static UINT getMaxRootArgumentSize()
		{
			return m_maxRootArgumentSize;
		}

		static UINT getSize(const RootComponent& rootComponent)
		{
			if (holds_alternative<RootComponent0>(rootComponent))
			{
				return SizeOfInUint32(RootComponent0);
			}
			else
			{
				return SizeOfInUint32(RootComponent1);
			}
		}

		//static const RootArgumentsMap& getRootArgsMap() { return m_rootArguments; }

		static void* getRootArguments(RootArguments& rootArguments)
		{
			if (auto rootArgsPtr = std::get_if<RootArguments0>(&rootArguments))
			{
				return rootArgsPtr;
			}
			else
			{
				throw invalid_argument("Unexpected root argument type. Check the types in ShaderCompatUtils.h.");
			}
		}

		/*static bool checkStruct(const string& structName)
		{
			return (m_payloads.find(structName) != m_payloads.end())
				|| (m_attribStructs.find(structName) != m_attribStructs.end())
				|| (m_rootComponents.find(structName) != m_rootComponents.end())
				|| (m_rootArguments.find(structName) != m_rootArguments.end());
		}*/
	private:
		friend class Constructor;
		struct Constructor
		{
			Constructor()
			{
				m_payloads["RadiancePayload"] = RadiancePayload();
				m_payloads["ShadowPayload"] = ShadowPayload();

				m_attribStructs["AttribStruct0"] = AttribStruct0();

				m_rootComponents["RootComponent0"] = RootComponent0();
				m_rootComponents["RootComponent1"] = RootComponent1();

				m_rootArguments["RootArguments0"] = RootArguments0();

				m_maxPayloadSize = 0u;
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(RadiancePayload)));
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(ShadowPayload)));

				m_maxAttribStructSize = 0u;
				m_maxAttribStructSize = max(m_maxAttribStructSize, UINT(sizeof(AttribStruct0)));

				m_maxRootArgumentSize = 0u;
				m_maxRootArgumentSize = max(m_maxRootArgumentSize, UINT(sizeof(RootArguments0)));
			}
		};

		static Constructor m_ctor;
		static PayloadMap m_payloads;
		static AttribStructMap m_attribStructs;
		static RootComponentMap m_rootComponents;
		static RootArgumentsMap m_rootArguments;
		
		static UINT m_maxPayloadSize;
		static UINT m_maxAttribStructSize;
		static UINT m_maxRootArgumentSize;
	};
}