#ifndef WATERSHADER_H
#define WATERSHADER_H

#include "Shader.h"
#include "Light.h"
#include "Camera.h"

class CWaterShader :
	public CShader
{
private:
	struct WaveBufferType
	{
		float WaveScale;
		D3DXVECTOR3 waveScalePadding;
	};

	struct WaterBufferType
	{
		D3DXVECTOR2 WaterMovement;
		float WaveScale;
		float RefractionStrength;
		float ReflectionStrength;
		float MaxDistortionDistance;
		D3DXVECTOR2 waterBufferPadding;
	};

	struct CameraBufferType
	{
		D3DXMATRIX CameraMatrix;
		D3DXVECTOR3 CameraPos;
		float cameraBufferPadding;
	};

	struct LightBufferType
	{
		D3DXVECTOR3 LightPosition;
		D3DXVECTOR4 AmbientColour;
		D3DXVECTOR4 DiffuseColour;
		D3DXVECTOR3 LightDirection;
		float SpecularPower;
		D3DXVECTOR4 SpecularColor;
		float lightBufferPadding;
	};

	struct ViewportBufferType
	{
		D3DXVECTOR2 ViewportSize;
		D3DXVECTOR2 viewportPadding;
	};

public:
	CWaterShader();
	~CWaterShader();
public:
	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale,
		int screenWidth, int screenHeight, CCamera* camera, CLight* light);

private:
	bool InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale,
		int screenWidth, int screenHeight, CCamera* camera, CLight* light);
	void RenderShader(ID3D11DeviceContext * deviceContext, int indexCount);
public:
	bool UpdateWaterBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXVECTOR2 WaterMovement, float refractionStrength, float reflectionStrength, float maxDistortionDistance, float WaveScale);
	bool UpdateMatrixBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj);
	bool UpdateWaveBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, float waveScale);
	bool UpdateCameraBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXMATRIX CameraWorldMatrix, D3DXVECTOR3 CameraPosition);
	bool UpdateLightBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, CLight* light);
	bool UpdateViewportBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, int screenWidth, int screenHeight);

	void SetHeightMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
	void SetRefractionMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
	void SetReflectionMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpPixelShader;
	ID3D11InputLayout* mpLayout;
	ID3D11SamplerState* mpTrilinearWrapSampleState;
	ID3D11Buffer* mpMatrixBuffer;
	ID3D11Buffer* mpWaveBuffer;
	ID3D11Buffer* mpWaterBuffer;
	ID3D11Buffer* mpCameraBuffer;
	ID3D11Buffer* mpLightBuffer;
	ID3D11Buffer* mpViewportBuffer;
};

#endif