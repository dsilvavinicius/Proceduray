#include "DeviceResources.h"
#include <memory>

namespace RtxEngine
{
	using namespace std;
	using namespace DX;

	using DxrDevicePtr = ComPtr<ID3D12Device5>;
	using DxrCommandListPtr = ComPtr<ID3D12GraphicsCommandList5>;
	using DxrStatePtr = ComPtr<ID3D12StateObject>;
	using DeviceResourcesPtr = shared_ptr<DeviceResources>;
}