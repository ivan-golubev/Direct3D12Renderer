module;
#include <memory>
#include <windows.h>
#include <cstdint>
#include <DirectXMath.h>
module Application;

import TimeManager;
import InputManager;
import D3D12Renderer;
import Logging;

using awesome::time::TimeManager;
using awesome::input::InputManager;
using awesome::renderer::D3D12Renderer;
using namespace awesome::logging;

namespace awesome::application
{
	Application::Application(uint32_t width, uint32_t height, HWND windowHandle)
		: mTimeManager{ std::make_shared<TimeManager>() }
		, mInputManager{ std::make_shared<InputManager>() }
		, mRenderer{ std::make_shared<D3D12Renderer>(width, height, windowHandle)}
	{
		/* Check for DirectX Math library support. */
		if (!DirectX::XMVerifyCPUSupport())
		{
			throw std::exception("Failed to verify DirectX Math library support");
		}
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

	void Application::SetWindowsResized(bool value)
	{
		mRenderer->SetWindowsResized(value);
	}

	void Application::OnWindowMessage(uint32_t uMsg, uint32_t wParam)
	{
		mInputManager->OnWindowMessage(uMsg, wParam);
	}

} // namespace awesome::application 
