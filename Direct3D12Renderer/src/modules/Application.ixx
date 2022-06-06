module;
#include <cstdint>
#include <memory>
#include <windows.h>
export module Application;

import D3D12Renderer;
import Input;
import TimeManager;

using awesome::input::InputManager;
using awesome::renderer::D3D12Renderer;
using awesome::time::TimeManager;

namespace awesome::application 
{
	export class Application 
	{
	public:
		static Application& Init(uint32_t width, uint32_t height, HWND windowHandle);
		static void Destroy();
		static bool IsInitialized();
		static Application& Get();

		void Tick();
		void OnWindowResized(uint32_t width, uint32_t height);
		void OnWindowMessage(uint32_t uMsg, uint32_t wParam);

		TimeManager& GetTimeManager() const;
		InputManager& GetInputManager() const;
	private:
		Application(uint32_t width, uint32_t height, HWND windowHandle);
		~Application();

		static Application* INSTANCE;

		std::unique_ptr<TimeManager> mTimeManager{};
		std::unique_ptr<InputManager> mInputManager{};
		std::unique_ptr<D3D12Renderer> mRenderer{};
	};
} // namespace awesome::application 