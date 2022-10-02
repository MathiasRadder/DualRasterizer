#pragma once
#include "EBaseEffect.h"
namespace Elite
{

	class Effect final : public BaseEffect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~Effect();

		void UpdateMatrixes(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix) override;
		void UpdateShaderMaps(ID3D11ShaderResourceView* pRescourceDiff, ID3D11ShaderResourceView* pRescourceNorm, ID3D11ShaderResourceView* pRescourceGloss, ID3D11ShaderResourceView* pRescourceSpec) override;

	private:
		void SetDiffuseMap(ID3D11ShaderResourceView* pRescourceView);

		void SetNormalMap(ID3D11ShaderResourceView* pRescourceView);

		void SetGlossinessMap(ID3D11ShaderResourceView* pRescourceView);

		void SetSpecularMap(ID3D11ShaderResourceView* pRescourceView);
		void CleanUp() override;
		void SetStates(ID3D11DeviceContext* pDeviceContext) override;
		
		//maps
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		//states
		ID3D11BlendState* m_pBlendState;
		ID3D11DepthStencilState* m_pStencilState;
		const FLOAT m_BlendFactor[4];
		const UINT m_SampleMask;
	};
}

