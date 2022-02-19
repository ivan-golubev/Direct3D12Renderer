module;
#include <DirectXMath.h>
#include <cmath>
module Math;

using DirectX::XMMATRIX;

namespace awesome::math {

	XMMATRIX rotateYMat(float rad) 
	{
        float sinTheta{ std::sin(rad) };
        float cosTheta{ std::cos(rad) };
        return {
            cosTheta, 0, sinTheta, 0,
            0, 1, 0, 0,
            -sinTheta, 0, cosTheta, 0,
            0, 0, 0, 1
        };
	}

} // namespace awesome::math