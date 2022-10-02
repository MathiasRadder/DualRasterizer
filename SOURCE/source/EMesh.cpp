#include "pch.h"
#include "EMesh.h"
#include "EEffect.h"
#include "EFlatEffect.h"
#include <string>
#include <iostream>
#include <fstream>
#include "EOBJParser.h"


Elite::Mesh::Mesh(ID3D11Device* pDevice, const std::string& fileName, const std::string& diffTextureFilePath, const std::string& normTextureFilePath, const std::string& glossTextureFilePath, const std::string& specTextureFilePath):
	m_pEffect{ new Effect{pDevice, L"Resources/PosCol3D.fx"} },
	m_AmountOfIndices{},
	m_pIndexBuffer{ nullptr },
	m_pVertexBuffer{ nullptr },
	m_pVertexLayout{ nullptr },
	m_DiffuseTexture{ diffTextureFilePath,pDevice },
	m_NormalTexture{ normTextureFilePath,pDevice },
	m_GlossinessTexture{ glossTextureFilePath,pDevice },
	m_SpecularTexture{ specTextureFilePath,pDevice },
	m_CullMode{CullModes::back}

{
	
	Initialize(pDevice, fileName);


}

Elite::Mesh::Mesh(ID3D11Device* pDevice, const std::string& fileName, const std::string& diffTextureFilePath):

	m_pEffect{ new FlatEffect{pDevice, L"Resources/PosCol3D.fx"} },
	m_AmountOfIndices{},
	m_pIndexBuffer{ nullptr },
	m_pVertexBuffer{ nullptr },
	m_pVertexLayout{ nullptr },
	m_DiffuseTexture{ diffTextureFilePath,pDevice },
	m_CullMode{ CullModes::none }

{
	Initialize(pDevice, fileName);
	
}

Elite::Mesh::~Mesh()
{
	if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
		m_pVertexLayout  = nullptr;
	}
}

void Elite::Mesh::Update(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix)
{
	// chaning the world view proj matrix
	m_pEffect->UpdateMatrixes(worldViewprojMatrix, worldMatrix, viewInverseMatrix);
	//adding the textures
	m_pEffect->UpdateShaderMaps(m_DiffuseTexture.GetTextureRescourceView(), m_NormalTexture.GetTextureRescourceView(), 
		m_GlossinessTexture.GetTextureRescourceView(), m_SpecularTexture.GetTextureRescourceView());
}

void Elite::Mesh::HardwareRender(ID3D11DeviceContext* pDeviceContext)
{
	//set vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//set index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	//set the input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//render a triangle
	D3DX11_TECHNIQUE_DESC techDesc;
	SetRasterState(pDeviceContext);
	m_pEffect->SetStates(pDeviceContext);
	m_pEffect->GetPointerTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		m_pEffect->GetPointerTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountOfIndices, 0, 0);
	}

}

void Elite::Mesh::CreateBoundingBox(const std::vector<Vertex>& transformedVertices, float screenWidth, float ScreenHeight, size_t& startC, size_t& startR, size_t& sizeC, size_t& sizeR)
{
	auto smallestX = std::min(std::min(transformedVertices[0].worldPosition.x, transformedVertices[1].worldPosition.x), transformedVertices[2].worldPosition.x);
	auto smallestY = std::min(std::min(transformedVertices[0].worldPosition.y, transformedVertices[1].worldPosition.y), transformedVertices[2].worldPosition.y);
	auto biggestX = std::max(std::max(transformedVertices[0].worldPosition.x, transformedVertices[1].worldPosition.x), transformedVertices[2].worldPosition.x);
	auto biggestY = std::max(std::max(transformedVertices[0].worldPosition.y, transformedVertices[1].worldPosition.y), transformedVertices[2].worldPosition.y);
	startC = static_cast<uint32_t>(Clamp(roundf(smallestX - 1.f), 0.f, roundf(screenWidth)));
	sizeC = static_cast<uint32_t>(Clamp(roundf(biggestX + 1.f), 0.f, roundf(screenWidth)));
	startR = static_cast<uint32_t>(Clamp(roundf(smallestY - 1.f), 0.f, roundf(ScreenHeight)));
	sizeR = static_cast<uint32_t>(Clamp(roundf(biggestY + 1.f), 0.f, roundf(ScreenHeight)));
}

bool Elite::Mesh::CreatePixelVertex(const std::vector<Vertex>& transformedVertices, const FPoint2& colRow, float& pixelDepth, Vertex& pixelVertex)
{
	float weightValue2{ Cross(colRow - transformedVertices[0].worldPosition.xy, 	transformedVertices[1].worldPosition.xy - transformedVertices[0].worldPosition.xy) };
	float weightValue0{ Cross(colRow - transformedVertices[1].worldPosition.xy, transformedVertices[2].worldPosition.xy - transformedVertices[1].worldPosition.xy) };
	float weightValue1{ Cross(colRow - transformedVertices[2].worldPosition.xy, transformedVertices[0].worldPosition.xy - transformedVertices[2].worldPosition.xy) };
	if (!IsCullingCorrectly(weightValue0, weightValue1,  weightValue2))
	{
		return false;
	}
	//Barycentric Coordinates
	float normalizer = { Cross((transformedVertices[0].worldPosition.xy - transformedVertices[1].worldPosition.xy),(transformedVertices[2].worldPosition.xy - transformedVertices[1].worldPosition.xy)) };
	weightValue0 /= normalizer;
	weightValue2 /= normalizer;
	weightValue1 /= normalizer;
	float depth{ 0.0f };
	//calculating depth
	depth = 1 / (((1 / transformedVertices[0].worldPosition.z) * weightValue0) + ((1 / transformedVertices[1].worldPosition.z) * weightValue1) + ((1 / transformedVertices[2].worldPosition.z) * weightValue2));
	if (depth > pixelDepth)
	{

		return false;
	}

	pixelDepth = depth;

	// w interpolated
	float wInterpolated = 1 / (((1 / transformedVertices[0].worldPosition.w) * weightValue0) + ((1 / transformedVertices[1].worldPosition.w) * weightValue1) + ((1 / transformedVertices[2].worldPosition.w) * weightValue2));
	pixelVertex.uv = ((((transformedVertices[0].uv) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].uv) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].uv) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;
	//normal interpolated
	pixelVertex.normal = ((((transformedVertices[0].normal) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].normal) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].normal) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;
	Normalize(pixelVertex.normal);
	//tangengt  interpolated
	pixelVertex.tangent = ((((transformedVertices[0].tangent) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].tangent) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].tangent) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;
	Normalize(pixelVertex.tangent);

	//vertex to world aka interpolated.
	pixelVertex.worldPosition.x = ((((transformedVertices[0].worldPosition.x) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].worldPosition.x) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].worldPosition.x) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;
	pixelVertex.worldPosition.y = ((((transformedVertices[0].worldPosition.y) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].worldPosition.y) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].worldPosition.y) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;
	pixelVertex.worldPosition.z = ((((transformedVertices[0].worldPosition.z) / transformedVertices[0].worldPosition.w) * weightValue0) + (((transformedVertices[1].worldPosition.z) / transformedVertices[1].worldPosition.w) * weightValue1) + (((transformedVertices[2].worldPosition.z) / transformedVertices[2].worldPosition.w) * weightValue2)) * wInterpolated;;
	return true;
}


void Elite::Mesh::ToggleThroughSamplerStates()
{
	m_pEffect->ToggleThroughSamplerStates();
}

void Elite::Mesh::CleanUp()
{
	if (m_pRasterStateFront)
	{
		m_pRasterStateFront->Release();
		m_pRasterStateFront = nullptr;
	}
	if (m_pRasterStateBack)
	{
		m_pRasterStateBack->Release();
		m_pRasterStateBack = nullptr;
	}
	if (m_pRasterStateNone)
	{
		m_pRasterStateNone->Release();
		m_pRasterStateNone = nullptr;
	}
	m_pEffect->CleanUp();
}

Elite::RGBColor Elite::Mesh::PixelShading(const FPoint3& CameraPos, const Vertex& pixelVertex)
{
	//light direction variables
	const FVector3 lightDirection{ 0.577f, -0.577f, -0.577f };
	const RGBColor lightColor{ 1.f,1.f,1.f };
	const float lightIntensity{ 7.f };
	const float shininess{ 25.f };
	//ambient variable
	const RGBColor ambientColor{ 0.025f,0.025f,0.025f };

	//Making the observed area for the normalmap
	//making the color from 0 to 255 to -1 to 1
	RGBColor ObserverAreaColor{ m_NormalTexture.Sample(pixelVertex.uv) };
	ObserverAreaColor *= 2;
	ObserverAreaColor.r -= 1;
	ObserverAreaColor.g -= 1;
	ObserverAreaColor.b -= 1;

	//making the observer area
	const FVector3 binormal{ Cross(pixelVertex.tangent, pixelVertex.normal) };
	FMatrix3 tangentSpaceAxis{ FMatrix3(pixelVertex.tangent, binormal, pixelVertex.normal) };
	FVector3 newNormal{ GetNormalized(tangentSpaceAxis * FVector3(ObserverAreaColor.r, ObserverAreaColor.g, ObserverAreaColor.b)) };

	float oberverdArea{ Clamp(Dot(newNormal, lightDirection), 0.f, 1.f) };
	oberverdArea /= M_PI;
	//creating variables for pong
	const FVector3 viewDirection{ GetNormalized(CameraPos - pixelVertex.worldPosition.xyz) };


	//calculating total color 
	RGBColor color{ (m_DiffuseTexture.Sample(pixelVertex.uv) + Phong(m_SpecularTexture.Sample(pixelVertex.uv), m_GlossinessTexture.Sample(pixelVertex.uv).r * shininess, lightDirection, viewDirection, pixelVertex.normal) + ambientColor) };

	RGBColor finalColor = lightColor * lightIntensity * color * oberverdArea;
	finalColor.MaxToOne();
	return finalColor;
}

std::vector<Elite::Vertex> Elite::Mesh::GetTriangleVertices(size_t index) const
{
	return {m_Vertices[index],m_Vertices[index+1],m_Vertices[index+2] };
}



size_t Elite::Mesh::GetAmountOfTriangles()
{
	return m_Indices.size();
}

void Elite::Mesh::ToggleRasterState()
{
	int tmp = static_cast<int>(m_CullMode);
	if (tmp < 2)
	{
		tmp++;
	}
	else
	{
		tmp = 0;
	}
	m_CullMode = static_cast<CullModes>(tmp);

	switch (m_CullMode)
	{
	case Elite::CullModes::front:
		std::cout << "Raster State: front \n";
		break;
	case Elite::CullModes::back:
		std::cout << "Raster State: back \n";
		break;
	case Elite::CullModes::none:
		std::cout << "Raster State: none \n";
		break;
	default:
		break;
	}
}


void Elite::Mesh::CreateTangent(std::vector<Vertex>& vertices, const std::vector<int>& indices)
{

	//link: https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		int index0 = indices[i];
		int index1 = indices[i+1];
		int index2 = indices[i+2];
		//posistions
		const FPoint3& p0 = vertices[index0].position;
		const FPoint3& p1 = vertices[index1].position;;
		const FPoint3& p2 = vertices[index2].position;;
		//normals
		const FVector3& uv0 = vertices[index0].uv;
		const FVector3& uv1 = vertices[index1].uv;
		const FVector3& uv2 = vertices[i+2].uv;
		//edges
		const FVector3 edge0 = p1 - p0;
		const FVector3 edge1 = p2 - p0;
		//diff
		const FVector2 diffX = FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const FVector2 diffY = FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Cross(diffX, diffY);
		//tangent
		FVector3 tangent = (edge0 * diffY.y - edge1 * diffX.x) * r;
		vertices[index0].tangent += tangent;
		vertices[index1].tangent += tangent;
		vertices[index2].tangent += tangent;
	}

	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].tangent = GetNormalized(Reject(vertices[i].tangent, vertices[i].normal));
	}

}

void Elite::Mesh::InvertZComponents(std::vector<Vertex>& vertices)
{
	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].tangent.z *= -1.f;
		vertices[i].normal.z *= -1.f ;
		vertices[i].position.z *= -1.f;
		vertices[i].worldPosition.z *= -1.f;	
	}
}

Elite::RGBColor Elite::Mesh::Phong(const RGBColor& specularReflactance, float phongExponent, const FVector3& l, const FVector3& v, const FVector3& dNormal) const
{
	const float spec = Clamp(Dot(Reflect(l, -dNormal), v), 0.f, 1.f);
	return specularReflactance * powf(spec, phongExponent);
}

void Elite::Mesh::SetVerticesWorldPositions(std::vector<Vertex>& vertices)
{
	for (Vertex& ver: vertices)
	{
		ver.worldPosition = ver.position;
		ver.worldPosition.w = ver.position.z;
	}
}


void Elite::Mesh::SetRasterState(ID3D11DeviceContext* pDeviceContext)
{
	switch (m_CullMode)
	{
	case Elite::CullModes::front:
		pDeviceContext->RSSetState(m_pRasterStateFront);
		break;
	case Elite::CullModes::back:
		pDeviceContext->RSSetState(m_pRasterStateBack);
		break;
	case Elite::CullModes::none:
		pDeviceContext->RSSetState(m_pRasterStateNone);
		break;
	default:
		break;
	}
	
}

bool Elite::Mesh::IsCullingCorrectly(float weightVal0, float weightVal1, float weightVal2)
{
	switch (m_CullMode)
	{
	case Elite::CullModes::front:
		if (weightVal2 > 0 || weightVal1 > 0 || weightVal0 > 0)
		{
			return false;
		}
		break;
	case Elite::CullModes::back:
		if (weightVal2 < 0 || weightVal1 < 0 || weightVal0 < 0)
		{
			return false;
		}
		break;
	case Elite::CullModes::none:
		if (!(weightVal2 > 0 && weightVal1 > 0 && weightVal0 > 0) || (weightVal2 < 0 && weightVal1 < 0 && weightVal0 < 0))
		{
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}

void Elite::Mesh::Initialize(ID3D11Device* pDevice, const std::string& fileName)
{
	//vector vars
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	//parse through file and put date in vectors
	ParseOBJ(fileName, vertices, indices);
	SetVerticesWorldPositions(vertices);
	//create tangents through crated vertices and indices
	CreateTangent(vertices, indices);
	//set software vectors
	m_Vertices = vertices;
	m_Indices = indices;
	//invert vertices
	InvertZComponents(vertices);

	m_AmountOfIndices = static_cast<UINT>(indices.size());

	//create vertex layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "Position";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 28;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 40;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "Texcoord";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 52;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


	//create vertec buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;




	//Create the input layour
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetPointerTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout);
	if (FAILED(result))
		return;

	//Create index buffer
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * (uint32_t)indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;

	//creating the raster state
	D3D11_RASTERIZER_DESC rasterizerStateBack;
	rasterizerStateBack.FillMode = D3D11_FILL_SOLID;
	rasterizerStateBack.CullMode = D3D11_CULL_BACK;
	rasterizerStateBack.FrontCounterClockwise = true;
	rasterizerStateBack.DepthBias = false;
	rasterizerStateBack.DepthBiasClamp = 0;
	rasterizerStateBack.SlopeScaledDepthBias = 0;
	rasterizerStateBack.DepthClipEnable = true;
	rasterizerStateBack.ScissorEnable = false;
	rasterizerStateBack.MultisampleEnable = false;
	rasterizerStateBack.AntialiasedLineEnable = false;
	pDevice->CreateRasterizerState(&rasterizerStateBack, &m_pRasterStateBack);

	//creating the raster state
	D3D11_RASTERIZER_DESC rasterizerStateFront;
	rasterizerStateFront.FillMode = D3D11_FILL_SOLID;
	rasterizerStateFront.CullMode = D3D11_CULL_FRONT;
	rasterizerStateFront.FrontCounterClockwise = true;//false
	rasterizerStateFront.DepthBias = false;
	rasterizerStateFront.DepthBiasClamp = 0;
	rasterizerStateFront.SlopeScaledDepthBias = 0;
	rasterizerStateFront.DepthClipEnable = true;
	rasterizerStateFront.ScissorEnable = false;
	rasterizerStateFront.MultisampleEnable = false;
	rasterizerStateFront.AntialiasedLineEnable = false;
	pDevice->CreateRasterizerState(&rasterizerStateFront, &m_pRasterStateFront);

	//creating the raster state
	D3D11_RASTERIZER_DESC rasterizerStateNone;
	rasterizerStateNone.FillMode = D3D11_FILL_SOLID;
	rasterizerStateNone.CullMode = D3D11_CULL_NONE;
	rasterizerStateNone.FrontCounterClockwise = true;
	rasterizerStateNone.DepthBias = false;
	rasterizerStateNone.DepthBiasClamp = 0;
	rasterizerStateNone.SlopeScaledDepthBias = 0;
	rasterizerStateNone.DepthClipEnable = true;
	rasterizerStateNone.ScissorEnable = false;
	rasterizerStateNone.MultisampleEnable = false;
	rasterizerStateNone.AntialiasedLineEnable = false;
	pDevice->CreateRasterizerState(&rasterizerStateNone, &m_pRasterStateNone);
}
