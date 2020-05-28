#include "ShaderTable.h"

namespace RtxEngine
{
	ShaderTable::ShaderTable(const StaticScenePtr& scene, DeviceResourcesPtr& deviceResources)
		: m_scene(scene),
		m_deviceResources(deviceResources),
		m_commonEntries(make_shared<ShaderTableEntries>())
	{}

	void ShaderTable::addRayGen(const string& rayGenShader)
	{
		m_rayGenEntry = rayGenShader;
	}

	void ShaderTable::addMiss(const string& rayId)
	{
		m_missEntries.push_back(rayId);
	}

	void ShaderTable::addCommonEntry(const ShaderTableEntry& entry)
	{
		m_commonEntries->push_back(entry);
	}

	// TODO: CONTINUE HERE AFTER CREATING THE STATE OBJECT!
	void ShaderTable::build()
	{
		auto device = m_deviceResources->GetD3DDevice();

		void* rayGenShaderID;
		void* missShaderIDs[RayType::Count];
		void* hitGroupShaderIDs_TriangleGeometry[RayType::Count];
		void* hitGroupShaderIDs_AABBGeometry[IntersectionShaderType::Count][RayType::Count];

		// A shader name look-up table for shader table debug print out.
		unordered_map<void*, wstring> shaderIdToStringMap;

		auto GetShaderIDs = [&](auto* stateObjectProperties)
		{
			rayGenShaderID = stateObjectProperties->GetShaderIdentifier(c_raygenShaderName);
			shaderIdToStringMap[rayGenShaderID] = c_raygenShaderName;

			for (UINT i = 0; i < RayType::Count; i++)
			{
				missShaderIDs[i] = stateObjectProperties->GetShaderIdentifier(c_missShaderNames[i]);
				shaderIdToStringMap[missShaderIDs[i]] = c_missShaderNames[i];
			}
			for (UINT i = 0; i < RayType::Count; i++)
			{
				hitGroupShaderIDs_TriangleGeometry[i] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_TriangleGeometry[i]);
				shaderIdToStringMap[hitGroupShaderIDs_TriangleGeometry[i]] = c_hitGroupNames_TriangleGeometry[i];
			}
			for (UINT r = 0; r < IntersectionShaderType::Count; r++)
				for (UINT c = 0; c < RayType::Count; c++)
				{
					hitGroupShaderIDs_AABBGeometry[r][c] = stateObjectProperties->GetShaderIdentifier(c_hitGroupNames_AABBGeometry[r][c]);
					shaderIdToStringMap[hitGroupShaderIDs_AABBGeometry[r][c]] = c_hitGroupNames_AABBGeometry[r][c];
				}
		};

		// Get shader identifiers.
		UINT shaderIDSize;
		{
			ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
			ThrowIfFailed(m_dxrStateObject.As(&stateObjectProperties));
			GetShaderIDs(stateObjectProperties.Get());
			shaderIDSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}

		/*************--------- Shader table layout -------*******************
		| --------------------------------------------------------------------
		| Shader table - HitGroupShaderTable:
		| [0] : MyHitGroup_Triangle
		| [1] : MyHitGroup_Triangle_ShadowRay
		| [2] : MyHitGroup_AABB_AnalyticPrimitive
		| [3] : MyHitGroup_AABB_AnalyticPrimitive_ShadowRay
		| ...
		| [6] : MyHitGroup_AABB_VolumetricPrimitive
		| [7] : MyHitGroup_AABB_VolumetricPrimitive_ShadowRay
		| [8] : MyHitGroup_AABB_SignedDistancePrimitive
		| [9] : MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay,
		| ...
		| [20] : MyHitGroup_AABB_SignedDistancePrimitive
		| [21] : MyHitGroup_AABB_SignedDistancePrimitive_ShadowRay
		| --------------------------------------------------------------------
		**********************************************************************/

		// RayGen shader table.
		{
			UINT numShaderRecords = 1;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			ShaderTable rayGenShaderTable(device, numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
			rayGenShaderTable.push_back(ShaderRecord(rayGenShaderID, shaderRecordSize, nullptr, 0));
			rayGenShaderTable.DebugPrint(shaderIdToStringMap);
			m_rayGenShaderTable = rayGenShaderTable.GetResource();
		}

		// Miss shader table.
		{
			UINT numShaderRecords = RayType::Count;
			UINT shaderRecordSize = shaderIDSize; // No root arguments

			ShaderTable missShaderTable(device, numShaderRecords, shaderRecordSize, L"MissShaderTable");
			for (UINT i = 0; i < RayType::Count; i++)
			{
				missShaderTable.push_back(ShaderRecord(missShaderIDs[i], shaderIDSize, nullptr, 0));
			}
			missShaderTable.DebugPrint(shaderIdToStringMap);
			m_missShaderTableStrideInBytes = missShaderTable.GetShaderRecordSize();
			m_missShaderTable = missShaderTable.GetResource();
		}

		// Hit group shader table.
		{
			UINT numShaderRecords = RayType::Count + IntersectionShaderType::TotalPrimitiveCount * RayType::Count;
			UINT shaderRecordSize = shaderIDSize + LocalRootSignature::MaxRootArgumentsSize();
			ShaderTable hitGroupShaderTable(device, numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");

			// Triangle geometry hit groups.
			{
				LocalRootSignature::Triangle::RootArguments rootArgs;
				rootArgs.materialCb = m_planeMaterialCB;

				for (auto& hitGroupShaderID : hitGroupShaderIDs_TriangleGeometry)
				{
					hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
				}
			}

			// AABB geometry hit groups.
			{
				LocalRootSignature::AABB::RootArguments rootArgs;
				UINT instanceIndex = 0;

				// Create a shader record for each primitive.
				for (UINT iShader = 0, instanceIndex = 0; iShader < IntersectionShaderType::Count; iShader++)
				{
					UINT numPrimitiveTypes = IntersectionShaderType::PerPrimitiveTypeCount(static_cast<IntersectionShaderType::Enum>(iShader));

					// Primitives for each intersection shader.
					for (UINT primitiveIndex = 0; primitiveIndex < numPrimitiveTypes; primitiveIndex++, instanceIndex++)
					{
						rootArgs.materialCb = m_aabbMaterialCB[instanceIndex];
						rootArgs.aabbCB.instanceIndex = instanceIndex;
						rootArgs.aabbCB.primitiveType = primitiveIndex;

						// Ray types.
						for (UINT r = 0; r < RayType::Count; r++)
						{
							auto& hitGroupShaderID = hitGroupShaderIDs_AABBGeometry[iShader][r];
							hitGroupShaderTable.push_back(ShaderRecord(hitGroupShaderID, shaderIDSize, &rootArgs, sizeof(rootArgs)));
						}
					}
				}
			}
			hitGroupShaderTable.DebugPrint(shaderIdToStringMap);
			m_hitGroupShaderTableStrideInBytes = hitGroupShaderTable.GetShaderRecordSize();
			m_hitGroupShaderTable = hitGroupShaderTable.GetResource();
		}
	}
}