module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
export module D3DHelpers;

export namespace awesome::d3dhelpers {

	/* Returns the first available hardware Direct3D 12 adapter
	   or nullptr if none can be found. */
	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

	void SetName(ID3D12Object*, std::wstring const&);
	std::wstring GetName(ID3D12Object*);

} // namespace awesome::d3dhelpers