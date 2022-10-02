#include "pch.h"
#include "EBaseEffect.h"
#include <sstream>



ID3DX11Effect* Elite::BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result = S_OK;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect;

    DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
        {
            char* pErrors = (char*)pErrorBlob->GetBufferPointer();

            std::wstringstream ss;
            for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                ss << pErrors[i];

            OutputDebugStringW(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::wcout << ss.str() << "\n";
        }
        else
        {
            std::wstringstream ss;
            ss << "EffectLoader : Failed to CreateEffectFromFile!\nPath" << assetFile;
            std::wcout << ss.str() << "\n";
            return nullptr;
        }
    }
    return pEffect;

}

Elite::BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile, bool isFlat, SamplerState samplestate):
    m_SampleState{samplestate}
{
    m_pEffect = BaseEffect::LoadEffect(pDevice, assetFile);

    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    if (!m_pMatWorldViewProjVariable->IsValid())

        std::wcout << "m_pMatWorldViewProjVariable not valid \n";
    m_pScalarIsFlatVariable = m_pEffect->GetVariableByName("gIsFlatShade")->AsScalar();
    if (!m_pScalarIsFlatVariable->IsValid())
        std::wcout << "m_pScalarFloatVariable not valid \n";

    m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
    if (!m_pWorldMatrixVariable->IsValid())
        std::wcout << "m_pWorldMatrixVariable not valid \n";
    m_pViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
    if (!m_pViewInverseVariable->IsValid())
        std::wcout << "m_pViewInverseVariable not valid \n";

    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    if (!m_pMatWorldViewProjVariable->IsValid())
        std::wcout << "m_pMatWorldViewProjVariable not valid \n";

    m_pPointTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
    if (!m_pPointTechnique->IsValid())
        std::wcout << "Technique not valid \n";

    m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
    if (!m_pLinearTechnique->IsValid())
        std::wcout << "Technique not valid \n";

    m_pAnisotrpicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
    if (!m_pAnisotrpicTechnique->IsValid())
        std::wcout << "Technique not valid \n";



    m_pScalarIsFlatVariable->SetBool(isFlat);
}

Elite::BaseEffect::~BaseEffect()
{
    
    if (m_pPointTechnique)
    {
        m_pPointTechnique->Release();
        m_pPointTechnique = nullptr;
    }
    if (m_pAnisotrpicTechnique)
    {
        m_pAnisotrpicTechnique->Release();
        m_pAnisotrpicTechnique = nullptr;
    }
    if (m_pLinearTechnique)
    {
        m_pLinearTechnique->Release();
        m_pLinearTechnique = nullptr;
    }
    if (m_pMatWorldViewProjVariable)
    {
        m_pMatWorldViewProjVariable->Release();
        m_pMatWorldViewProjVariable = nullptr;
    }
    if (m_pWorldMatrixVariable)
    {
        m_pWorldMatrixVariable->Release();
        m_pWorldMatrixVariable = nullptr;
    }
    if (m_pViewInverseVariable)
    {
        m_pViewInverseVariable->Release();
        m_pViewInverseVariable = nullptr;
    }

    if (m_pEffect)
    {
        m_pEffect->Release();
        m_pEffect = nullptr;
    }
}

ID3DX11Effect* Elite::BaseEffect::GetPointerConstEffect() const
{
    return m_pEffect;
}
ID3DX11Effect* Elite::BaseEffect::GetPointerEffect()
{
    return m_pEffect;
}

ID3DX11EffectTechnique* Elite::BaseEffect::GetPointerTechnique() const
{
    switch (m_SampleState)
    {
    case Elite::SamplerState::Point:
        return m_pPointTechnique;
    case Elite::SamplerState::Linear:
        return m_pLinearTechnique;;
    case Elite::SamplerState::Anisotropic:
        return m_pAnisotrpicTechnique;
    default:
        return m_pPointTechnique;
    }
}

void Elite::BaseEffect::ToggleThroughSamplerStates()
{
    int tmp = static_cast<int>(m_SampleState);
    if (tmp < 2)
    {
        tmp++;
    }
    else
    {
        tmp = 0;
    }
    m_SampleState = static_cast<SamplerState>(tmp);

    switch (m_SampleState)
    {
    case Elite::SamplerState::Point:
        std::cout << "Sample State: Point \n";
        break;
    case Elite::SamplerState::Linear:
        std::cout << "Sample State: Linear \n";
        break;
    case Elite::SamplerState::Anisotropic:
        std::cout << "Sample State: Anisotropic \n";
        break;
    default:
        break;
    }
}
void Elite::BaseEffect::SetWorldViewProjMatrix(const float* matrix)
{
    m_pMatWorldViewProjVariable->SetMatrix(matrix);
}
void Elite::BaseEffect::SetWorldMatrix(const float* matrix)
{
    m_pWorldMatrixVariable->SetMatrix(matrix);
}

void Elite::BaseEffect::SetViewInverseMatrix(const float* matrix)
{
    m_pViewInverseVariable->SetMatrix(matrix);
}