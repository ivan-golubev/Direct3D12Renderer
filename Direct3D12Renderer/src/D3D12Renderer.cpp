module;
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl.h>
module D3D12Renderer;

import ErrorHandling;
import GlobalSettings;
import D3DHelpers;

using Microsoft::WRL::ComPtr;
using awesome::errorhandling::ThrowIfFailed;
using awesome::globals::IsDebug;
using awesome::d3dhelpers::GetHardwareAdapter;

namespace awesome::renderer {

    D3D12Renderer::D3D12Renderer(uint32_t width, uint32_t height, HWND windowHandle)
        : mWidth{ width }, mHeight{ height }, mWindowHandle{ windowHandle }
    {
		uint8_t dxgiFactoryFlags{ 0 };
        
        if constexpr ( IsDebug() )
        { /* Enable the debug layer */
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
        
        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        { /* Select a hardware adapter and create a D3D12 device */
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);

            ThrowIfFailed(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&mDevice)
            ));
        }

        { /* Create a command queue */
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
        }

        { /* Create a swap chain */
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.BufferCount = FrameCount;
            swapChainDesc.Width = mWidth;
            swapChainDesc.Height = mHeight;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            ComPtr<IDXGISwapChain1> swapChain;
            ThrowIfFailed(factory->CreateSwapChainForHwnd(
                mCommandQueue.Get(),
                mWindowHandle,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain
            ));
            ThrowIfFailed(swapChain.As(&mSwapChain));
        }

        /* No support for fullscreen transitions. */
        ThrowIfFailed(factory->MakeWindowAssociation(mWindowHandle, DXGI_MWA_NO_ALT_ENTER));

        { /* Describe and create a render target view (RTV) descriptor heap. */
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRenderTargetViewHeap)));

            mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        
        { /* Create render targets */
            for (uint8_t n = 0; n < FrameCount; n++)
            {
                ThrowIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));

                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle
                { 
                    mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart().ptr + n * mRtvDescriptorSize
                };
                mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, rtvHandle);
            }
        }
        /* Create a command allocator and a command list */
        ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
        ThrowIfFailed(mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandList)));
        
        { /* Create synchronization objects */ 
            ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
            mFenceValue = 1;

            // Create an event handle to use for frame synchronization.
            mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (mFenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
	}

    void D3D12Renderer::SetWindowsResized(bool value) 
    { 
        mWindowResized = value; 
    }

    void D3D12Renderer::Render(uint64_t deltaTimeMs)
    {
    
    }

} // namespace awesome::renderer
