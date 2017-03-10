#ifndef WATERSHADER_H
#define WATERSHADER_H

#include "Shader.h"
#include "Light.h"
#include "Camera.h"

class CWaterShader :
	public CShader
{
public:
	enum RenderState
	{
		Height,
		Surface,
		Reflection,
		Refraction
	};
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
		float WaterPlaneY;
		float waterBufferPadding;
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
		D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale, float WaterPlaneY,
		int screenWidth, int screenHeight, CCamera* camera, CLight* light, ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView* refractionMap,
		ID3D11ShaderResourceView* reflectionMap);

private:
	bool InitialiseShader(ID3D11Device * device, HWND hwnd, std::string waterSurfaceVSFilename, std::string waterSurfacePSFilename, std::string waterBodyVSName, std::string waterHeightPSName, std::string reflectionPSName, std::string refractionPSName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
		D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale, float WaterPlaneY,
		int screenWidth, int screenHeight, CCamera* camera, CLight* light, ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView* refractionMap,
		ID3D11ShaderResourceView* reflectionMap);
	void RenderBuffers(ID3D11DeviceContext * deviceContext, int indexCount);
public:
	bool UpdateWaterBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXVECTOR2 WaterMovement,
						   float refractionStrength, float reflectionStrength, float maxDistortionDistance, float WaveScale, float WaterPlaneY);
	bool UpdateMatrixBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj);
	bool UpdateWaveBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, float waveScale);
	bool UpdateCameraBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXMATRIX CameraWorldMatrix, D3DXVECTOR3 CameraPosition);
	bool UpdateLightBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, CLight* light);
	bool UpdateViewportBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, int screenWidth, int screenHeight);

	void SetHeightMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
	void SetRefractionMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
	void SetReflectionMap(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);

	void SetRenderState(RenderState renderState);

	void SetReflectionTex(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* resource);
private:

	ID3D11VertexShader* mpSurfaceVertexShader;
	ID3D11PixelShader* mpSurfacePixelShader;

	ID3D11VertexShader* mpWaterModelVertexShader;
	ID3D11PixelShader* mpWaterHeightPixelShader;

	ID3D11PixelShader* mpReflectionPixelShader;
	ID3D11PixelShader* mpRefractionPixelShader;


	ID3D11VertexShader* mpCurrentVertexShader;
	ID3D11PixelShader* mpCurrentPixelShader;

	ID3D11InputLayout* mpLayout;
	ID3D11Buffer* mpMatrixBuffer;
	ID3D11Buffer* mpWaveBuffer;
	ID3D11Buffer* mpWaterBuffer;
	ID3D11Buffer* mpCameraBuffer;
	ID3D11Buffer* mpLightBuffer;
	ID3D11Buffer* mpViewportBuffer;

	RenderState mCurrentRenderState;
};

#endif