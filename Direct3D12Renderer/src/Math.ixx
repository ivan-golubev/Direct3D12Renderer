module;
#include <DirectXMath.h>
export module Math;

using DirectX::XMMATRIX;

export namespace awesome::math {

	XMMATRIX rotateYMat(float rad);

} // namespace awesome::math