/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include "EMesh.h"
#include "ECamera.h"
struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	class Timer;
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;
		void Update(Timer* pTimer);
		void Render();

	private:
		void ToggleRender();
		void HardwareRender();
		void SoftwareRender();
		bool InitializeDirectXVariables();
		void KeysUpdate(float elapsedSec);
		bool IsInCullRange(const std::vector<Vertex>& transformedVertices);
		void SoftwareVerticesTransformationFunction(const std::vector<Vertex>& originalVertices, std::vector<Vertex>& transformedVertices);


		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		std::vector<float> m_DepthBuffer;
		bool m_IsInitialized;
		//directx vars
		ID3D11Device* m_pDevice;
		IDXGISwapChain* m_pSwapChain;
		ID3D11DeviceContext* m_pDeviceContext;
		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		IDXGIFactory* m_pDXGIFactory;
		//meshes
		Mesh* m_pVehicleMesh;
		Mesh* m_pFireMesh;
		bool m_FireMeshVisible;
		//camera
		Camera m_Camera;
		//matrixes
		FMatrix4 m_WorldRotationMatrix;
		FMatrix4 m_WorldTranslationMatrix;
		FMatrix4 m_WorldMatrix;
		float m_RotationSpeed;
		bool m_Rotating;
		float m_TotalIdleTime;
		const float m_RotationXWorldMatrix;
		//keys couldown
		float m_Counter;
		const float m_Cooldown;
		//Render vars
		const RGBColor m_ClearColor;
		bool m_RenderingHardware;
	};
}

#endif