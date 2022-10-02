#include "pch.h"
#include "EFlatEffect.h"

Elite::FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile) :
    m_BlendFactor{ 0.f,0.f,0.f,0.f },
    m_SampleMask{ 0xFFFFFFFF },
    BaseEffect::BaseEffect(pDevice, assetFile, true, SamplerState::Point),
    m_pRasterState(nullptr)

{
  

    m_pDiffuseMapVariable = GetPointerEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << "m_pDiffuseMapVariable not valid \n";


    //ceating blend state
    D3D11_BLEND_DESC BlendStateDesc;
    BlendStateDesc.AlphaToCoverageEnable = false;
    BlendStateDesc.IndependentBlendEnable = false;
    BlendStateDesc.RenderTarget->BlendEnable = true;

    BlendStateDesc.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendStateDesc.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendStateDesc.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
    BlendStateDesc.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendStateDesc.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;

    BlendStateDesc.RenderTarget->RenderTargetWriteMask = 0x0F;
    pDevice->CreateBlendState(&BlendStateDesc, &m_pBlendState);

    //create Stencil state
    D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
    DepthStencilDesc.DepthEnable = true;
    DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    DepthStencilDesc.StencilEnable = false;

    DepthStencilDesc.StencilReadMask = 0x0F;
    DepthStencilDesc.StencilWriteMask = 0x0F;

    DepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    DepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

    DepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

    DepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    pDevice->CreateDepthStencilState(&DepthStencilDesc, &m_pStencilState);


}
    

Elite::FlatEffect::~FlatEffect()
{
    if (m_pDiffuseMapVariable)
    {
        m_pDiffuseMapVariable->Release();
        m_pDiffuseMapVariable = nullptr;
    }
  
}



void Elite::FlatEffect::UpdateMatrixes(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix)
{
    FMatrix4 tmpMatrix = worldViewprojMatrix;
    SetWorldViewProjMatrix(reinterpret_cast<float*>(&tmpMatrix));
    tmpMatrix = worldMatrix;
    SetWorldMatrix(reinterpret_cast<float*>(&tmpMatrix));
    tmpMatrix = viewInverseMatrix;
    SetViewInverseMatrix(reinterpret_cast<float*>(&tmpMatrix));

}

void Elite::FlatEffect::UpdateShaderMaps(ID3D11ShaderResourceView* pRescourceDiff, ID3D11ShaderResourceView* , ID3D11ShaderResourceView* , ID3D11ShaderResourceView* )
{
    if (m_pDiffuseMapVariable->IsValid())
        m_pDiffuseMapVariable->SetResource(pRescourceDiff);
}


void Elite::FlatEffect::CleanUp()
{
    if (m_pRasterState)
    {
        m_pRasterState->Release();
        m_pRasterState = nullptr;
    }

    if (m_pBlendState)
    {
        m_pBlendState->Release();
        m_pBlendState = nullptr;
    }
    if (m_pStencilState)
    {
        m_pStencilState->Release();
        m_pStencilState = nullptr;
    }
}


void Elite::FlatEffect::SetStates(ID3D11DeviceContext* pDeviceContext)
{
    pDeviceContext->OMSetDepthStencilState(m_pStencilState, 0);
    pDeviceContext->OMSetBlendState(m_pBlendState,m_BlendFactor,m_SampleMask);
}