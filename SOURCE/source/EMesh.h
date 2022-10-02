#pragma once
#include "Emath.h"
#include <vector>
#include "EVertex.h"
#include "EFace.h"
#include "ETexture.h"
namespace Elite
{
	class BaseEffect;

	enum class CullModes
	{
		none,
		front,
		back
	};

	class Mesh
	{
	public:
		Mesh(ID3D11Device* pDevice, const std::string& fileName, const std::string& diffTextureFilePath, const std::string& normTextureFilePath, const std::string& glossTextureFilePath, const std::string& specTextureFilePath);
		Mesh(ID3D11Device* pDevice, const std::string& fileName, const std::string& diffTextureFilePath);
		~Mesh();
		void Update(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix);
		void HardwareRender(ID3D11DeviceContext* pDeviceContext);
		void CreateBoundingBox(const std::vector<Vertex>& transformedVertices, float screenWidth, float ScreenHeight, size_t& startC, size_t& startR, size_t& sizeC, size_t& sizeR);
		bool CreatePixelVertex(const std::vector<Vertex>& transformedVertices, const FPoint2& colRow, float& pixelDepth, Vertex& pixelVertex);
		void ToggleThroughSamplerStates();
		void CleanUp();
		RGBColor PixelShading(const FPoint3& CameraPos, const Vertex& pixelVertex);
		std::vector<Vertex> GetTriangleVertices(size_t index) const;
		size_t GetAmountOfTriangles();
		void ToggleRasterState();
		void SetRasterState(ID3D11DeviceContext* pDeviceContext);
	private:
		bool IsCullingCorrectly(float weightVal0, float weightVal1, float weightVal2);
		void Initialize(ID3D11Device* pDevice, const std::string& fileName);
		void CreateTangent(std::vector<Vertex>& vertices, const std::vector<int>& indices);
		void InvertZComponents(std::vector<Vertex>& vertices);
		RGBColor Phong(const RGBColor& specularReflactance, float phongExponent, const FVector3& l, const FVector3& v, const FVector3& dNormal) const;
		void SetVerticesWorldPositions(std::vector<Vertex>& vertices);

		//hardware variables
		BaseEffect* m_pEffect;
		ID3D11InputLayout* m_pVertexLayout;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		UINT m_AmountOfIndices;
		//textures
		Texture m_DiffuseTexture;
		Texture m_NormalTexture;
		Texture m_SpecularTexture;
		Texture m_GlossinessTexture;
		//software variables
		std::vector<Vertex> m_Vertices;
		std::vector<int> m_Indices;
		//raster states
		ID3D11RasterizerState* m_pRasterStateBack;
		ID3D11RasterizerState* m_pRasterStateFront;
		ID3D11RasterizerState* m_pRasterStateNone;
		CullModes m_CullMode;
	};
}

