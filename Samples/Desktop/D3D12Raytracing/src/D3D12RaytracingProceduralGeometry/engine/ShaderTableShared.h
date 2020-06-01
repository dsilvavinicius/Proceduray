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
		string rootParametersId;
	};

	struct BuildedShaderTable
	{
		ComPtr<ID3D12Resource> missShaderTable;
		UINT missShaderTableStrideInBytes;
		ComPtr<ID3D12Resource> hitGroupShaderTable;
		UINT hitGroupShaderTableStrideInBytes;
		ComPtr<ID3D12Resource> rayGenShaderTable;
	};

	using ShaderTableEntries = vector<ShaderTableEntry>;
	using ShaderTableEntriesPtr = shared_ptr<ShaderTableEntries>;
	using BuildedShaderTablePtr = shared_ptr<BuildedShaderTable>;
}