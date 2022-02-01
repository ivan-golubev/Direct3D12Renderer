module;
#include <DirectXMath.h>
export module Vertex;

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;

namespace awesome::structs {

	export struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

}
