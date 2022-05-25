module;
#include <DirectXMath.h>
#include <format>
#include <memory>
module Camera;

import Input;
import Logging;

using awesome::logging::DebugLog;
using awesome::logging::DebugLevel;
using awesome::input::InputAction;
using awesome::input::InputManager;
using DirectX::XMConvertToRadians;
using DirectX::XMMATRIX;
using DirectX::XMMatrixPerspectiveFovLH;
using DirectX::XMVECTOR;
using DirectX::XMFLOAT3;
using DirectX::XMVector3Cross;
using DirectX::XMVector3Normalize;
using DirectX::XMVectorScale;
using DirectX::XMVectorAdd;
using DirectX::XMVectorSubtract;
using DirectX::XMStoreFloat3;

namespace awesome::camera {

    static XMFLOAT3 const mCameraFwd{ 0, 0, -1 };

    Camera::Camera(std::shared_ptr<InputManager> im) 
        : mInputManager{ im }
        , mCamFwdXZ{ XMVector3Normalize({ mCameraFwd.x, 0, mCameraFwd.z }) }
    {}

    void Camera::UpdateCamera(uint64_t deltaTimeMs) 
    {
        float const CAM_MOVE_AMOUNT{ CAM_MOVE_SPEED * deltaTimeMs / 1000.0f };
        {
            XMVECTOR const moveFB{ XMVectorScale(mCamFwdXZ, CAM_MOVE_AMOUNT) };
            if (mInputManager->IsKeyDown(InputAction::MoveCameraForward))
                mCameraPos = XMVectorAdd(mCameraPos, moveFB);
            if (mInputManager->IsKeyDown(InputAction::MoveCameraBack))
                mCameraPos = XMVectorSubtract(mCameraPos, moveFB);
        }
        {
            XMVECTOR const cameraRightXZ{ XMVector3Cross(mCamFwdXZ, { 0, 1, 0 }) };
            XMVECTOR const moveLR{ XMVectorScale(cameraRightXZ, CAM_MOVE_AMOUNT) };
            if (mInputManager->IsKeyDown(InputAction::MoveCameraLeft))
                mCameraPos = XMVectorSubtract(mCameraPos, moveLR);
            if (mInputManager->IsKeyDown(InputAction::MoveCameraRight))
                mCameraPos = XMVectorAdd(mCameraPos, moveLR);
        }
        {
            XMVECTOR const moveUD{ XMVECTOR{ 0.0f, CAM_MOVE_AMOUNT } };
            if (mInputManager->IsKeyDown(InputAction::RaiseCamera))
                mCameraPos = XMVectorAdd(mCameraPos, moveUD);
            if (mInputManager->IsKeyDown(InputAction::LowerCamera))
                mCameraPos = XMVectorSubtract(mCameraPos, moveUD);
        }
        {
            float const CAM_TURN_AMOUNT{ CAM_TURN_SPEED * deltaTimeMs / 1000.0f };
            if (mInputManager->IsKeyDown(InputAction::TurnCameraLeft))
                mCameraYaw += CAM_TURN_AMOUNT;
            if (mInputManager->IsKeyDown(InputAction::TurnCameraRight))
                mCameraYaw -= CAM_TURN_AMOUNT;
            if (mInputManager->IsKeyDown(InputAction::LookCameraUp))
                mCameraPitch += CAM_TURN_AMOUNT;
            if (mInputManager->IsKeyDown(InputAction::LookCameraDown))
                mCameraPitch -= CAM_TURN_AMOUNT;
        }
        XMFLOAT3 cameraPos{};
        XMStoreFloat3(&cameraPos, mCameraPos);
        DebugLog(DebugLevel::Info, std::format(L"Camera position: ({}, {}, {})", cameraPos.x, cameraPos.y, cameraPos.z));
        //// Wrap yaw to avoid floating-point errors if we turn too far
        //while (cameraYaw >= 2 * static_cast<float>(DirectX::XM_PI))
        //    cameraYaw -= 2 * static_cast<float>(DirectX::XM_PI);
        //while (cameraYaw <= -2 * static_cast<float>(DirectX::XM_PI))
        //    cameraYaw += 2 * static_cast<float>(DirectX::XM_PI);

        //// Clamp pitch to stop camera flipping upside down
        //if (cameraPitch > XMConvertToRadians(85))
        //    cameraPitch = XMConvertToRadians(85);
        //if (cameraPitch < -XMConvertToRadians(85))
        //    cameraPitch = -XMConvertToRadians(85);

        //mViewMatrix = translationMat(-mCameraPos) * rotateYMat(-mCameraYaw) * rotateXMat(-mCameraPitch);
    }

    void Camera::UpdatePerspectiveMatrix(float windowAspectRatio) 
    {
        mPerspectiveMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(FOV), windowAspectRatio, 0.1f, 100.0f);
    }

    XMMATRIX const & Camera::GetViewMatrix() const { return mViewMatrix; }
    XMMATRIX const & Camera::GetPerspectiveMatrix() const { return mPerspectiveMatrix; }

} // namespace awesome::camera