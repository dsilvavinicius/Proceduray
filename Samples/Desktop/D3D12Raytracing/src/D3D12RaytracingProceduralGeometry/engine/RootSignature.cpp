#include "../stdafx.h"
#include "RootSignature.h"

namespace RtxEngine
{
	RootSignature::RootSignature(const string& name, const DeviceResourcesPtr& deviceResources,
		const DescriptorHeapPtr& descriptorHeap, bool isLocal)
		: Entity(name),
		m_deviceResources(deviceResources),
		m_descriptorHeap(descriptorHeap),
		m_isLocal(isLocal),
		m_builded(nullptr)
	{}

	RootSignature::~RootSignature()
	{
		m_builded.Reset();
	}

	RootSignature::DescriptorRange RootSignature::createRange(D3D12_GPU_DESCRIPTOR_HANDLE baseHandleToHeap, BufferEntry type, UINT baseReg, UINT numRegs, UINT space) const
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE(type), numRegs, baseReg, space);
		
		DescriptorRange descRange = { range, baseHandleToHeap };
		return descRange;
	}

	void RootSignature::addConstant(const RootComponent& component, UINT reg, UINT space)
	{
		CD3DX12_ROOT_PARAMETER param;
		param.InitAsConstants(ShaderCompatUtils::getSize(component), reg, space);
		m_params.push_back(param);
	}

	void RootSignature::addEntry(RootComponent component, BufferEntry type, const shared_ptr<GpuUploadBuffer>& buffer, UINT reg, UINT space)
	{
		m_uploadBuffers[UINT(m_params.size())] = buffer;
		addEntry(type, reg, space);
	}


	void RootSignature::addEntry(RootComponent component, BufferEntry type, const ComPtr<ID3D12Resource>& resource, UINT reg, UINT space)
	{
		m_genericResources[UINT(m_params.size())] = resource;
		addEntry(type, reg, space);
	}

	void RootSignature::addEntry(BufferEntry type, UINT reg, UINT space)
	{
		CD3DX12_ROOT_PARAMETER param;
		switch (type)
		{
		case CBV: param.InitAsConstantBufferView(reg, space); break;
		case SRV: param.InitAsShaderResourceView(reg, space);  break;
		case UAV: param.InitAsUnorderedAccessView(reg, space);  break;
		}

		m_params.push_back(param);
	}

	UINT RootSignature::addDescriptorTable(const vector<DescriptorRange>& ranges)
	{
		UINT descriptorEntryIdx = UINT(m_params.size());
		CD3DX12_ROOT_PARAMETER param;
		m_ranges.push_back(vector<CD3DX12_DESCRIPTOR_RANGE>());
		auto& untyped_ranges = *m_ranges.rbegin();
		for (auto range : ranges)
		{
			untyped_ranges.push_back(range.range);
		}
		m_baseHandlesToHeap[descriptorEntryIdx] = ranges[0].baseHandleToHeap;
		param.InitAsDescriptorTable(UINT(untyped_ranges.size()), untyped_ranges.data());

		m_params.push_back(param);

		return descriptorEntryIdx;
	}

	void RootSignature::updateHeapHandle(UINT baseHandleIdx, D3D12_GPU_DESCRIPTOR_HANDLE& handle)
	{
		m_baseHandlesToHeap[baseHandleIdx] = handle;
	}

	ComPtr<ID3D12RootSignature>& RootSignature::getBuilded()
	{
		if (m_builded == nullptr)
		{
			build();
		}
		return m_builded;
	}

	void RootSignature::build()
	{
		CD3DX12_ROOT_SIGNATURE_DESC desc(UINT(m_params.size()), m_params.data());
		if (m_isLocal)
		{
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		}

		auto device = m_deviceResources->GetD3DDevice();
		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
		ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_builded)));
	}

	void RootSignature::prepareToRender()
	{
		auto commandList = m_deviceResources->GetCommandList();
		auto frameIndex = m_deviceResources->GetCurrentFrameIndex();
		
		commandList->SetComputeRootSignature(m_builded.Get());
		m_descriptorHeap->bind();

		for (int i = 0; i < m_params.size(); ++i)
		{
			// Descriptor table case: just associate root slot with handle.
			auto handleIter = m_baseHandlesToHeap.find(i);
			if (handleIter != m_baseHandlesToHeap.end())
			{
				commandList->SetComputeRootDescriptorTable(i, handleIter->second);
				continue;
			}
			
			// Upload buffer or Generic resource case: get gpu address.
			D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
			
			auto bufferIter = m_uploadBuffers.find(i);
			if (bufferIter != m_uploadBuffers.end())
			{
				// Upload buffer must upload to gpu, additionaly.
				auto buffer = bufferIter->second;
				buffer->CopyStagingToGpu(frameIndex);
				gpuAddress = buffer->GpuVirtualAddress(frameIndex);
			}
			else
			{
				auto genericResourceIter = m_genericResources.find(i);
				gpuAddress = genericResourceIter->second->GetGPUVirtualAddress();
			}

			// Use gpu address to associate root slot and resource.
			auto paramType = m_params[i].ParameterType;
			switch (paramType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_CBV: commandList->SetComputeRootConstantBufferView(i, gpuAddress); break;
			case D3D12_ROOT_PARAMETER_TYPE_SRV: commandList->SetComputeRootShaderResourceView(i, gpuAddress); break;
			case D3D12_ROOT_PARAMETER_TYPE_UAV: commandList->SetComputeRootUnorderedAccessView(i, gpuAddress); break;
			}
			
		}
	}
}
