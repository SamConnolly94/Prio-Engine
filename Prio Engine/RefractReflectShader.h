#ifndef REFRACTIONSHADER_H
#define REFRACTIONSHADER_H

#include "Shader.h"
#include "Light.h"
class CReflectRefractShader :
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
		D3DXVECTOR4	AmbientColour;
		D3DXVECTOR4	DiffuseColour;
		D3DXVECTOR3	LightDirection;
		D3DXVECTOR4 mSpecularColour;
		D3DXVECTOR3 mLightPosition;
		float mSpecularPower;
		float lightBufferPadding;
	};

	struct TerrainAreaBufferType
	{
		float snowHeight;
		float grassHeight;
		float dirtHeight;
		float sandHeight;
	};

	struct PositioningBufferType
	{
		float yOffset;
		float WaterPlaneY;
		D3DXVECTOR2 posPadding;
		D3DXVECTOR4 posPadding2;
	};

	struct FoliageBufferType
	{
		D3DXVECTOR3 WindDirection;
		float FrameTime;
		D3DXVECTOR3 FoliageTranslation;
		float WindStrength;
	};
public:
	CReflectRefractShader();
	~CReflectRefractShader();
public:
	bool Initialise(ID3D11Device* device, HWND hwnd);
	void Shutdown();
	bool RefractionRender(ID3D11DeviceContext* deviceContext, int indexCount);
	bool ReflectionRender(ID3D11DeviceContext* deviceContext, int indexCount);

	bool RenderFoliageRefraction(ID3D11DeviceContext* deviceContext, int indexCount);
private:
	D3DXMATRIX mWorldMatrix; 
	D3DXMATRIX mViewMatrix;
	D3DXMATRIX mProjMatrix; 
	D3DXVECTOR4 mAmbientColour;
	D3DXVECTOR4 mDiffuseColour;
	D3DXVECTOR3 mLightDirection;
	float mSpecularPower;
	D3DXVECTOR4 mSpecularColour;
	D3DXVECTOR3 mLightPosition;
	D3DXVECTOR2 mViewportSize; 
	float mSnowHeight; 
	float mGrassHeight; 
	float mDirtHeight; 
	float mSandHeight; 
	float mTerrainYOffset;
	float mWaterPlaneYOffset;
	// Don't manage memory for these, we should never allocate it in the class to the texture resoureces, so deallocate / release it outside too.
	ID3D11ShaderResourceView* mpWaterHeightMap; 
	ID3D11ShaderResourceView* mpDirtTexArray[2];
	ID3D11ShaderResourceView* mpGrassTextures[2];
	ID3D11ShaderResourceView* mpPatchMap;
	ID3D11ShaderResourceView* mpRockTextures[2];
public:
	void SetLightProperties(CLight* light);
	void SetViewportProperties(int screenWidth, int screenHeight);
	void SetTerrainAreaProperties(float snowHeight, float grassHeight, float dirtHeight, float sandHeight);
	void SetPositioningProperties(float terrainPositionY, float waterPlanePositionY);
	void SetWaterHeightmap(ID3D11ShaderResourceView* waterHeightMap);
	void SetDirtTextureArray(CTexture** dirtTexArray);
	void SetGrassTextureArray(CTexture** grassTexArray);
	void SetPatchMap(CTexture* patchMap);
	void SetRockTexture(CTexture** rockTexArray);
private:
	bool InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename, std::string reflectionPSFilename, std::string modelReflectionPSName, std::string foliageRefractionVSName, std::string foliageRefractionPSName);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext);
	bool SetFoliageShaderParameters(ID3D11DeviceContext* deviceContext);


	void RenderRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount);
	void RenderReflectionShader(ID3D11DeviceContext * deviceContext, int indexCount);
	void RenderFoliageRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount);

private:
	ID3D11VertexShader* mpVertexShader;
	ID3D11PixelShader* mpRefractionPixelShader;
	ID3D11PixelShader* mpReflectionPixelShader;
	ID3D11PixelShader* mpModelReflectionPixelShader;

	ID3D11VertexShader* mpFoliageVertexShader;
	ID3D11PixelShader* mpFoliageRefractionPixelShader;

	ID3D11InputLayout* mpLayout;
	ID3D11SamplerState* mpTrilinearWrap;
	ID3D11SamplerState* mpPointClamp;
	ID3D11SamplerState* mpBilinearMirror;
	ID3D11Buffer* mpLightBuffer;
	ID3D11Buffer* mpViewportBuffer;
	ID3D11Buffer* mpTerrainAreaBuffer;
	ID3D11Buffer* mpPositioningBuffer;
	ID3D11Buffer* mpFoliageBuffer;

public:
	void SetGrassTexture(ID3D11ShaderResourceView * grassTexture);
	void SetGrassAlphaTexture(ID3D11ShaderResourceView * alphaTexture);
	void SetWindDirection(D3DXVECTOR3 direction);
	void SetFrameTime(float frameTime);
	void SetWindStrength(float strength);
	void SetTranslation(D3DXVECTOR3 translation);
public:
	ID3D11ShaderResourceView * mpGrassTexture;
	ID3D11ShaderResourceView * mpGrassAlphaTexture;
	D3DXVECTOR3 mWindDirection;
	float mFrameTime;
	float mStrength;
	D3DXVECTOR3 mTranslation;
};

#endif