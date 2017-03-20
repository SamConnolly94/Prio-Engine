#ifndef REFRACTIONSHADER_H
#define REFRACTIONSHADER_H

#include "Shader.h"
class CRefractionShader :
	public CShader
{
private:
	struct ViewportBufferType
	{
		D3DXVECTOR2 ViewportSize;
		D3DXVECTOR4 viewportPadding1;
		D3DXVECTOR2 viewportPadding2;
	};

	struct LightBufferType
	{
		D3DXVECTOR4 AmbientColour;
		D3DXVECTOR4 DiffuseColour;
		D3DXVECTOR3 LightDirection;
		float padding;
	};

public:
	CRefractionShader();
	~CRefractionShader();
public:
	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool RefractionRender(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour,
		D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView* waterHeightMap, ID3D11ShaderResourceView* refractionTex);
	bool ReflectionRender(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour,
		D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView* waterHeightMap, ID3D11ShaderResourceView* refractionTex);
private:
	bool InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename, std::string reflectionPSFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour,
		D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView* waterHeightMap, ID3D11ShaderResourceView* refractionTex);

	void CRefractionShader::RenderRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount);
	void CRefractionShader::RenderReflectionShader(ID3D11DeviceContext * deviceContext, int indexCount);

private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpRefractionPixelShader;
	ID3D11PixelShader* mpReflectionPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11SamplerState* mpTrilinearWrap;
	ID3D11SamplerState* mpBilinearMirror;
	ID3D11Buffer* mpMatrixBuffer;
	ID3D11Buffer* mpLightBuffer;
	ID3D11Buffer* mpViewportBuffer;
};

#endif