#include "pch.h"
#include "EEffect.h"

Elite::Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile):
    m_BlendFactor{  0.f,0.f,0.f,0.f  },
    m_SampleMask{ 0xFFFFFFFF },
    BaseEffect::BaseEffect(pDevice, assetFile,false,SamplerState::Point)
{
  
    m_pDiffuseMapVariable = GetPointerEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << "m_pDiffuseMapVariable not valid \n";

    m_pNormalMapVariable = GetPointerEffect()->GetVariableByName("gNormalMap")->AsShaderResource();
    if (!m_pNormalMapVariable->IsValid())
        std::wcout << "m_pNormalMapVariable not valid \n";

    m_pGlossinessMapVariable = GetPointerEffect()->GetVariableByName("gGlossinessMap")->AsShaderResource();
    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << "m_pGlossinessMapVariable not valid \n";

    m_pSpecularMapVariable = GetPointerEffect()->GetVariableByName("gSpecularMap")->AsShaderResource();
    if (!m_pSpecularMapVariable->IsValid())
        std::wcout << "m_pSpecularMapVariable not valid \n";



   //creating the blending state
  D3D11_BLEND_DESC BlendStateDesc;
  BlendStateDesc.AlphaToCoverageEnable = false;
  BlendStateDesc.IndependentBlendEnable = false;
  BlendStateDesc.RenderTarget->BlendEnable = false;

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
  DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
  DepthStencilDesc.StencilEnable = true;

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

Elite::Effect::~Effect()
{
    if (m_pSpecularMapVariable)
    {
        m_pSpecularMapVariable->Release();
        m_pSpecularMapVariable = nullptr;
    }
    if (m_pDiffuseMapVariable)
    {
        m_pDiffuseMapVariable->Release();
        m_pDiffuseMapVariable = nullptr;
    }
    if (m_pNormalMapVariable)
    {
        m_pNormalMapVariable->Release();
        m_pNormalMapVariable = nullptr;
    }
    if (m_pGlossinessMapVariable)
    {
        m_pGlossinessMapVariable->Release();
        m_pGlossinessMapVariable = nullptr;
    }
 
  
}




void Elite::Effect::UpdateMatrixes(const FMatrix4& worldViewprojMatrix, const FMatrix4& worldMatrix, const FMatrix4& viewInverseMatrix)
{
    FMatrix4 tmpMatrix = worldViewprojMatrix;
    SetWorldViewProjMatrix(reinterpret_cast<float*>(&tmpMatrix));
    tmpMatrix = worldMatrix;
    SetWorldMatrix(reinterpret_cast<float*>(&tmpMatrix));
    tmpMatrix = viewInverseMatrix;
    SetViewInverseMatrix(reinterpret_cast<float*>(&tmpMatrix));

}

void Elite::Effect::UpdateShaderMaps(ID3D11ShaderResourceView* pRescourceDiff, ID3D11ShaderResourceView* pRescourceNorm,
    ID3D11ShaderResourceView* pRescourceGloss, ID3D11ShaderResourceView* pRescourceSpec)
{
    SetDiffuseMap(pRescourceDiff);
    SetNormalMap(pRescourceNorm);
    SetGlossinessMap(pRescourceGloss);
    SetSpecularMap(pRescourceSpec);
}


void Elite::Effect::SetDiffuseMap(ID3D11ShaderResourceView* pRescourceView)
{
    if (m_pDiffuseMapVariable->IsValid())
        m_pDiffuseMapVariable->SetResource(pRescourceView);
 
}

void Elite::Effect::SetNormalMap(ID3D11ShaderResourceView* pRescourceView)
{
    if (m_pNormalMapVariable->IsValid())
        m_pNormalMapVariable->SetResource(pRescourceView);
}

void Elite::Effect::SetGlossinessMap(ID3D11ShaderResourceView* pRescourceView)
{
    if (m_pGlossinessMapVariable->IsValid())
        m_pGlossinessMapVariable->SetResource(pRescourceView);
}

void Elite::Effect::SetSpecularMap(ID3D11ShaderResourceView* pRescourceView)
{
    if (m_pSpecularMapVariable->IsValid())
        m_pSpecularMapVariable->SetResource(pRescourceView);
}

void Elite::Effect::CleanUp()
{
    
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

void Elite::Effect::SetStates(ID3D11DeviceContext* pDeviceContext)
{

    pDeviceContext->OMSetDepthStencilState(m_pStencilState, 0);
    pDeviceContext->OMSetBlendState(m_pBlendState, m_BlendFactor, m_SampleMask);
}




