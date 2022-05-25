module;
#include <cstdint>
#include <DirectXMath.h>
#include <memory>
#include <windows.h>
module Application;

import D3D12Renderer;
import Input;
import Logging;
import TimeManager;

using awesome::input::InputManager;
using awesome::renderer::D3D12Renderer;
using awesome::time::TimeManager;
using namespace awesome::logging;

namespace awesome::application
{
	Application::Application(uint32_t width, uint32_t height, HWND windowHandle)
		: mTimeManager{ std::make_shared<TimeManager>() }
		, mInputManager{ std::make_shared<InputManager>() }
		, mRenderer{ std::make_shared<D3D12Renderer>(width, height, windowHandle, mInputManager)}
	{
		/* Check for DirectX Math library support. */
		if (!DirectX::XMVerifyCPUSupport())
			throw std::exception("Failed to verify DirectX Math library support");
	}

	Application::~Application() 
	{
		DebugLog(DebugLevel::Info, L"Shutting down the application");
	}

	void Application::Tick()
	{
		uint64_t dt = mTimeManager->Tick();
		mRenderer->Render(dt);
	}

	void Application::OnWindowResized(uint32_t width, uint32_t height)
	{
		mRenderer->OnWindowResized(width, height);
	}

	void Application::OnWindowMessage(uint32_t uMsg, uint32_t wParam)
	{
		mInputManager->OnWindowMessage(uMsg, wParam);
	}

} // namespace awesome::application 
