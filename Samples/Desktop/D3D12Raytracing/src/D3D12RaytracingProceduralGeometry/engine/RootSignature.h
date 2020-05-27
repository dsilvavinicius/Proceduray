#pragma once

#include "../util/HlslCompat.h"
#include "../stdafx.h"
#include "ShaderCompatUtils.h"
#include "DeviceHelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <d3dx12.h>

namespace RtxEngine
{
	using namespace std;

	/** Root Signature. Add components using the add* methods and build() it. The resulting device root signature will have entries in the same order they are added.*/
	class RootSignature
	{
		enum SimpleEntry
		{
			Constant,
			SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
		};

		using DescriptorRange = pair<CD3DX12_DESCRIPTOR_RANGE, ShaderStruct>;

		RootSignature(const DeviceResourcesPtr& device);

		template<typename T>
		DescriptorRange createRange(SimpleEntry type, UINT baseReg, UINT numRegs, UINT space) const;

		/** Adds a simple entry (root constants or descriptors).*/
		template<typename T>
		void addEntry(SimpleEntry type, UINT reg, UINT space = 0u);
		
		/** Adds a complex entry (descriptor table (one more indirection)).*/
		void addDescriptorTable(const vector<DescriptorRange>& ranges);

		/** Adds a static sampler to the heap.*/
		void addStaticSampler();

		// Build the root signature.
		void build();
	private:
		void createResources();
		// REMEMBER: ONE HEAP FOR SAMPLERS AND ANOTHER FOR THE REST!
		void createHeap();
		void createLowLvl();
		void bind();

		DeviceResourcesPtr m_device;
		vector<CD3DX12_ROOT_PARAMETER> m_params;
		ComPtr<ID3D12RootSignature> m_buildedRoot;
	};

	using RootSignaturePtr = shared_ptr<RootSignature>;
	using RootSignatureMap = unordered_map<string, RootSignaturePtr>;
}