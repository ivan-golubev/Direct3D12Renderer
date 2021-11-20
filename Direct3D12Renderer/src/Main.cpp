#include <windows.h>
#include <iostream>
#include <exception>
#include <cstdint>
#include <format>
import D3D12Renderer;
import ErrorHandling;
import Logging;

using awesome::renderer::D3D12Renderer;
using awesome::errorhandling::ComException;
using namespace awesome::logging;

namespace awesome 
{
    std::shared_ptr<D3D12Renderer> Renderer;
}

int MainLoop() 
{
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nShowCmd)
{
    wchar_t const  CLASS_NAME[] = L"Direct3D 12 Renderer";
    /* register a windows class */
    WNDCLASSEXW winClass {};
    winClass.cbSize = sizeof(WNDCLASSEXW);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = &WindowProc;
    winClass.hInstance = hInstance;
    winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
    winClass.hCursor = LoadCursorW(0, IDC_ARROW);
    winClass.lpszClassName = CLASS_NAME;
    winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

    if (!RegisterClassExW(&winClass)) 
    {
        DebugLog(DebugLevel::Error, L"Failed to register the window class");
        return GetLastError();
    }

    uint32_t const Width{ 1920 };
    uint32_t const Height{ 1080 };

    RECT initialRect = { 0, 0, Width, Height };
    AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
    LONG initialWidth = initialRect.right - initialRect.left;
    LONG initialHeight = initialRect.bottom - initialRect.top;

    HWND windowHandle = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        winClass.lpszClassName,
        CLASS_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        initialWidth,
        initialHeight,
        0, 0, hInstance, 0
    );

    if (!windowHandle) {
        DebugLog(DebugLevel::Error, L"CreateWindowEx failed");
        return GetLastError();
    }

    try
    {
        awesome::Renderer = std::make_shared<D3D12Renderer>(Width, Height, windowHandle);
        awesome::Renderer->Init();

        DebugLog(DebugLevel::Info, L"Successfully initialized the d3d12 renderer");
    }
    catch (ComException const& e)
    {
        DebugLog(DebugLevel::Error, std::format(L"Caught ComException with message: {}", e.whatString()));
        return -1;
    }
    catch (std::exception const& e)
    {
        std::wstring const errorMsg = ToWString(std::format("Caught exception with message: {}", e.what()));
        DebugLog(DebugLevel::Error, errorMsg);
        return -1;
    }
    return MainLoop();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCREATESTRUCT a = LPCREATESTRUCT(lParam);

    LRESULT result = 0;
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_SIZE:
    {   // TODO: implement  window  resizing
        //awesome::Renderer->SetWindowsResized(true);
        break;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
            DestroyWindow(hwnd);
        else { // TODO: process the input
            //inputManager.OnWindowMessage(uMsg, static_cast<unsigned int>(wParam));
        }
        break;
    default:
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}
