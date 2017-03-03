#ifndef WATER_H
#define WATER_H

#include "Texture.h"
#include "PrioEngineVars.h"
#include <D3DX10math.h>

const int kNumberOfVerticesInWater = 6;

class CWater
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
	bool Initialise(ID3D11Device* device, std::string filename);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);
private:
	bool InitialiseBuffers(ID3D11Device* device);
private:
	unsigned int mNumVertices;
	unsigned int mNumIndices;
	VertexType mpVertices[kNumberOfVerticesInWater];
	unsigned int mpIndices[kNumberOfVerticesInWater];
	CTexture* mpTexture;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
public:
	unsigned int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture() { return mpTexture->GetTexture(); };
private:
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
};

#endif