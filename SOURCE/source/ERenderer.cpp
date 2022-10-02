#include "pch.h"

//Project includes
#include "ERenderer.h"
#include "ETimer.h"

Elite::Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false },
	m_Camera{ {0.f,0.f, 0.f}, {0.f,0.f,-1.f},45.f },
	m_Cooldown{ 0.25f },
	m_Counter{ 0.f },
	m_WorldRotationMatrix{ FMatrix4::Identity() },
	m_WorldTranslationMatrix{ FMatrix4::Identity() },
	m_WorldMatrix{ FMatrix4::Identity() },
	m_RotationSpeed{ 45.f },
	m_FireMeshVisible{ true },
	m_ClearColor{ 0.1f,0.1f,0.1f },//0.3f
	m_RenderingHardware{ true },
	m_Rotating{true},
	m_TotalIdleTime{0.f},
	m_RotationXWorldMatrix{180.f}

{
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	//Initialize DirectX pipeline
	m_IsInitialized = InitializeDirectXVariables();
	m_pVehicleMesh = new Mesh{ m_pDevice ,"Resources/vehicle.obj","Resources/vehicle_diffuse.png","Resources/vehicle_normal.png","Resources/vehicle_gloss.png","Resources/vehicle_specular.png" };
	m_pFireMesh = new Mesh{ m_pDevice, "Resources/fireFX.obj","Resources/fireFX_diffuse.png" };

	//Initialize matrices
	m_WorldTranslationMatrix[3][2] = -50.f;

	m_Camera.CreateProjectionMatrix(100.f,0.1f, (static_cast<float>(width) / static_cast<float>(height)));
	//software vars
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_DepthBuffer = std::vector<float>((static_cast<size_t>(m_Width) * static_cast<size_t>(m_Height)));
	std::fill(m_DepthBuffer.begin(), m_DepthBuffer.end(), 1.f);
}

Elite::Renderer::~Renderer()
{
	m_pFireMesh->CleanUp();
	m_pVehicleMesh->CleanUp();
	delete m_pVehicleMesh;
	m_pVehicleMesh = nullptr;

	delete m_pFireMesh;
	m_pFireMesh = nullptr;

	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}

	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
		m_pRenderTargetBuffer = nullptr;
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}

	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
		m_pDeviceContext = nullptr;
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
		m_pDXGIFactory = nullptr;
	}
}

//return false if the Initializing fails, otherwise it returns true
bool Elite::Renderer::InitializeDirectXVariables()
{
	//Initialize DirectX pipeline
	// Create device and device contexxt, using acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
	//for debug
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if (FAILED(result))
		return false;

	//create DXGI Factory to create SwapCHain based on hardware
	//IDXGIFactory* pDXGIFactory;
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory)); //m_pDXGIFactory
	if (FAILED(result))
		return false;
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the hande GWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;
	//Create SwapCHain and hook it into the handle of the sdl window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return false;

	//create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//resource view for our Depth/Stencil Buffer.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilViewDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	//Now we can create the actual resource and the “matching” resource view
	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return false;
	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return false;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return false;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return false;

	//Bind the views to the output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);


	return true;
}

void Elite::Renderer::KeysUpdate(float elapsedSec)
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
	m_Counter += elapsedSec;
	if (m_Counter < m_Cooldown)
		return;

	if (pKeyboardState[SDL_SCANCODE_F])
	{
		m_Counter = 0.f;
		m_pVehicleMesh->ToggleThroughSamplerStates();
		m_pFireMesh->ToggleThroughSamplerStates();
	}
	if (pKeyboardState[SDL_SCANCODE_T])
	{
		m_Counter = 0.f;
		m_FireMeshVisible = !m_FireMeshVisible;
	}
	if (pKeyboardState[SDL_SCANCODE_E])
	{
		m_Counter = -1;
		m_RenderingHardware = !m_RenderingHardware;
		ToggleRender();	
	}
	if (pKeyboardState[SDL_SCANCODE_R])
	{
		m_Counter = 0.f;
		m_Rotating = !m_Rotating;
	}
	if (pKeyboardState[SDL_SCANCODE_C])
	{
		m_Counter = 0.f;
		m_pVehicleMesh->ToggleRasterState();
	}
}
void Elite::Renderer::ToggleRender()
{
	float nearDistance{ 0.1f };
	float farCDistance{ 100.f };
	if (m_RenderingHardware)
	{
		std::cout << "Rendering Hardware\n";
		m_RotationSpeed *= -1;
		m_WorldTranslationMatrix[3][2] *= -1.f;
	}
	else
	{
		std::cout << "Rendering Software\n";
		m_RotationSpeed *= -1;
		m_WorldTranslationMatrix[3][2] *= -1.f;
	}
	m_Camera.HardwareRendering(m_RenderingHardware);
}

bool Elite::Renderer::IsInCullRange(const std::vector<Vertex>& transformedVertices)
{
	for (size_t j = 0; j < transformedVertices.size(); j++)
	{
		if (transformedVertices[j].worldPosition.z < 0.f || transformedVertices[j].worldPosition.z > 1.f)
		{
			return true;
		}
		if (transformedVertices[j].worldPosition.y < -1.f || transformedVertices[j].worldPosition.y > 1.f)
		{
			return true;
		}
		if (transformedVertices[j].worldPosition.x < -1.f || transformedVertices[j].worldPosition.x > 1.f)
		{
			return true;
		}
	}
	return false;
}

void Elite::Renderer::SoftwareVerticesTransformationFunction(const std::vector<Vertex>& originalVertices, std::vector<Vertex>& transformedVertices)
{
	FMatrix4 wvp = m_Camera.GetProjectionMatrix() * m_Camera.GetWorldToView() * m_WorldMatrix;
	transformedVertices.clear();
	for (size_t i = 0; i < originalVertices.size(); i++)
	{
		Vertex tmpCV{ originalVertices[i] };
		tmpCV.worldPosition = wvp * tmpCV.worldPosition;
		tmpCV.worldPosition.x /= tmpCV.worldPosition.w;
		tmpCV.worldPosition.y /= tmpCV.worldPosition.w;
		tmpCV.worldPosition.z /= tmpCV.worldPosition.w;
		//checks if position is in cull range
		if (IsInCullRange({ tmpCV }))
		{
			return;
		}
		tmpCV.worldPosition.x = ((tmpCV.worldPosition.x + 1.f) / 2.f) * m_Width;
		tmpCV.worldPosition.y = ((1.f - tmpCV.worldPosition.y) / 2.f) * m_Height;
		//// we multiply our normals with the World matrix, NOT the WorldViewProjection matrix.
		tmpCV.normal = (FMatrix3(m_WorldMatrix) * (tmpCV.normal));
		tmpCV.tangent = (FMatrix3(m_WorldMatrix) * (tmpCV.tangent));
		transformedVertices.push_back(tmpCV);
	}
}



void Elite::Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer->GetElapsed());
	KeysUpdate(pTimer->GetElapsed());
	//update matrices
	if (m_Rotating)
	{	
		if (m_RenderingHardware)
		{
			m_WorldRotationMatrix = MakeRotationY(ToRadians((m_RotationSpeed * (pTimer->GetTotal() - m_TotalIdleTime))));	
		}
		else
		{
			m_WorldRotationMatrix = MakeRotationZYX(0.f, ToRadians((m_RotationSpeed * (pTimer->GetTotal() - m_TotalIdleTime))), ToRadians(m_RotationXWorldMatrix));
		}
	}
	else
	{
		m_TotalIdleTime += pTimer->GetElapsed();
	}
	m_WorldMatrix = m_WorldTranslationMatrix * m_WorldRotationMatrix;
	//Update Meshes
	m_pVehicleMesh->Update(m_Camera.GetProjectionMatrix() * m_Camera.GetWorldToView() * m_WorldMatrix, m_WorldMatrix, m_Camera.GetViewToWorld());
	m_pFireMesh->Update(m_Camera.GetProjectionMatrix() * m_Camera.GetWorldToView() * m_WorldMatrix, m_WorldMatrix, m_Camera.GetViewToWorld());
}

void Elite::Renderer::Render()
{
	if (!m_IsInitialized)
		return;
	if (m_RenderingHardware)
	{
		//Directx
		HardwareRender();	
	}
	else
	{
		//software
		SoftwareRender();
	}
}






void Elite::Renderer::HardwareRender()
{
	//Directx
	//clear buffers
	RGBColor clearColor{ 0.1f,0.1f,0.1f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//Render
	m_pVehicleMesh->HardwareRender(m_pDeviceContext);
	if (m_FireMeshVisible)
	{
		m_pFireMesh->HardwareRender(m_pDeviceContext);
	}
	//Present
	m_pSwapChain->Present(0, 0);
}

void Elite::Renderer::SoftwareRender()
{
	//lock backbuffer
	SDL_LockSurface(m_pBackBuffer);
	//pixel variables
	size_t startingC{ 0 };
	size_t startingR{ 0 };
	size_t sizeC{ m_Width };
	size_t sizeR{ m_Height };
	Vertex pixelVertex{};
	RGBColor pixelColor{};
	//VertexVertices
	std::vector<Vertex> transformedVertices{};
	//refilling depth amnd back buffer
	std::fill(m_DepthBuffer.begin(), m_DepthBuffer.end(), 1.f);
	SDL_FillRect(m_pBackBuffer, NULL, 0x1E1E1E);

	for (size_t i = 0; i < m_pVehicleMesh->GetAmountOfTriangles(); i += 3)
	{
		//transfrom the vertices
		SoftwareVerticesTransformationFunction(m_pVehicleMesh->GetTriangleVertices(i), transformedVertices);
		//skips render if its culled
		if (transformedVertices.size() < 3)
		{
			continue;
		}
		//created the bounding boxes
		m_pVehicleMesh->CreateBoundingBox(transformedVertices,static_cast<float>(m_Width), static_cast<float>(m_Height),startingC,startingR,sizeC,sizeR);

		//goes through pixels
		for (size_t r = startingR; r < sizeR; ++r)
		{
			for (size_t c = startingC; c < sizeC; ++c)
			{
				
				//looks if the pixel is in the triangle and then makes the pixel vertex, also checks depth
				if (m_pVehicleMesh->CreatePixelVertex(transformedVertices, { static_cast<float>(c),static_cast<float>(r) }, m_DepthBuffer[(c + (r * static_cast<size_t>(m_Width)))], pixelVertex))
				{
					const FVector3 viewDirection{ GetNormalized(m_Camera.GetPosition() - pixelVertex.worldPosition.xyz) };
					//shading
					pixelColor = m_pVehicleMesh->PixelShading(m_Camera.GetPosition(),pixelVertex);
					m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(pixelColor.r * 255.f),
						static_cast<uint8_t>(pixelColor.g * 255.f),
						static_cast<uint8_t>(pixelColor.b * 255.f));
						
				}
			}

		}
	}
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}
