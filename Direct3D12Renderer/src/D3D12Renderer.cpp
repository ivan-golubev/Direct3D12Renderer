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

	void D3D12Renderer::Init() 
    {
		uint8_t dxgiFactoryFlags{ 0 };
        /* Enable the debug layer */
        if constexpr ( IsDebug() )
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
        
        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(
            CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))
        );

        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter, true);
        
        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&mDevice)
        ));
	}

} // namespace awesome
