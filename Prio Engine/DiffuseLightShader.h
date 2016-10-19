#ifndef DIFFUSELIGHTSHADER_H
#define DIFFUSELIGHTSHADER_H

#include <d3d11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>
#include "Logger.h"

class CDiffuseLightShader
{
private:
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 diffuseColour;
		D3DXVECTOR3 lightDirection;
		float padding;
	};

public:
	CDiffuseLightShader();
	~CDiffuseLightShader();

	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColour);

private:
	bool InitialiseShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR3 lightDirection, D3DXVECTOR4 diffuseColour);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11Buffer* mpMatrixBuffer;
	ID3D11SamplerState* mpSampleState;
	ID3D11Buffer* mpLightBuffer;

	CLogger* mpLogger;
};

#endif