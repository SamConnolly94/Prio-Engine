#ifndef FOLIAGESHADER_H
#define FOLIAGESHADER_H

#include "Shader.h"
class CFoliageShader : public CShader
{
private:
	struct LightBufferType
	{
		D3DXVECTOR4 AmbientColour;
		D3DXVECTOR4 DiffuseColour;
		D3DXVECTOR3 LightDirection;
		float padding;
	};
public:
	CFoliageShader();
	~CFoliageShader();
public:
	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, ID3D11ShaderResourceView * grassTexture, ID3D11ShaderResourceView * alphaMask, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection);

private:
	bool InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView * grassTexture, ID3D11ShaderResourceView * alphaMask, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection);
	void RenderShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11SamplerState* mpSampleState;
	ID3D11Buffer* mpLightBuffer;
};

#endif