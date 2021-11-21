module;
#include <cstdint>
export module InputManager;

namespace awesome::input
{
	enum InputAction {
		RaiseCamera,
		LowerCamera,
		MoveCameraLeft,
		MoveCameraRight,
		MoveCameraForward,
		MoveCameraBack,
		TurnCameraLeft,
		TurnCameraRight,
		LookCameraUp,
		LookCameraDown,
		Count
	};

	export class InputManager {
	public:
		void OnWindowMessage(uint32_t uMsg, uint32_t wParam);
		void SetKeyDown(InputAction a, bool value);
		void ClearKeys();
		bool IsKeyDown(InputAction a) const;
		float GetPlayerSpeed(uint64_t deltaMs) const;
	private:
		bool Keys[InputAction::Count];
		float PlayerSpeed{ 1.5f };
	};

} // namespace awesome::input
