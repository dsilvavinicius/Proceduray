#include "RayTracingState.h"
#include "ShaderCompatUtils.h"
#include "CompiledShaders\Raytracing.hlsl.h"
#include "../DirectXRaytracingHelper.h"

namespace RtxEngine
{
	RayTracingState::RayTracingState(const StaticScenePtr& scene, const ShaderTableEntriesPtr& shaderTableEntries, const DxrDevicePtr& dxrDevice)
		: m_scene(scene),
		m_shaderTableEntries(shaderTableEntries),
		m_dxrDevice(dxrDevice)
	{
		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		// DXIL library
		createDxilLibrarySubobject(&raytracingPipeline);

		// Hit groups
		createHitGroupSubobjects(&raytracingPipeline);

		// Shader config
		// Defines the maximum sizes in bytes for the ray rayPayload and attribute structure.
		auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(ShaderCompatUtils::getMaxPayloadSize(), ShaderCompatUtils::getMaxAttribStructSize());

		// Local root signature and shader association
		// This is a root signature that enables a shader to have unique arguments that come from shader tables.
		createLocalRootSignatureSubobjects(&raytracingPipeline);

		// Global root signature
		// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
		auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(m_scene->getGlobalSignature().getBuilded().Get());

		// Pipeline config
		// Defines the maximum TraceRay() recursion depth.
		auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		// PERFOMANCE TIP: Set max recursion depth as low as needed
		// as drivers may apply optimization strategies for low recursion depths.
		UINT maxRecursionDepth = MAX_RAY_RECURSION_DEPTH;
		pipelineConfig->Config(maxRecursionDepth);

		PrintStateObjectDesc(raytracingPipeline);

		// Create the state object.
		ThrowIfFailed(m_dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrState)), L"Couldn't create DirectX Raytracing state object.\n");
	}

	// DXIL library
	// This contains the shaders and their entrypoints for the state object.
	// Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
	void RayTracingState::createDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		auto lib = raytracingPipeline->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
		lib->SetDXILLibrary(&libdxil);
		// Use default shader exports for a DXIL library/collection subobject ~ surface all shaders.
	}

	// Hit groups
	// A hit group specifies closest hit, any hit and intersection shaders 
	// to be executed when a ray intersects the geometry.
	void RayTracingState::createHitGroupSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		auto hitGroups = m_scene->getHitGroups();

		for (auto hitGroupEntry : hitGroups)
		{
			auto name = hitGroupEntry.first;
			auto hitGroup = hitGroupEntry.second;
			auto hitGroupSO = raytracingPipeline->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			if (!hitGroup->m_intersection.empty())
			{
				hitGroupSO->SetIntersectionShaderImport(hitGroup->m_intersection.c_str());
				hitGroupSO->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
			}
			
			if (!hitGroup->m_anyHit.empty())
			{
				hitGroupSO->SetAnyHitShaderImport(hitGroup->m_anyHit.c_str());
			}


			if (!hitGroup->m_closestHit.empty())
			{
				hitGroupSO->SetClosestHitShaderImport(hitGroup->m_closestHit.c_str());
			}

			hitGroupSO->SetHitGroupExport(name.c_str());
		}
	}

	// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
	void RayTracingState::createLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
	{
		unordered_map<string, vector<LPCWSTR>> rootToHit;
		for(const auto& entry : *m_shaderTableEntries)
		{
			rootToHit[entry.rootSignatureId].push_back(entry.hitGroupId.c_str());
		}

		for (const auto& rootToHitEntry : rootToHit)
		{
			auto rootSignatureId = rootToHitEntry.first;
			auto hitGroupIds = rootToHitEntry.second;

			const auto& rootSignature = m_scene->getLocalSignatures().at(rootSignatureId);
			auto rootSignatureSO = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			rootSignatureSO->SetRootSignature(rootSignature->getBuilded().Get());

			// Shader association
			auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*rootSignatureSO);
			for (const auto& hitGroupId : hitGroupIds)
			{
				rootSignatureAssociation->AddExports(hitGroupIds.data(), hitGroupIds.size());
			}
		}
	}
}