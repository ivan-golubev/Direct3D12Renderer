module;
#include <cstdint>
#include <DirectXMath.h>
#include <memory>
export module Camera;

import Input;

using awesome::input::InputManager;
using DirectX::XMFLOAT3;
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

export namespace awesome::camera {

	class Camera
	{
	public:
		Camera(std::shared_ptr<InputManager>);
		void UpdateCamera(uint64_t deltaTimeMs);
		void UpdatePerspectiveMatrix(float windowAspectRatio);
		XMMATRIX const & GetViewMatrix() const;
		XMMATRIX const & GetPerspectiveMatrix() const;

	private:
		std::shared_ptr<InputManager> mInputManager;

		XMVECTOR mCameraPos{ 0, 0, 2 };
		XMFLOAT3 const mCameraFwd{ 0, 0, -1 };
		XMVECTOR mCamFwdXZ;

		float mCameraPitch{ 0.f };
		float mCameraYaw{ 0.f };

		XMMATRIX mPerspectiveMatrix{};
		XMMATRIX mViewMatrix{};

		static constexpr float CAM_MOVE_SPEED{ 5.f }; // in metres per second
		static constexpr float CAM_TURN_SPEED{ DirectX::XM_PI }; // in radians per second
		static constexpr float FOV{ 90.0f };
	};

} // namespace awesome::camera