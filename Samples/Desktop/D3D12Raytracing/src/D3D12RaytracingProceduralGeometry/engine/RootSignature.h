#pragma once

#include "ShaderCompatUtils.h"
#include "DescriptorHeap.h"
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
		enum BufferEntry
		{
			SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
			CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			Sampler = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
		};

		struct DescriptorRange
		{
			CD3DX12_DESCRIPTOR_RANGE range;
			D3D12_GPU_DESCRIPTOR_HANDLE baseHandleToHeap;
		};

		RootSignature(const DeviceResourcesPtr& deviceResources, const DescriptorHeapPtr& descriptorHeap, bool isLocal);

		~RootSignature();

		DescriptorRange createRange(D3D12_GPU_DESCRIPTOR_HANDLE baseHandleToHeap, BufferEntry type, UINT baseReg, UINT numRegs, UINT space = 0u) const;

		/** Adds a constant entry.*/
		void RootSignature::addConstant(const RootComponent& component, UINT reg, UINT space = 0u);

		/** Adds a descriptor entry which buffer must be uploaded every frame.*/
		void addEntry(RootComponent, BufferEntry type, const shared_ptr<GpuUploadBuffer>& buffer, UINT reg, UINT space = 0u);

		/** Adds a descriptor entry which does not need to be uploaded every frame. */
		void addEntry(RootComponent, BufferEntry type, const ComPtr<ID3D12Resource>& resource, UINT reg, UINT space = 0u);
		
		/** Adds a complex entry (descriptor table (one more indirection to the descriptor heap)).*/
		UINT addDescriptorTable(const vector<DescriptorRange>& ranges);

		void updateHeapHandle(UINT baseHandleIdx, D3D12_GPU_DESCRIPTOR_HANDLE& handle);

		/** Adds a static sampler to the heap.*/
		void addStaticSampler();

		void setRootArgumentsType(const RootArguments& rootArguments) { m_rootArguments = rootArguments; }

		bool isRootArgumentsTypeEqual(const RootArguments& other) const { return other.index() == m_rootArguments.index(); }

		ComPtr<ID3D12RootSignature>& getBuilded();

		void prepareToRender();
	private:
		void addEntry(BufferEntry type, UINT reg, UINT space);
		void build();

		// Input.
		DeviceResourcesPtr m_deviceResources;
		DescriptorHeapPtr m_descriptorHeap;
		bool m_isLocal;

		// Generated.
		vector<CD3DX12_ROOT_PARAMETER> m_params;
		unordered_map<UINT, D3D12_GPU_DESCRIPTOR_HANDLE> m_baseHandlesToHeap;
		unordered_map<UINT, shared_ptr<GpuUploadBuffer>> m_uploadBuffers;
		unordered_map<UINT, ComPtr<ID3D12Resource>> m_genericResources;
		RootArguments m_rootArguments;
		vector<vector<CD3DX12_DESCRIPTOR_RANGE>> m_ranges;

		// Output.
		ComPtr<ID3D12RootSignature> m_builded;
	};

	using RootSignaturePtr = shared_ptr<RootSignature>;
	using RootSignatureMap = unordered_map<string, RootSignaturePtr>;
}