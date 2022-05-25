module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <string>
#include <windows.h>
#include <wrl.h>
module D3DHelpers;

import GlobalSettings;

using Microsoft::WRL::ComPtr;
using awesome::globals::IsFinal;

namespace awesome::d3dhelpers {

    /* Taken from Microsoft's DirectX-Graphics-Samples  */
	void GetHardwareAdapter(
		_In_ IDXGIFactory1* pFactory,
		_Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
		bool requestHighPerformanceAdapter  ) 
	{
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<IDXGIFactory6> factory6;

        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (nullptr == adapter.Get())
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }
        *ppAdapter = adapter.Detach();
	}

    void SetName(ID3D12Object* object, std::wstring const & name)
    {
        if constexpr (!IsFinal())
        {
            object->SetName(name.c_str());
        }
    }

    std::wstring GetName(ID3D12Object* pObject)
    {
        if constexpr (IsFinal())
            return {};
        else 
        {
            uint32_t size{ 0 };
            if (FAILED(pObject->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, nullptr)))
            {
                return L"Unnamed";
            }
            std::wstring name(size, 0);
            if (FAILED(pObject->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name.data())))
            {
                return L"Unnamed";
            }
            return name;
        }
    }

} // namespace awesome::d3dhelpers