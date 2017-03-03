#include "RenderTexture.h"



CRenderTexture::CRenderTexture()
{
}


CRenderTexture::~CRenderTexture()
{
}

bool CRenderTexture::Initialise(ID3D11Device * device, int width, int height)
{
	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	HRESULT result;

	ZeroMemory(&texDesc, sizeof(texDesc));

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&texDesc, NULL, &mpRenderTargetTexture);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the render target texture from the texture description provided in rendertexture class.");
		return false;
	}

	// Copy the same format as what we used for tex desc.
	renderTargetViewDesc.Format = texDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(mpRenderTargetTexture, &renderTargetViewDesc, &mpRenderTargetView);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the render target view from the render target view desc provided in rendertexture class.");
		return false;
	}

	shaderResourceViewDesc.Format = texDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(mpRenderTargetTexture, &shaderResourceViewDesc, &mpShaderResourceView);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the shader resource view from the desc provided in render texture class.");
		return false;
	}
	
	logger->GetInstance().WriteLine("Successfully initialised render texture.");

	return true;
}

void CRenderTexture::Shutdown()
{
	if (mpShaderResourceView)
	{
		mpShaderResourceView->Release();
		mpShaderResourceView = nullptr;
	}

	if (mpRenderTargetView)
	{
		mpRenderTargetView->Release();
		mpRenderTargetView = nullptr;
	}

	if (mpRenderTargetTexture)
	{
		mpRenderTargetTexture->Release();
		mpRenderTargetView = nullptr;
	}
}

void CRenderTexture::SetRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView)
{
	deviceContext->OMSetRenderTargets(1, &mpRenderTargetView, depthStencilView);
}

void CRenderTexture::ClearRenderTarget(ID3D11DeviceContext * deviceContext, ID3D11DepthStencilView * depthStencilView, float red, float green, float blue, float alpha)
{
	float colour[4];

	colour[0] = red;
	colour[1] = green;
	colour[2] = blue;
	colour[3] = alpha;

	deviceContext->ClearRenderTargetView(mpRenderTargetView, colour);

	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

ID3D11ShaderResourceView * CRenderTexture::GetShaderResourceView()
{
	return mpShaderResourceView;
}
