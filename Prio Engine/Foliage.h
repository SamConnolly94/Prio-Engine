#ifndef FOLIAGE_H
#define FOLIAGE_H

#include <d3d11.h>
#include <d3dx10math.h>
#include "Texture.h"

class CFoliage
{
private:
	CLogger* logger;
public:
	struct FoliageVertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 UV;
		D3DXVECTOR3 normal;
	};
public:
	CFoliage();
	~CFoliage();
public:
	bool Initialise(ID3D11Device* device, FoliageVertexType* vertices, int numberOfVertices, unsigned int* indices, int numberOfIndices);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);
	void Update(float updateTime);
private:
	bool InitialiseBuffers(ID3D11Device* device, FoliageVertexType* vertices, unsigned int* indices);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
public:
	int GetNumberOfIndices();
	ID3D11ShaderResourceView* GetFoliageTexture() { return mpFoliageTex->GetTexture(); };
	ID3D11ShaderResourceView* GetFoliageAlphaTexture() { return mpFoliageAlphaTex->GetTexture(); };
	D3DXVECTOR3 GetTranslation() { return mFoliageTranslation; };
private:
	int mIndexCount;
	int mVertexCount;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	CTexture* mpGrassTexture;
	CTexture* mpGrassAlphaTexture;
	CTexture* mpFoliageTex;
	CTexture* mpFoliageAlphaTex;
	D3DXVECTOR3 mFoliageTranslation;
	D3DXVECTOR3 mWindDirection = { 0.0f, 0.0f, 0.2f };
};

#endif