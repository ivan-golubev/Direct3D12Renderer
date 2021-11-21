module;
#include <memory>
#include <windows.h>
#include <cstdint>
export module Application;

import TimeManager;
import InputManager;
import D3D12Renderer;

using awesome::time::TimeManager;
using awesome::input::InputManager;
using awesome::renderer::D3D12Renderer;

namespace awesome::application 
{
	export class Application 
	{
	public:
		Application(uint32_t width, uint32_t height, HWND windowHandle);
		~Application();
		void Tick();
		void SetWindowsResized(bool);
		void OnWindowMessage(uint32_t uMsg, uint32_t wParam);
	private:
		std::shared_ptr<TimeManager> mTimeManager{};
		std::shared_ptr<InputManager> mInputManager{};
		std::shared_ptr<D3D12Renderer> mRenderer{};
	};
} // namespace awesome::application 
