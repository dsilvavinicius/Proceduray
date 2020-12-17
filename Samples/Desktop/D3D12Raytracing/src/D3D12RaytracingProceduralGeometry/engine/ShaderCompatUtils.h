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
	struct DontApply {};
	using RootComponent = variant<PrimitiveConstantBuffer, PrimitiveInstanceConstantBuffer, SceneConstantBuffer, InstanceBuffer, DontApply>;
	using RootComponentMap = unordered_map<string, RootComponent>;
	
	// Root arguments
	using RootArguments = variant<TriangleRootArguments, ProceduralRootArguments>;
	using RootArgumentsMap = unordered_map<string, RootArguments>;
	
	// Attrib structs
	using AttribStruct = variant<ProceduralPrimitiveAttributes>;
	using AttribStructMap = unordered_map<string, AttribStruct>;

	class ShaderCompatUtils
	{
	public:
		friend class Constructor;
		struct Constructor
		{
			Constructor()
			{
				// Payloads
				m_payloads["RayPayload"] = RayPayload();
				m_payloads["ShadowRayPayload"] = ShadowRayPayload();

				// Attribute structures
				m_attribStructs["ProceduralPrimitiveAttributes"] = ProceduralPrimitiveAttributes();

				// Root Components
				m_rootComponents["PrimitiveConstantBuffer"] = PrimitiveConstantBuffer();
				m_rootComponents["PrimitiveInstanceConstantBuffer"] = PrimitiveInstanceConstantBuffer();
				m_rootComponents["SceneConstantBuffer"] = SceneConstantBuffer();
				m_rootComponents["PrimitiveInstancePerFrameBuffer"] = InstanceBuffer();
				m_rootComponents["DontApply"] = DontApply();

				// Root Arguments
				m_rootArguments["TriangleRootArguments"] = TriangleRootArguments();
				m_rootArguments["ProceduralRootArguments"] = ProceduralRootArguments();

				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(RayPayload)));
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(ShadowRayPayload)));

				m_maxAttribStructSize = max(m_maxAttribStructSize, UINT(sizeof(ProceduralPrimitiveAttributes)));

				m_maxRootArgumentSize = max(m_maxRootArgumentSize, UINT(sizeof(TriangleRootArguments)));
				m_maxRootArgumentSize = max(m_maxRootArgumentSize, UINT(sizeof(ProceduralRootArguments)));
			}
		};

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
			else if (holds_alternative<PrimitiveInstanceConstantBuffer>(rootComponent))
			{
				return SizeOfInUint32(PrimitiveInstanceConstantBuffer);
			}
			else if (holds_alternative<SceneConstantBuffer>(rootComponent))
			{
				return SizeOfInUint32(SceneConstantBuffer);
			}
			else if (holds_alternative<InstanceBuffer>(rootComponent))
			{
				return SizeOfInUint32(InstanceBuffer);
			}
			else
			{
				throw(invalid_argument("You should not be asking the size of a DontApply RootComponent."));
			}
		}

		//static const RootArgumentsMap& getRootArgsMap() { return m_rootArguments; }

		static pair<void*, size_t> getRootArguments(RootArguments& rootArguments)
		{
			if (auto rootArgsPtr = std::get_if<TriangleRootArguments>(&rootArguments))
			{
				return pair<void*, size_t>(rootArgsPtr, sizeof(TriangleRootArguments));
			}
			else if (auto rootArgsPtr = std::get_if<ProceduralRootArguments>(&rootArguments))
			{
				return pair<void*, size_t>(rootArgsPtr, sizeof(ProceduralRootArguments));
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