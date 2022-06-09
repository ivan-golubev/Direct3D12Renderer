module;
#include <algorithm>
#include <cstdint>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <format>
#include <pix3.h>
#include <vector>
#include <windows.h>
#include <wrl.h>
module D3D12Renderer;

import Application;
import Camera;
import D3DHelpers;
import ErrorHandling;
import GlobalSettings;
import Input;
import PipelineStateStream;
import Vertex;

using namespace DirectX;
using Microsoft::WRL::ComPtr;

namespace gg {

    D3D12Renderer::D3D12Renderer(uint32_t width, uint32_t height, HWND windowHandle)
        : mWidth{ width }
        , mHeight{ height }
        , mWindowHandle{ windowHandle }
        , mScissorRect{ D3D12_DEFAULT_SCISSOR_STARTX, D3D12_DEFAULT_SCISSOR_STARTY, D3D12_VIEWPORT_BOUNDS_MAX, D3D12_VIEWPORT_BOUNDS_MAX }
        , mViewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f }
        , mCamera{ std::make_unique<Camera>() }
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
            swapChainDesc.BufferCount = mFrameCount;
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
            rtvHeapDesc.NumDescriptors = mFrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRenderTargetViewHeap)));

            mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        /* Create render targets */
        ResizeRenderTargets();
        
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

        { /* Describe and create a depth stencil view (DSV) descriptor heap. */
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.NumDescriptors = 1;
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mDepthStencilHeap)));

            mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        /* Create depth buffer */
        ResizeDepthBuffer();

        /* Read shaders */
        {
            ThrowIfFailed(D3DReadFileToBlob(L"shaders//colored_surface_VS.cso", &mVertexShaderBlob));
            ThrowIfFailed(D3DReadFileToBlob(L"shaders//colored_surface_PS.cso", &mPixelShaderBlob));
            // TODO: can I set the shader name ?
        }
        UploadGeometry();

        /* Specify the input layout */
        D3D12_INPUT_ELEMENT_DESC const inputLayout[] {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        {
            /* Check for the highest supported root signature */
            // TODO: move the root signature to HLSL and precompile it
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
            {
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }
            /* Create the root signature */
            D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

            CD3DX12_ROOT_PARAMETER1 rootParameters[1];
            rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / sizeof(float), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
            rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

            /* Serialize the root signature */
            ComPtr<ID3DBlob> rootSignatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
                featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

            // Create the root signature.
            ThrowIfFailed(mDevice->CreateRootSignature(
                0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature))
            );
            SetName(mRootSignature.Get(), L"RootSignature");
        }

        { // PSO
            D3D12_RT_FORMAT_ARRAY rtvFormats{};
            rtvFormats.NumRenderTargets = 1;
            rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

            CD3DX12_BLEND_DESC blendDesc{ D3D12_DEFAULT };
            CD3DX12_RASTERIZER_DESC rasterizerDesc{ D3D12_DEFAULT };

            PipelineStateStream pipelineStateStream{};
            pipelineStateStream.pRootSignature = mRootSignature.Get();
            pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
            pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pipelineStateStream.BlendState = blendDesc;
            pipelineStateStream.RasterizerState = rasterizerDesc;
            pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(mVertexShaderBlob.Get());
            pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(mPixelShaderBlob.Get());
            pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            pipelineStateStream.RTVFormats = rtvFormats;

            D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc {
                sizeof(PipelineStateStream), &pipelineStateStream
            };
            ThrowIfFailed(mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState)));
            SetName(mPipelineState.Get(), L"DefaultPipelineState");
        }

    }

    void D3D12Renderer::UploadGeometry()
    {
        /* Initialize the vertices. TODO: move to a separate class */
        // TODO: in fact, cubes are not fun, read data from an .fbx
        std::vector<Vertex> const vertices{
            /*  x      y      z     w     r      g    b     a */
            {-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // 0
            {-1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f}, // 1
            { 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f}, // 2
            { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f}, // 3
            {-1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f}, // 4
            {-1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f}, // 5
            { 1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f}, // 6
            { 1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f}, // 7
        };
        std::vector<uint32_t> const indices{
            0, 1, 2, 0, 2, 3,
            4, 6, 5, 4, 7, 6,
            4, 5, 1, 4, 1, 0,
            3, 2, 6, 3, 6, 7,
            1, 5, 6, 1, 6, 2,
            4, 0, 3, 4, 3, 7
        };
        mIndexCount = static_cast<uint32_t>(indices.size());

        ThrowIfFailed(mCommandAllocator->Reset());
        ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));

        uint32_t const VB_sizeBytes = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
        uint32_t const IB_sizeBytes = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));

        CreateBuffer(mCommandList, mVB_GPU_Resource, mVB_CPU_Resource, vertices.data(), VB_sizeBytes, L"VertexBuffer");
        CreateBuffer(mCommandList, mIB_GPU_Resource, mIB_CPU_Resource, indices.data(), IB_sizeBytes, L"IndexBuffer");

        ThrowIfFailed(mCommandList->Close());

        /* Upload Vertex and Index buffers */
        ID3D12CommandList* ppCommandLists[] { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        WaitForPreviousFrame();

        /* Init the Vertex/Index buffer views */
        mVertexBufferView.BufferLocation = mVB_GPU_Resource->GetGPUVirtualAddress();
        mVertexBufferView.SizeInBytes = VB_sizeBytes;
        mVertexBufferView.StrideInBytes = sizeof(Vertex);
        
        mIndexBufferView.BufferLocation = mIB_GPU_Resource->GetGPUVirtualAddress();
        mIndexBufferView.SizeInBytes = IB_sizeBytes;
        mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }

    void D3D12Renderer::ResizeWindow()
    {
        mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f,static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0f, 1.0f);
        ResizeRenderTargets();
        ResizeDepthBuffer();
        mWindowResized = false;
    }

    void D3D12Renderer::ResizeRenderTargets()
    {
        for (uint32_t i{ 0 }; i < mFrameCount; ++i)
            mRenderTargets[i].Reset();

        ThrowIfFailed(mSwapChain->ResizeBuffers(mFrameCount, mWidth, mHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

        for (uint8_t n = 0; n < mFrameCount; n++)
        {
            ThrowIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
            mRtvHandles[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), n, mRtvDescriptorSize);
            mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, mRtvHandles[n]);
            SetName(mRenderTargets[n].Get(), std::format(L"SwapChainBuffer[{}]", n));
        }
    }

    void D3D12Renderer::ResizeDepthBuffer()
    {
        mDepthBuffer.Reset();

        D3D12_CLEAR_VALUE optimizedClearValue{};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        uint32_t const width{ std::max(mWidth, 0U) };
        uint32_t const height{ std::max(mHeight, 0U) };

        CD3DX12_HEAP_PROPERTIES const defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };
        D3D12_RESOURCE_DESC const resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        );
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&mDepthBuffer)
        ));
        SetName(mDepthBuffer.Get(), std::format(L"{}_GPU", L"DepthStencilTexture"));

        /* create the DSV */
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        mDsvHandle = { mDepthStencilHeap->GetCPUDescriptorHandleForHeapStart() };
        mDevice->CreateDepthStencilView(mDepthBuffer.Get(), &dsvDesc, mDsvHandle);
    }

    void D3D12Renderer::CreateBuffer(
        ComPtr<ID3D12GraphicsCommandList> const & commandList,
        ComPtr<ID3D12Resource> & gpuResource,
        ComPtr<ID3D12Resource> & cpuResource,
        void const * data,
        uint64_t sizeBytes,
        std::wstring const & resourceName
    )
    {
        /* create an intermediate resource */
        CD3DX12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
        CD3DX12_RESOURCE_DESC uploadResourceProps{ CD3DX12_RESOURCE_DESC::Buffer(sizeBytes) };
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadResourceProps,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&cpuResource))
        );
        SetName(cpuResource.Get(), std::format(L"{}_CPU", resourceName));

        /* create the target resource on the GPU */
        CD3DX12_HEAP_PROPERTIES const defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };
        CD3DX12_RESOURCE_DESC const gpuResourceProps{ CD3DX12_RESOURCE_DESC::Buffer(sizeBytes, D3D12_RESOURCE_FLAG_NONE) };
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &defaultHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &gpuResourceProps,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&gpuResource))
        );
        SetName(gpuResource.Get(), std::format(L"{}_GPU", resourceName));

        /* transfer the data */
        D3D12_SUBRESOURCE_DATA subresourceData {};
        subresourceData.pData = data;
        subresourceData.RowPitch = subresourceData.SlicePitch = sizeBytes;

        UpdateSubresources(
            commandList.Get(),
            gpuResource.Get(),
            cpuResource.Get(),
            0, 0, 1,
            &subresourceData
        );
    }

    D3D12Renderer::~D3D12Renderer()
    {
        /* Ensure that the GPU is no longer referencing resources that are about to be
         cleaned up by the destructor. */
        WaitForPreviousFrame();
        CloseHandle(mFenceEvent);
    }

    void D3D12Renderer::OnWindowResized(uint32_t width, uint32_t height)
    {
        mWindowResized = true;
        mWidth = std::max(8u, width);
        mHeight = std::max(8u, height);
    }

    void D3D12Renderer::Render(uint64_t deltaTimeMs)
    {
        if (mWindowResized)
        {
            ResizeWindow();
            float windowAspectRatio{ mWidth / static_cast<float>(mHeight) };
            mCamera->UpdateProjectionMatrix(windowAspectRatio);
        }
        /* Rotate the model */
        auto const elapsedTimeMs{ Application::Get().GetTimeManager().GetCurrentTimeMs() };
        auto const rotation{ 0.0002f * DirectX::XM_PI * elapsedTimeMs };
        XMMATRIX const modelMatrix{ XMMatrixMultiply(XMMatrixRotationY(rotation), XMMatrixRotationZ(rotation)) };

        mCamera->UpdateCamera(deltaTimeMs);
        XMMATRIX const& viewMatrix{ mCamera->GetViewMatrix() };
        XMMATRIX const& mProjectionMatrix{ mCamera->GetProjectionMatrix() };

        XMMATRIX mvpMatrix{ XMMatrixMultiply(modelMatrix, viewMatrix) };
        mvpMatrix = XMMatrixMultiply(mvpMatrix, mProjectionMatrix); 

        /* Record all the commands we need to render the scene into the command list. */
        PopulateCommandList(mvpMatrix);

        /* Execute the command list. */
        ID3D12CommandList* ppCommandLists[] { mCommandList.Get() };
        mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        /* Present the frame and inefficiently wait for the frame to render. */
        ThrowIfFailed(mSwapChain->Present(1, 0));
        WaitForPreviousFrame();
    }

    void D3D12Renderer::WaitForPreviousFrame()
    {
        uint64_t const fence{ mFenceValue };
        ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fence));
        mFenceValue++;

        /* Wait until the previous frame is finished */
        if (mFence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
            WaitForSingleObject(mFenceEvent, INFINITE);
        }
    }

    void D3D12Renderer::PopulateCommandList(XMMATRIX const & mvpMatrix)
    {
        //ThrowIfFailed(mCommandAllocator->Reset());
        ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));
        
        uint8_t const frameIndex{ static_cast<uint8_t>(mSwapChain->GetCurrentBackBufferIndex()) };
        /* Set all the state first */
        {            
            mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
            mCommandList->IASetIndexBuffer(&mIndexBufferView);

            mCommandList->RSSetViewports(1, &mViewport);
            mCommandList->RSSetScissorRects(1, &mScissorRect);

            mCommandList->SetPipelineState(mPipelineState.Get());
            mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
            mCommandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(float), &mvpMatrix, 0);

            mCommandList->OMSetRenderTargets(1, &mRtvHandles[frameIndex], true, &mDsvHandle);
        }

        {
            PIXScopedEvent(mCommandList.Get(), PIX_COLOR(0,0,255), L"RenderFrame");

            /* Back buffer to be used as a Render Target */
            CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(
                mRenderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
            ) };
            mCommandList->ResourceBarrier(1, &barrier);

            PIXSetMarker(mCommandList.Get(), PIX_COLOR_DEFAULT, L"SampleMarker");

            {  /* Record commands */
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, mRtvDescriptorSize };
                float const clearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
                mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
                mCommandList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
                mCommandList->DrawIndexedInstanced(mIndexCount, 1, 0, 0, 0);
            }
            /* Indicate that the back buffer will now be used to present. */
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                mRenderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
            );
            mCommandList->ResourceBarrier(1, &barrier);
        }
        ThrowIfFailed(mCommandList->Close());
    }

} // namespace gg
