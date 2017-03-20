#ifndef WATERSHADER_H
#define WATERSHADER_H

#include "Shader.h"

class CWaterShader :
	public CShader
{
// Constant buffer structs
private:
	// Vertex shader and surface pixel shader buffer reg 1.
	struct WaterBufferType
	{
		D3DXVECTOR4 WaterSize;
		D3DXVECTOR4 WaterSpeed;
		D3DXVECTOR2 WaterTranslation;
		float WaveHeight;
		float WaveScale;
		float RefractionDistortion;
		float ReflectionDistortion;
		float MaxDistortionDistance;
		float RefractionStrength;
		float ReflectionStrength;
		D3DXVECTOR3 waterPadding;
	};

	// surface pixel shader reg 2.
	struct CameraBufferType
	{
		D3DXMATRIX CameraMatrix;
		D3DXVECTOR3	CameraPosition;
		float cameraPadding;
	};

	// Surface pixel shader reg 3.
	struct ViewportBufferType
	{
		D3DXVECTOR2 ViewportSize;
		D3DXVECTOR4 viewportPadding1;
		D3DXVECTOR2 viewportPadding2;
	};

	// Surface pixel shader reg 4
	struct LightBufferType
	{
		D3DXVECTOR4	AmbientColour;
		D3DXVECTOR4	DiffuseColour;
		D3DXVECTOR3	LightDirection;
		float lightBufferPadding;
	};
public:
	CWaterShader();
	~CWaterShader();

	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool RenderSurface(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix, 
		D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion, float maxDistortion,
		float refractionStrength, float reflectionStrength,	D3DXMATRIX cameraMatrix, D3DXVECTOR3 cameraPosition, D3DXVECTOR2 viewportSize, D3DXVECTOR4 ambientColour, 
		D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* normalHeightMap, ID3D11ShaderResourceView* refractionMap, ID3D11ShaderResourceView* reflectionMap);

	bool RenderHeight(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
		D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion, 
		float maxDistortion, float refractionStrength, float reflectionStrength, ID3D11ShaderResourceView* normalHeightMap);
private:
	bool InitialiseShader(ID3D11Device* device, HWND hwnd, std::string vsFilename, std::string surfacePsFilename, std::string heightPsFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetSurfaceShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
		D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion, float maxDistortion,
		float refractionStrength, float reflectionStrength, D3DXMATRIX cameraMatrix, D3DXVECTOR3 cameraPosition, D3DXVECTOR2 viewportSize, D3DXVECTOR4 ambientColour,
		D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* normalHeightMap, ID3D11ShaderResourceView* refractionMap, ID3D11ShaderResourceView* reflectionMap);
	bool SetHeightShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
		D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion,
		float maxDistortion, float refractionStrength, float reflectionStrength, ID3D11ShaderResourceView* normalHeightMap);
	void RenderSurfaceShader(ID3D11DeviceContext* deviceContext, int indexCount);
	void RenderHeightShader(ID3D11DeviceContext* deviceContext, int indexCount);

private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpSurfacePixelShader;
	ID3D11PixelShader* mpHeightPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11SamplerState* mpTrilinearWrap;
	ID3D11SamplerState* mpBilinearMirror;

	ID3D11Buffer* mpMatrixBuffer;
	ID3D11Buffer* mpWaterBuffer;
	ID3D11Buffer* mpCameraBuffer;
	ID3D11Buffer* mpViewportBuffer;
	ID3D11Buffer* mpLightBuffer;

};

#endif