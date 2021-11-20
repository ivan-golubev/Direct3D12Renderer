module;
#include <dxgi1_6.h>
export module D3DHelpers;

namespace awesome::d3dhelpers {

	/* Returns the first available hardware Direct3D 12 adapter
	   or nullptr if none can be found. */
	export void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter = false);

} // namespace awesome::d3dhelpers