module;
#include <DirectXMath.h>
#include <memory>
module Camera;

import Input;

using DirectX::XMMATRIX;
using DirectX::XMVECTOR;
using DirectX::XMVector3Normalize;
using DirectX::XMMatrixPerspectiveFovLH;
using DirectX::XMConvertToRadians;
using DirectX::XMVector3Cross;
using DirectX::XMVectorScale;
using awesome::input::InputManager;
using awesome::input::InputAction;

namespace awesome::camera {

    Camera::Camera(std::shared_ptr<InputManager> im) : mInputManager(im) {}

    void Camera::UpdateCamera(uint64_t deltaTimeMs) {
        XMVECTOR camFwdXZ = XMVector3Normalize({ cameraFwd.x, 0, cameraFwd.z });
        XMVECTOR cameraRightXZ = XMVector3Cross(camFwdXZ, { 0, 1, 0 });
        // TODO: figure out these vector classes
        float const CAM_MOVE_AMOUNT = CAM_MOVE_SPEED * deltaTimeMs / 1000.0f;
        if (mInputManager->IsKeyDown(InputAction::MoveCameraForward))
            cameraPos += XMVectorScale(camFwdXZ, CAM_MOVE_AMOUNT);
        if (mInputManager->IsKeyDown(InputAction::MoveCameraBack))
            cameraPos -= XMVectorScale(camFwdXZ, CAM_MOVE_AMOUNT);
        if (mInputManager->IsKeyDown(InputAction::MoveCameraLeft))
            cameraPos -= XMVectorScale(cameraRightXZ, CAM_MOVE_AMOUNT);
        if (mInputManager->IsKeyDown(InputAction::MoveCameraRight))
            cameraPos += XMVectorScale(cameraRightXZ, CAM_MOVE_AMOUNT);
        if (mInputManager->IsKeyDown(InputAction::RaiseCamera))
            cameraPos += XMVECTOR{ 0.0f, CAM_MOVE_AMOUNT };
        if (mInputManager->IsKeyDown(InputAction::LowerCamera))
            cameraPos -= XMVECTOR{ 0.0f, CAM_MOVE_AMOUNT };

        float const CAM_TURN_AMOUNT = CAM_TURN_SPEED * deltaTimeMs / 1000.0f;
        if (mInputManager->IsKeyDown(InputAction::TurnCameraLeft))
            cameraYaw += CAM_TURN_AMOUNT;
        if (mInputManager->IsKeyDown(InputAction::TurnCameraRight))
            cameraYaw -= CAM_TURN_AMOUNT;
        if (mInputManager->IsKeyDown(InputAction::LookCameraUp))
            cameraPitch += CAM_TURN_AMOUNT;
        if (mInputManager->IsKeyDown(InputAction::LookCameraDown))
            cameraPitch -= CAM_TURN_AMOUNT;

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

        ////viewMatrix = translationMat(-cameraPos) * rotateYMat(-cameraYaw) * rotateXMat(-cameraPitch);
    }

    void Camera::UpdatePerspectiveMatrix(float windowAspectRatio) {
        perspectiveMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(mFoV), windowAspectRatio, 0.1f, 100.0f);
    }

    XMMATRIX& Camera::GetViewMatrix() { return viewMatrix; }
    XMMATRIX& Camera::GetPerspectiveMatrix() { return perspectiveMatrix; }

} // namespace awesome::camera