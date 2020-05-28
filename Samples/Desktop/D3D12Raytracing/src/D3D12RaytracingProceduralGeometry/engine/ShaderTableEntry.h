#pragma once

#include "Ray.h"
#include "Geometry.h"
#include "HitGroup.h"
#include "RootSignature.h"

namespace RtxEngine
{
	using namespace std;
	
	struct ShaderTableEntry
	{
		string rayId;
		string geometryId;
		wstring hitGroupId;
		string rootSignatureId;
	};

	using ShaderTableEntries = vector<ShaderTableEntry>;
	using ShaderTableEntriesPtr = shared_ptr<ShaderTableEntries>;
}