module;
#include <DirectXMath.h>
#include <cstdint>
#include <memory>
export module Camera;

import Input;

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;
using awesome::input::InputManager;

export namespace awesome::camera {

	class Camera
	{
	public:
		Camera(std::shared_ptr<InputManager>);
		void UpdateCamera(uint64_t deltaTimeMs);
		void UpdatePerspectiveMatrix(float windowAspectRatio);
		XMMATRIX& GetViewMatrix();
		XMMATRIX& GetPerspectiveMatrix();

	private:
		std::shared_ptr<InputManager> mInputManager;

		XMFLOAT3 cameraPos{ 0, 0, 2 };
		XMFLOAT3 cameraFwd{ 0, 0, -1 };
		float cameraPitch{ 0.f };
		float cameraYaw{ 0.f };

		float const mFoV{ 90.0f };

		XMMATRIX perspectiveMatrix{};
		XMMATRIX viewMatrix{};

		float const CAM_MOVE_SPEED{ 5.f }; // in metres per second
		float const CAM_TURN_SPEED{ DirectX::XM_PI }; // in radians per second
	};

} // namespace awesome::camera