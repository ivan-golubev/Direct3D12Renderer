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
