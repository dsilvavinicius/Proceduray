#include "Payloads.h"
#include "ShaderCompat.h"
#include "../util/HlslCompat.h"
#include <unordered_map>
#include <algorithm>
#include <variant>

namespace RtxEngine
{
	using namespace std;

	using Payload = variant<IlluminationPayload, ShadowPayload>;
	using PayloadMap = unordered_map<string, Payload>;
	using ShaderStruct = variant<Placeholder0, Placeholder1>;
	using ShaderStructMap = unordered_map<string, ShaderStruct>;

	class ShaderCompatUtils
	{
	public:
		static uint getMaxPayloadSize()
		{
			return m_maxPayloadSize;
		}

		static bool checkStructName(const string& name)
		{
			return (m_payloads.find(name) != m_payloads.end()) || (m_shaderStructs.find(name) != m_payloads.end());
		}
	private:
		friend class Constructor;
		struct Constructor
		{
			Constructor()
			{
				m_payloads["IlluminationPayload"] = IlluminationPayload;
				m_payloads["ShadowPayload"] = ShadowPayload;

				m_shaderStructs["Placeholder0"] = Placeholder0;
				m_shaderStructs["Placeholder1"] = Placeholder1;

				m_maxPayloadSize = 0u;
				m_maxPayloadSize = max(m_maxPayloadSize, sizeof(IlluminationPayload));
				m_maxPayloadSize = max(m_maxPayloadSize, sizeof(ShadowPayload));
			}
		};

		static Constructor ctor;
		static PayloadMap m_payloads;
		static ShaderStructMap m_shaderStructs;
		static uint m_maxPayloadSize;
	};
}