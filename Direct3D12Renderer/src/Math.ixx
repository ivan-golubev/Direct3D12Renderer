module;
#include <DirectXMath.h>
export module Math;

using DirectX::XMMATRIX;
using DirectX::XMFLOAT3;

export namespace awesome::math {

	XMMATRIX rotateXMat(float rad);
	XMMATRIX rotateYMat(float rad);
	XMMATRIX translationMat(XMFLOAT3 translation);

} // namespace awesome::math