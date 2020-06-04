#include "../stdafx.h"
#include "ShaderTable.h"
#include "../DirectXRaytracingHelper.h"

namespace RtxEngine
{
	ShaderTable::ShaderTable(const StaticScenePtr& scene, DeviceResourcesPtr& deviceResources)
		: m_scene(scene),
		m_deviceResources(deviceResources),
		m_commonEntries(make_shared<ShaderTableEntries>())
	{}

	void ShaderTable::addRayGen(const wstring& rayGenShader)
	{
		m_rayGenEntry = rayGenShader;
	}

	void ShaderTable::addMiss(const wstring& rayId)
	{
		m_missEntries.push_back(rayId);
	}

	void ShaderTable::addCommonEntry(const ShaderTableEntry& entry)
	{
		m_commonEntries->push_back(entry);
	}

	const BuildedShaderTablePtr& ShaderTable::getBuilded(RayTracingState& rayTracingState)
	{
		if (m_buildedShaderTable == nullptr)
		{
			build(rayTracingState);
		}

		return m_buildedShaderTable;
	}

	void ShaderTable::build(RayTracingState& rayTracingState)
	{
		auto device = m_deviceResources->GetD3DDevice();

		void* rayGenShaderID;
		vector<void*> missShaderIDs(m_missEntries.size());
		vector<void*> hitGroupShaderIDs(m_commonEntries->size());

		// A shader name look-up table for shader table debug print out.
		unordered_map<void*, wstring> shaderIdToStringMap;

		auto GetShaderIDs = [&](auto* stateObjectProperties)
		{
			rayGenShaderID = stateObjectProperties->GetShaderIdentifier(m_rayGenEntry.c_str());
			shaderIdToStringMap[rayGenShaderID] = m_rayGenEntry;

			for (UINT i = 0; i < m_missEntries.size(); i++)
			{
				missShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(m_missEntries[i].c_str());
				shaderIdToStringMap[missShaderIDs[i]] = m_missEntries[i];
			}
			for (UINT i = 0; i < m_commonEntries->size(); i++)
			{
				auto hitGroupName = m_commonEntries[i].hitGroupId;
				hitGroupShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(hitGroupName);
				shaderIdToStringMap[hitGroupShaderIDs[i]] = hitGroupName;
			}
		};

		// Get shader identifiers.
		UINT shaderIDSize;
		{
			ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
			ThrowIfFailed(rayTracingState.getBuilded().As(&stateObjectProperties));
			GetShaderIDs(stateObjectProperties.Get());
			shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}

		/*************--------- Shader table layout -------*******************
		| --------------------------------------------------------------------
		| Shader table - HitGroupShaderTable:
		| [0] : RayGen
		| [1] : Miss 0
		| ... : 
		| [n + 1] : Miss n
		| [n + 2] : Hitgroup 0
		| ... : 
		| [n + m + 1] : HitGroup m
		| --------------------------------------------------------------------
		**********************************************************************/

		// RayGen shader table.
		{
			UINT numShaderRecords = 1;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			::ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
			rayGenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderRecordSize, nullptr, 0));
			rayGenShaderTable.DebugPrint(shaderIdToStringMap);
			m_buildedShaderTable->rayGenShaderTable = rayGenShaderTable.GetResource();
		}

		// Miss shader table.
		{
			UINT numShaderRecords = missShaderIDs.size();
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			::ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
			for (UINT i = 0; i < missShaderIDs.size(); i++)
			{
				missShaderTable.push_back(ShaderRecord(missShaderIDs[i], shaderIDSize, nullptr, 0));
			}
			missShaderTable.DebugPrint(shaderIdToStringMap);
			m_buildedShaderTable->missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
			m_buildedShaderTable->missShaderTable = missShaderTable.GetResource();
		}

		// Hit group shader table.
		{
			UINT numShaderRecords = hitGroupShaderIDs.size();
			UINT shaderRecordSize = shaderIDSize + ShaderCompatUtils::getMaxRootArgumentSize();
			::ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

			for (int i = 0; i < hitGroupShaderIDs.size(); ++i)
			{
				auto& entry = (*m_commonEntries)[i];
				const auto& rootSignature = m_scene->getLocalSignatures().at(entry.rootSignatureId);
				ThrowIfFalse(rootSignature->isRootArgumentsTypeEqual(entry.rootArguments));
				void* rootArguments = ShaderCompatUtils::getRootArguments(entry.rootArguments);
				hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderIDs[i], shaderIDSize, rootArguments, sizeof(rootArguments)));
			}

			hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
			m_buildedShaderTable->hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
			m_buildedShaderTable->hitGroupShaderTable = hitGroupShaderTable.GetResource();
		}
	}
}