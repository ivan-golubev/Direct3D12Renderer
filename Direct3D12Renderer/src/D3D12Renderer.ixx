module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
export module D3D12Renderer;

using Microsoft::WRL::ComPtr;

namespace awesome::renderer {

	export class D3D12Renderer {
	public:
		void Init();
	private:
		ComPtr<ID3D12Device> mDevice;
		ComPtr<IDXGISwapChain3> mSwapChain;
	};

} // namespace awesome::renderer
