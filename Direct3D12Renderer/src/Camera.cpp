module;
#include <DirectXMath.h>
#include <format>
module Camera;

import Input;
import Application;
import Logging;

using awesome::application::Application;
using awesome::input::InputAction;
using awesome::input::InputManager;
using DirectX::XMConvertToRadians;
using DirectX::XMMATRIX;
using DirectX::XMMatrixLookAtLH;
using DirectX::XMMatrixPerspectiveFovLH;
using DirectX::XMVECTOR;
using DirectX::XMFLOAT3;
using DirectX::XMVectorAdd;
using DirectX::XMVectorScale;
using DirectX::XMVectorSubtract;
using DirectX::XMStoreFloat3;

using namespace awesome::logging;

namespace awesome::camera {
    constexpr float CAM_MOVE_SPEED{ 5.f }; // in metres per second
    constexpr float CAM_TURN_SPEED{ DirectX::XM_PI }; // in radians per second
    constexpr float FOV{ 90.f };
    constexpr float NEAR{ 0.1f };
    constexpr float FAR{ 100.f };

    constexpr XMVECTOR forward{ 0.f, 0.f, 1.f };
    constexpr XMVECTOR up{ 0.f, 1.f, 0.f, 0.f };
    constexpr XMVECTOR right{ 1.f, 0.f, 0.f };

    void Camera::UpdateCamera(uint64_t deltaTimeMs) 
    {
        auto& inputManager = Application::Get().GetInputManager();
        float const CAM_MOVE_AMOUNT{ CAM_MOVE_SPEED * deltaTimeMs / 1000.0f };
        {
            XMVECTOR const moveFB{ XMVectorScale(forward, CAM_MOVE_AMOUNT) };
            if (inputManager.IsKeyDown(InputAction::MoveCameraForward))
                mCameraPos = XMVectorAdd(mCameraPos, moveFB);
            if (inputManager.IsKeyDown(InputAction::MoveCameraBack))
                mCameraPos = XMVectorSubtract(mCameraPos, moveFB);
        }
        {
            XMVECTOR const moveLR{ XMVectorScale(right, CAM_MOVE_AMOUNT) };
            if (inputManager.IsKeyDown(InputAction::MoveCameraRight))
                mFocusPoint = XMVectorAdd(mFocusPoint, moveLR);
            if (inputManager.IsKeyDown(InputAction::MoveCameraLeft))
                mFocusPoint = XMVectorSubtract(mFocusPoint, moveLR);
        }
        {
            XMVECTOR const moveUD{ XMVectorScale(up, CAM_MOVE_AMOUNT) };
            if (inputManager.IsKeyDown(InputAction::RaiseCamera))
                mCameraPos = XMVectorAdd(mCameraPos, moveUD);
            if (inputManager.IsKeyDown(InputAction::LowerCamera))
                mCameraPos = XMVectorSubtract(mCameraPos, moveUD);
        }
        {
            float const CAM_TURN_AMOUNT{ CAM_TURN_SPEED * deltaTimeMs / 1000.0f };
            XMVECTOR const moveLR{ XMVectorScale(right, CAM_TURN_AMOUNT) };
            if (inputManager.IsKeyDown(InputAction::TurnCameraRight))
                mFocusPoint = XMVectorAdd(mFocusPoint, moveLR);
            if (inputManager.IsKeyDown(InputAction::TurnCameraLeft))
                mFocusPoint = XMVectorSubtract(mFocusPoint, moveLR);

            XMVECTOR const moveUP{ XMVectorScale(up, CAM_TURN_AMOUNT) };
            if (inputManager.IsKeyDown(InputAction::LookCameraUp))
                mFocusPoint = XMVectorAdd(mFocusPoint, moveUP);
            if (inputManager.IsKeyDown(InputAction::LookCameraDown))
                mFocusPoint = XMVectorSubtract(mFocusPoint, moveUP);
        }
        // TODO: fix the float rounding errors, Up/Down camera movement is wrong
        // debug
#if 1
        XMFLOAT3 cameraPosFloat3{};
        XMFLOAT3 focusPointFloat3{};
        XMStoreFloat3(&cameraPosFloat3, mCameraPos);
        XMStoreFloat3(&focusPointFloat3, mFocusPoint);
        DebugLog(DebugLevel::Info, std::format(L"Camera pos: ({}, {}, {}), Focus: ({}, {}, {})"
            , cameraPosFloat3.x, cameraPosFloat3.y, cameraPosFloat3.z
            , focusPointFloat3.x, focusPointFloat3.y, focusPointFloat3.z)
        );
#endif
        //

        mViewMatrix = XMMatrixLookAtLH(mCameraPos, mFocusPoint, up);
    }

    void Camera::UpdateProjectionMatrix(float windowAspectRatio)
    {
        mProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(FOV), windowAspectRatio, NEAR, FAR);
    }

    XMMATRIX const & Camera::GetViewMatrix() const { return mViewMatrix; }
    XMMATRIX const & Camera::GetProjectionMatrix() const { return mProjectionMatrix; }

} // namespace awesome::camera