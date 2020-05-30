#pragma once

#include "../util/HlslCompat.h"
#include "ShaderCompatUtils.h"
#include "DeviceHelper.h"
#include <wrl/client.h>
#include <d3dx12.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace RtxEngine
{
	using namespace std;

	/** Root Signature. Add components using the add* methods and build() it. The resulting device root signature will have entries in the same order they are added.
	* The RootSignature must be compatible with the RootArg structure.
	*/
	class RootSignature
	{
	public:
		enum SimpleEntry
		{
			Constant = 1000,
			SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
		};

		using DescriptorRange = pair<CD3DX12_DESCRIPTOR_RANGE, RootComponent>;

		RootSignature(const DeviceResourcesPtr& device);

		DescriptorRange createRange(RootComponent, SimpleEntry type, UINT baseReg, UINT numRegs, UINT space) const;

		/** Adds a simple entry (root constants or descriptors).*/
		void addEntry(RootComponent, SimpleEntry type, UINT reg, UINT space = 0u);
		
		/** Adds a complex entry (descriptor table (one more indirection)).*/
		void addDescriptorTable(const vector<DescriptorRange>& ranges);

		/** Adds a static sampler to the heap.*/
		void addStaticSampler();

		void setRootArguments(const RootArguments& rootArgument) { m_rootArgument = rootArgument; }

		void* getRootArguments() { return ShaderCompatUtils::getRootArguments(m_rootArgument); }

		ComPtr<ID3D12RootSignature>& getBuilded();
	private:
		void createResources();
		// REMEMBER: ONE HEAP FOR SAMPLERS AND ANOTHER FOR THE REST!
		void createHeap();
		void createLowLvl();
		void bind();

		DeviceResourcesPtr m_device;
		vector<CD3DX12_ROOT_PARAMETER> m_params;
		RootArguments m_rootArgument;
		ComPtr<ID3D12RootSignature> m_builded;
	};

	using RootSignaturePtr = shared_ptr<RootSignature>;
	using RootSignatureMap = unordered_map<string, RootSignaturePtr>;
}