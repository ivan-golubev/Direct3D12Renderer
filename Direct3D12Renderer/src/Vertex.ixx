module;
#include <DirectXMath.h>
export module Vertex;

using DirectX::XMFLOAT3;

namespace awesome::structs {

	export struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
	};

}
