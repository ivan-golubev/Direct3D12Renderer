module;
#include <cmath>
#include <DirectXMath.h>
module Math;

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;

namespace awesome::math {

	XMMATRIX rotateXMat(float rad) 
	{
        float const sinTheta{ std::sin(rad) };
        float const cosTheta{ std::cos(rad) };
        return {
            1, 0, 0, 0,
            0, cosTheta, -sinTheta, 0,
            0, sinTheta, cosTheta, 0,
            0, 0, 0, 1
        };
	}

    XMMATRIX rotateYMat(float rad)
    {
        float const sinTheta{ std::sin(rad) };
        float const cosTheta{ std::cos(rad) };
        return {
            cosTheta, 0, sinTheta, 0,
            0, 1, 0, 0,
            -sinTheta, 0, cosTheta, 0,
            0, 0, 0, 1
        };
    }

    XMMATRIX translationMat(XMFLOAT3 translation)
    {
        return {
            1, 0, 0, translation.x,
            0, 1, 0, translation.y,
            0, 0, 1, translation.z,
            0, 0, 0, 1
        };
    }

} // namespace awesome::math