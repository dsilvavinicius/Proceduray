#include "../stdafx.h"
#include "ShaderCompatUtils.h"

namespace RtxEngine
{
	UINT ShaderCompatUtils::m_maxPayloadSize = 0u;
	UINT ShaderCompatUtils::m_maxAttribStructSize = 0u;
	UINT ShaderCompatUtils::m_maxRootArgumentSize = 0u;

	PayloadMap ShaderCompatUtils::m_payloads = PayloadMap();
	AttribStructMap ShaderCompatUtils::m_attribStructs = AttribStructMap();
	RootComponentMap ShaderCompatUtils::m_rootComponents = RootComponentMap();
	RootArgumentsMap ShaderCompatUtils::m_rootArguments = RootArgumentsMap();

	ShaderCompatUtils::Constructor m_ctor = ShaderCompatUtils::Constructor();
}