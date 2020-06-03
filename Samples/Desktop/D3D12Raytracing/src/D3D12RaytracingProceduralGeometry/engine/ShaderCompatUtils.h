#pragma once

#include "ShaderCompat.h"
#include "../DirectXRaytracingHelper.h"
#include <unordered_map>
#include <algorithm>
#include <variant>

namespace RtxEngine
{
	using namespace std;

	// Ray payloads
	using Payload = variant<RayPayload, ShadowRayPayload>;
	using PayloadMap = unordered_map<string, Payload>;
	
	// Root components
	using RootComponent = variant<PrimitiveConstantBuffer>;
	using RootComponentMap = unordered_map<string, RootComponent>;
	
	// Root arguments
	using RootArguments = variant<RootArguments0>;
	using RootArgumentsMap = unordered_map<string, RootArguments>;
	
	// Attrib structs
	using AttribStruct = variant<ProceduralPrimitiveAttributes>;
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
			if (holds_alternative<PrimitiveConstantBuffer>(rootComponent))
			{
				return SizeOfInUint32(PrimitiveConstantBuffer);
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
				m_payloads["RayPayload"] = RayPayload();
				m_payloads["ShadowRayPayload"] = ShadowRayPayload();

				m_attribStructs["ProceduralPrimitiveAttributes"] = ProceduralPrimitiveAttributes();

				m_rootComponents["PrimitiveConstantBuffer"] = PrimitiveConstantBuffer();

				m_rootArguments["RootArguments0"] = RootArguments0();

				m_maxPayloadSize = 0u;
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(RayPayload)));
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(ShadowRayPayload)));

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