module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
export module D3D12Renderer;

using Microsoft::WRL::ComPtr;

namespace awesome::renderer {

	export class D3D12Renderer {
	public:
		D3D12Renderer(uint32_t width, uint32_t height, HWND windowHandle);
		void SetWindowsResized(bool value);
		void Render(uint64_t deltaTimeMs);
	private:
		static int8_t const FrameCount{ 2 };
		uint32_t const mWidth;
		uint32_t const mHeight;
		HWND const mWindowHandle;
		bool mWindowResized{ false };

		ComPtr<ID3D12Device> mDevice;
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<IDXGISwapChain3> mSwapChain;
	};

} // namespace awesome::renderer
