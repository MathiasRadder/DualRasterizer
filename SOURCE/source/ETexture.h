#pragma once
#include <string>
#include "EMath.h"
#include "ERGBColor.h"
struct SDL_Surface;
namespace Elite
{
	class Texture
	{
	public:
		Texture(const std::string& filePath, ID3D11Device* pDevice);
		Texture();
		ID3D11ShaderResourceView* GetTextureRescourceView() const;
		RGBColor Sample(const Elite::FVector2& uv) const;
		~Texture();
	private:
		SDL_Surface* m_pSurface;
		ID3D11ShaderResourceView* m_pTextureRescourceView;
		ID3D11Texture2D* m_pTexture;
		const float m_MaxColorValue;
	};

}

