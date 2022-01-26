module;
#include <windows.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <format>
#include <wrl.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <pix3.h>
module D3D12Renderer;

import ErrorHandling;
import GlobalSettings;
import D3DHelpers;
import Vertex;

using Microsoft::WRL::ComPtr;
using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;
using awesome::errorhandling::ThrowIfFailed;
using awesome::globals::IsDebug;
using awesome::d3dhelpers::GetHardwareAdapter;
using awesome::d3dhelpers::SetName;
using awesome::d3dhelpers::GetName;
using awesome::structs::Vertex;

namespace awesome::renderer {

    D3D12Renderer::D3D12Renderer(uint32_t width, uint32_t height, HWND windowHandle)
        : mWidth{ width }
        , mHeight{ height }
        , mWindowHandle{ windowHandle }
        , mScissorRect{ D3D12_DEFAULT_SCISSOR_STARTX, D3D12_DEFAULT_SCISSOR_STARTY, D3D12_VIEWPORT_BOUNDS_MAX, D3D12_VIEWPORT_BOUNDS_MAX }
        , mViewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height) }
    {
        uint8_t dxgiFactoryFlags{ 0 };

        if constexpr (IsDebug())
        { /* Enable the debug layer */
            ComPtr<ID3D12Debug1> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                debugController->SetEnableGPUBasedValidation(true);
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
            SetName(mDevice.Get(), L"DefaultDevice");
        }

        { /* Create a command queue */
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
            SetName(mCommandQueue.Get(), L"MainCommandQueue");
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
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), n, mRtvDescriptorSize };
                mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, rtvHandle);
                SetName(mRenderTargets[n].Get(), std::format(L"SwapChainBuffer[{}]", n));
            }
        }
        /* Create a command allocator and a command list */
        ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
        SetName(mCommandAllocator.Get(), L"DefaultAllocator");
        ThrowIfFailed(mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandList)));
        SetName(mCommandList.Get(), L"DefaultCommandList");
        auto name = GetName(mCommandList.Get());

        { /* Create synchronization objects */
            ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
            SetName(mFence.Get(), L"DefaultFence");
            mFenceValue = 1;

            // Create an event handle to use for frame synchronization.
            mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (mFenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
#if 0
        /* Initialize the vertices. TODO: move to a separate class */
        mVertices.insert(mVertices.end(), {
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, // 0
            { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, // 1
            { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, // 2
            { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, // 3
            { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, // 4
            { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, // 5
            { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, // 6
            { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  // 7
        });
        mIndices = {
            0, 1, 2, 0, 2, 3,
            4, 6, 5, 4, 7, 6,
            4, 5, 1, 4, 1, 0,
            3, 2, 6, 3, 6, 7,
            1, 5, 6, 1, 6, 2,
            4, 0, 3, 4, 3, 7
        };
        
        /* create an intermediate resource */
        size_t bufferSize{ mVertices.size() * sizeof(Vertex) };
        CD3DX12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
        CD3DX12_RESOURCE_DESC uploadResourceProps{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize) };
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadResourceProps,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mVBIntermediateResource))
        );
        /* create the target resource on the GPU */
        CD3DX12_HEAP_PROPERTIES defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };
        CD3DX12_RESOURCE_DESC gpuResourceProps{ CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE) };
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &gpuResourceProps,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&mVertexBuffer))
        );
        /* transfer the data */
        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = mVertices.data();
        subresourceData.RowPitch = subresourceData.SlicePitch = bufferSize;

        // TODO: in which place this upload should happen ?
        ThrowIfFailed(mCommandAllocator->Reset());
        ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));
        UpdateSubresources(
            mCommandList.Get(),
            mVertexBuffer.Get(),
            mVBIntermediateResource.Get(),
            0, 0, 1, 
            &subresourceData
        );
        ThrowIfFailed(mCommandList->Close());
#endif
    }

    D3D12Renderer::~D3D12Renderer()
    {
        /* Ensure that the GPU is no longer referencing resources that are about to be
         cleaned up by the destructor. */
        WaitForPreviousFrame();
        CloseHandle(mFenceEvent);
    }

    void D3D12Renderer::SetWindowsResized(bool value)
    {
        mWindowResized = value;
    }

    void D3D12Renderer::Render(uint64_t deltaTimeMs)
    {
        /* Record all the commands we need to render the scene into the command list. */
        PopulateCommandList();

        /* Execute the command list. */
        ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        /* Present the frame and inefficiently wait for the frame to render. */
        ThrowIfFailed(mSwapChain->Present(1, 0));
        WaitForPreviousFrame();
    }

    void D3D12Renderer::WaitForPreviousFrame()
    {
        uint64_t const fence = mFenceValue;
        ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fence));
        mFenceValue++;

        /* Wait until the previous frame is finished */
        if (mFence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
            WaitForSingleObject(mFenceEvent, INFINITE);
        }
    }

    void D3D12Renderer::PopulateCommandList()
    {
        ThrowIfFailed(mCommandAllocator->Reset());
        ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));
        {
            uint8_t const frameIndex = mSwapChain->GetCurrentBackBufferIndex();
            PIXScopedEvent(mCommandList.Get(), PIX_COLOR(0,0,255), L"RenderFrame");

            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = mRenderTargets[frameIndex].Get();
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            /* Back buffer to be used as a Render Target */
            mCommandList->ResourceBarrier(1, &barrier);

            PIXSetMarker(mCommandList.Get(), PIX_COLOR_DEFAULT, L"SampleMarker");

            { /* Record commands */
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, mRtvDescriptorSize };
                const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
                mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
            }
            { /* Indicate that the back buffer will now be used to present. */
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                mCommandList->ResourceBarrier(1, &barrier);
            }
        }
        ThrowIfFailed(mCommandList->Close());
    }

} // namespace awesome::renderer
