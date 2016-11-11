#ifndef TERRAIN_H
#define TERRAIN_H

#include <d3d11.h>
#include <d3dx10math.h>
#include "Engine\PrioEngineVars.h"

class CTerrainGrid
{
public:
	CTerrainGrid();
	~CTerrainGrid();
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 colour;
	};
public:
	bool Initialise(ID3D11Device* device);
	void Render(ID3D11DeviceContext* context);

private:
	bool InitialiseBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* context);
private:
	int mWidth;
	int mHeight;
	int mVertexCount;
	int mIndexCount;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
// Getters.
public:
	int GetVertexCount() { return mVertexCount; };
	int GetIndexCount() { return mIndexCount; };
};

#endif