#include <exception>
#include <format>
#include <memory>
#include <windows.h>
import Application;
import ErrorHandling;
import Logging;

using namespace gg;

void MainLoop();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int /*nShowCmd*/)
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

    uint32_t const width{ 1920 };
    uint32_t const height{ 1080 };

    RECT initialRect{ 0, 0, width, height };
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

    if (!windowHandle) 
    {
        DebugLog(DebugLevel::Error, L"CreateWindowEx failed");
        return GetLastError();
    }

    try
    {
        Application::Init(width, height, windowHandle);
        DebugLog(DebugLevel::Info, L"Successfully initialized the d3d12 application");
    }
    catch (ComException const& e)
    {
        DebugLog(DebugLevel::Error, std::format(L"Caught ComException with message: {}", e.whatString()));
        __debugbreak();
        return EXIT_FAILURE;
    }
    catch (std::exception const& e)
    {
        std::wstring const errorMsg = ToWString(std::format("Caught exception with message: {}", e.what()));
        DebugLog(DebugLevel::Error, errorMsg);
        return EXIT_FAILURE;
    }
    MainLoop();
    Application::Destroy();
    return EXIT_SUCCESS;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result{ 0 };
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
    {   
        RECT rect;
        if (GetClientRect(hwnd, &rect) && Application::IsInitialized())
            Application::Get().OnWindowResized(rect.right - rect.left, rect.bottom - rect.top);
        break;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
            DestroyWindow(hwnd);
        else if (Application::IsInitialized())
            Application::Get().OnWindowMessage(uMsg, static_cast<unsigned int>(wParam));
        break;
    default:
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return result;
}

void MainLoop()
{
    MSG msg{};
    bool isRunning{ true };

    while (isRunning)
    {
        if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Application::Get().Tick();
    }
}
