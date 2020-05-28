#include "RootSignature.h"

namespace RtxEngine
{
	RootSignature::RootSignature(const DeviceResourcesPtr& device)
		: m_device(device),
		m_builded(nullptr)
	{}

	template<typename T>
	RootSignature::DescriptorRange RootSignature::createRange(SimpleEntry type, UINT baseReg, UINT numRegs, UINT space) const
	{
		if (type == Constant)
		{
			throw invalid_argument("SimpleEntry::Constant is not allowed in descriptor ranges.");
		}

		CD3DX12_DESCRIPTOR_RANGE range;
		range.Init(SimpleEntry, numRegs, baseReg, space);
	}

	template<typename T>
	void RootSignature::addEntry(SimpleEntry type, UINT reg, UINT space)
	{
		CD3DX12_ROOT_PARAMETER param;
		switch (type)
		{
		case Constant: param.InitAsConstants(SizeOfInUint32(T), reg, space);
		case CBV: param.InitAsConstantBufferView(reg, space);
		case SRV: param.InitAsShaderResourceView(reg, space);
		case UAV: param.InitAsUnorderedAccessView(reg, space);
		}
		
		m_params.push_back(param);
	}

	void RootSignature::addDescriptorTable(const vector<DescriptorRange>& ranges)
	{
		CD3DX12_ROOT_PARAMETER param;
		vector<CD3DX12_DESCRIPTOR_RANGE> untyped_ranges;
		for (auto range : ranges)
		{
			untyped_ranges.push_back(range.first);
		}
		param.InitAsDescriptorTable(untyped_ranges.size(), untyped_ranges.data());

		m_params.push_back(param);
	}

	ComPtr<ID3D12RootSignature>& RootSignature::getBuilded()
	{
		if (m_builded == nullptr)
		{
			createLowLvl();
		}
		return m_builded;
	}


	void RootSignature::createLowLvl()
	{
		CD3DX12_ROOT_SIGNATURE_DESC desc(m_params.size(), m_params.data());
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		auto device = m_device->GetD3DDevice();
		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
		ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*m_builded))));
	}
}
