#include "Payloads.h"
#include "AttribStructs.h"
#include "ShaderCompat.h"
#include "../util/HlslCompat.h"
#include <unordered_map>
#include <algorithm>
#include <variant>

namespace RtxEngine
{
	using namespace std;

	using Payload = variant<RadiancePayload, ShadowPayload>;
	using PayloadMap = unordered_map<string, Payload>;
	using ShaderStruct = variant<Placeholder0, Placeholder1>;
	using ShaderStructMap = unordered_map<string, ShaderStruct>;
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

		static bool checkStructName(const string& name)
		{
			return (m_payloads.find(name) != m_payloads.end()) || (m_attribStructs.find(name) != m_attribStructs.end())
				|| (m_shaderStructs.find(name) != m_shaderStructs.end());
		}
	private:
		friend class Constructor;
		struct Constructor
		{
			Constructor()
			{
				m_payloads["IlluminationPayload"] = RadiancePayload();
				m_payloads["ShadowPayload"] = ShadowPayload();

				m_attribStructs["AttribStruct0"] = AttribStruct0();

				m_shaderStructs["Placeholder0"] = Placeholder0();
				m_shaderStructs["Placeholder1"] = Placeholder1();

				m_maxPayloadSize = 0u;
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(RadiancePayload)));
				m_maxPayloadSize = max(m_maxPayloadSize, UINT(sizeof(ShadowPayload)));

				m_maxAttribStructSize = 0u;
				m_maxAttribStructSize = max(m_maxAttribStructSize, UINT(sizeof(AttribStruct0)));
			}
		};

		static Constructor ctor;
		static PayloadMap m_payloads;
		static AttribStructMap m_attribStructs;
		static ShaderStructMap m_shaderStructs;
		static UINT m_maxPayloadSize;
		static UINT m_maxAttribStructSize;
	};
}