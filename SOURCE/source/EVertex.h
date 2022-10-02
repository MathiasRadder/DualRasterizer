#pragma once
#include "Emath.h"
#include "ERGBColor.h"
namespace Elite
{

	struct Vertex
	{
		FPoint3 position = {0.f,0.f,0.f};
		FPoint4 worldPosition = {0.f,0.f,0.f,0.f};
		FVector3 normal{ 0.f,0.f,0.f };
		FVector3 tangent{ 0.f,0.f,0.f };
		FVector2 uv{0.f,0.f};//0 to 1
	};
}
