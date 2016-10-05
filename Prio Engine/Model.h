#ifndef MODEL_H
#define MODEL_H

// Include DirectX libraries.
#include <d3d11.h>
#include <D3DX10math.h>
#include "PrioTypes.h"

#include "Logger.h"
#include "Texture.h"

class CModel
{
private:
	// Structure types, should reflect the vertex / pixel shader.
	struct VertexColourType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 colour;
	};
	struct VertexTextureType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};
	//struct VertexDiffuseLightingType
	//{
	//	D3DXVECTOR3 position;
	//	D3DXVECTOR2 texture;
	//	D3DXVECTOR3 normal;
	//};
	struct VertexDiffuseLightingType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};

public:
	CModel(WCHAR* textureFilename);
	CModel(WCHAR* textureFilename, bool useLighting);
	CModel(float3 colour);
	~CModel();
public:
	bool Initialise(ID3D11Device* device);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndex();

	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitialiseBuffers(ID3D11Device* device, bool applyTexture);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	bool LoadTexture(ID3D11Device* device);
	void ReleaseTexture();
private:
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	int mVertexCount;
	int mIndexCount;
	CTexture* mpTexture;
	WCHAR* mpTextureFilename;

	CLogger* mpLogger;

	float3 mColour;
	void ResetColour();

	bool mUseDiffuseLighting;
public:
	bool HasTexture();
	bool HasColour();
	bool UseDiffuseLight();
};

#endif