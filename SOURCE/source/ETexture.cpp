#include "pch.h"
#include "ETexture.h"
#include "SDL_image.h"

Elite::Texture::Texture(const std::string& filePath, ID3D11Device* pDevice):
	m_pSurface{ IMG_Load(filePath.c_str()) },
	m_pTexture{nullptr},
	m_pTextureRescourceView{nullptr},
	m_MaxColorValue{255.f}
{
	if (!m_pSurface)
	{
		return;
	}
	//createing Texture
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);
	if (FAILED(hr))
		return;

	//Creating texture rescource
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = SRVDesc.Format; //is porbs wrong
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pTextureRescourceView);



}

Elite::Texture::Texture():
	m_pSurface{  },
	m_pTexture{ nullptr },
	m_pTextureRescourceView{ nullptr },
	m_MaxColorValue{255.f}
{
}


Elite::RGBColor Elite::Texture::Sample(const Elite::FVector2& uv) const
{
	float u = uv.x * m_pSurface->w;
	float v = uv.y * m_pSurface->h;
	Uint32 index = static_cast<Uint32>(roundf(u) + (roundf(v) * m_pSurface->w));
	Uint8 r{ 0 };
	Uint8 g{ 0 };
	Uint8 b{ 0 };
	SDL_GetRGB(((uint32_t*)m_pSurface->pixels)[index], m_pSurface->format, &r, &g, &b);
	RGBColor color{ static_cast<float>(r),static_cast<float>(g),static_cast<float>(b) };
	color /= m_MaxColorValue;
	return color;

}



ID3D11ShaderResourceView* Elite::Texture::GetTextureRescourceView() const
{
	return m_pTextureRescourceView;
}
Elite::Texture::~Texture()
{
	SDL_FreeSurface(m_pSurface);

	if (m_pTextureRescourceView)
	{
		m_pTextureRescourceView->Release();
		m_pTextureRescourceView = nullptr;
	}


	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
}


