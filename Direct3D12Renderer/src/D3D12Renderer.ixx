module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <vector>
#include <string>
#include <DirectXMath.h>
export module D3D12Renderer;

import Vertex;

using Microsoft::WRL::ComPtr;
using DirectX::XMMATRIX;
using awesome::structs::Vertex;

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
		void UploadGeometry();
		void CreateBuffer(
			ComPtr<ID3D12GraphicsCommandList>& commandList, 
			ComPtr<ID3D12Resource>& gpuResource, 
			ComPtr<ID3D12Resource>& cpuResource, 
			void* data, 
			uint64_t sizeBytes,
			std::wstring resourceName
		);

		static int8_t const FrameCount{ 2 };
		uint32_t const mWidth;
		uint32_t const mHeight;
		HWND const mWindowHandle;
		bool mWindowResized{ false };

		D3D12_VIEWPORT mViewport;
		D3D12_RECT mScissorRect;

		ComPtr<ID3D12Device4> mDevice;
		ComPtr<ID3D12CommandQueue> mCommandQueue;
		ComPtr<ID3D12CommandAllocator> mCommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> mCommandList;
		ComPtr<IDXGISwapChain3> mSwapChain;
		ComPtr<ID3D12PipelineState> mPipelineState;
		ComPtr<ID3D12RootSignature> mRootSignature;

		/* Render Targets */
		ComPtr<ID3D12Resource> mRenderTargets[FrameCount];
		ComPtr<ID3D12DescriptorHeap> mRenderTargetViewHeap;
		uint32_t mRtvDescriptorSize;

		/* Depth */
		ComPtr<ID3D12Resource> mDepthBuffer;
		ComPtr<ID3D12DescriptorHeap> mDepthStencilHeap;
		uint32_t mDsvDescriptorSize;

		/* Vertex and Index Buffers for the cube. TODO: There is a better place for them. */
		ComPtr<ID3D12Resource> mVB_GPU_Resource;
		ComPtr<ID3D12Resource> mVB_CPU_Resource;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

		ComPtr<ID3D12Resource> mIB_GPU_Resource;
		ComPtr<ID3D12Resource> mIB_CPU_Resource;
		D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

		/* Shaders */
		ComPtr<ID3DBlob> mVertexShaderBlob;
		ComPtr<ID3DBlob> mPixelShaderBlob;

		/* TODO: move this to a "game_object" class */
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;
		XMMATRIX mModelMatrix;
		XMMATRIX mViewMatrix;
		XMMATRIX mProjectionMatrix;

		/* Synchronization objects */
		ComPtr<ID3D12Fence> mFence;
		uint32_t mFrameIndex{0};
		uint64_t mFenceValue{0};
		HANDLE mFenceEvent{nullptr};
	};

} // namespace awesome::renderer
