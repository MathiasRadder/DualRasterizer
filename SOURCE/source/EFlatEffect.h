#pragma once
#include "EBaseEffect.h"


namespace Elite
{

	class FlatEffect final : public BaseEffect
	{
	public:
		FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		~FlatEffect();
		//ID3DX11Effect* GetPointerEffect() const override;
		//ID3DX11EffectTechnique* GetPointerTechnique()const override;
		void UpdateMatrixes(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix) override;
		void UpdateShaderMaps(ID3D11ShaderResourceView* pRescourceDiff, ID3D11ShaderResourceView* pRescourceNorm, ID3D11ShaderResourceView* pRescourceGloss, ID3D11ShaderResourceView* pRescourceSpec) override;
		//void ToggleThroughSamplerStates() override;
		void CleanUp() override;
		void SetStates(ID3D11DeviceContext* pDeviceContext) override;
	private:

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		//states
		ID3D11RasterizerState* m_pRasterState;
		ID3D11BlendState* m_pBlendState;
		ID3D11DepthStencilState* m_pStencilState;
		const FLOAT m_BlendFactor[4];
		const UINT m_SampleMask;
	};
}

