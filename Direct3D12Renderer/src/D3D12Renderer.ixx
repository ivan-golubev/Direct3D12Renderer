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
		~D3D12Renderer();
		void SetWindowsResized(bool value);
		void Render(uint64_t deltaTimeMs);
	private:
		void PopulateCommandList();
		void WaitForPreviousFrame();

		static int8_t const FrameCount{ 2 };
		uint32_t const mWidth;
		uint32_t const mHeight;
		HWND const mWindowHandle;
		bool mWindowResized{ false };

		ComPtr<ID3D12Device4> mDevice;
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<ID3D12CommandAllocator> mCommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> mCommandList;
		ComPtr<IDXGISwapChain3> mSwapChain;
		ComPtr<ID3D12PipelineState> mPipelineState;
		ComPtr<ID3D12Resource> mRenderTargets[FrameCount];
		ComPtr<ID3D12DescriptorHeap> mRenderTargetViewHeap;
		uint32_t mRtvDescriptorSize;

		/* Synchronization objects */
		ComPtr<ID3D12Fence> mFence;
		uint32_t mFrameIndex{0};
		uint64_t mFenceValue{0};
		HANDLE mFenceEvent{nullptr};
	};

} // namespace awesome::renderer
