#pragma once

namespace Elite
{
	enum class SamplerState
	{
		Point,
		Linear,
		Anisotropic
	};


	class BaseEffect
	{
	public:
		BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile, bool isFlat, SamplerState samplestate);
		virtual ~BaseEffect();
		ID3DX11Effect* GetPointerConstEffect() const ;
		ID3DX11EffectTechnique* GetPointerTechnique()const;
		virtual void UpdateMatrixes(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix) = 0;
		virtual void UpdateShaderMaps(ID3D11ShaderResourceView* pRescourceDiff, ID3D11ShaderResourceView* pRescourceNorm, ID3D11ShaderResourceView* pRescourceGloss, ID3D11ShaderResourceView* pRescourceSpec) = 0;
		void ToggleThroughSamplerStates();
		virtual void CleanUp() = 0;
		virtual void SetStates(ID3D11DeviceContext* pDeviceContext) = 0;
		
	protected:
		ID3DX11Effect* GetPointerEffect();
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetWorldViewProjMatrix(const float* matrix);
		void SetWorldMatrix(const float* matrix);

		void SetViewInverseMatrix(const float* matrix);

	private:
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pPointTechnique;
		ID3DX11EffectTechnique* m_pLinearTechnique;
		ID3DX11EffectTechnique* m_pAnisotrpicTechnique;
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
		ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable;
		ID3DX11EffectMatrixVariable* m_pViewInverseVariable;
		ID3DX11EffectScalarVariable* m_pScalarIsFlatVariable;
		SamplerState m_SampleState;

	};

}