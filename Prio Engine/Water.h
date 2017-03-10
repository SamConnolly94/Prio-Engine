#ifndef WATER_H
#define WATER_H

#include "Texture.h"
#include "PrioEngineVars.h"
#include <D3DX10math.h>
#include <D3DX11.h>
#include "WaterShader.h"

class CWater : public CModelControl
{
private:
	CLogger* logger;
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texCoord;
		D3DXVECTOR3 normal;
	};
public:
	CWater();
	~CWater();
	bool Initialise(ID3D11Device* device, D3DXVECTOR3 minPoint, D3DXVECTOR3 maxPoint, int screenWidth, int screenHeight, unsigned int subDivisionX, unsigned int subDivisionZ, std::string normalMap, bool useNormals = true, bool useUV = true);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

private:
	unsigned int mNumVertices;
	unsigned int mNumIndices;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	CTexture* mpNormalHeightMap;
	D3DXVECTOR2 waterPos;

	ID3D11ShaderResourceView* mpReflectionResource;
	ID3D11Texture2D* mpReflectionMap;
	ID3D11RenderTargetView* mpReflectionTarget;

	ID3D11ShaderResourceView* mpRefractionResource;
	ID3D11Texture2D* mpRefractionMap;
	ID3D11RenderTargetView* mpRefractionTarget;

	ID3D11ShaderResourceView* mpHeightMapResource;
	ID3D11Texture2D* mpHeightMap;
	ID3D11RenderTargetView* mpHeightMapTarget;

	void RenderBuffers(ID3D11DeviceContext* deviceContext);
public:
	unsigned int GetNumberOfIndices();
	CTexture* GetNormalHeightMap();
	void SetWaterHeightRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
	void SetRefractionRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
	void SetReflectionRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView);
private:
	void Update(float updateTime);
	bool InitialiseBuffers(ID3D11Device* device, D3DXVECTOR3 minPoint, D3DXVECTOR3 maxPoint, unsigned int subDivisionX, unsigned int subDivisionZ, bool useNormals, bool useUv);
	bool CreateTextureResources(ID3D11Device* device, int screenWidth, int screenHeight);
	// Water vars for shader.
private:
	D3DXVECTOR2 mMovement;
	float mDistortionDistance;
	D3DXVECTOR2 mDistortion;
	D3DXVECTOR2 mStrength;
	float mWaveScale;

	int mWidth;
	int mHeight;
public:
	D3DXVECTOR2 GetMovement() { return mMovement; };
	float GetDistortionDistance() { return mDistortionDistance; };
	D3DXVECTOR2 GetDistortionValue() { return mDistortion; };
	D3DXVECTOR2 GetStrength() { return mStrength; };
	float GetWaveScale() { return mWaveScale; };
	int GetWidth() { return mWidth; };
	int GetHeight() { return mHeight; };

	ID3D11ShaderResourceView* GetHeightMap();
	ID3D11ShaderResourceView* GetRefractionMap();
	ID3D11ShaderResourceView* GetReflectionMap();
};

#endif