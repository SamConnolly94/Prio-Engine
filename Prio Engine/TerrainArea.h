#ifndef TERRAINAREA_H
#define TERRAINAREA_H

#include "Engine\PrioEngineVars.h"
#include "TerrainTile.h"
#include <vector>
#include <d3d11.h>
#include <d3dx10math.h>

class CTerrainArea
{
public:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 colour;
	};
public:
	CTerrainArea();
	~CTerrainArea();
private:
	std::vector<CTerrainTile> mpTiles;
	int mNumberOfVertices;
	int mNumberOfIndices;
	D3DXVECTOR4 mColour;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
public:
	void AddTile(CTerrainTile tile);
	void SetColour(D3DXVECTOR4 colour);
	void ClearAllTiles();
	bool EraseTile(CTerrainTile tile);
	bool SetBuffers(ID3D11Device* device);
	void Render(ID3D11DeviceContext * context);
	void Shutdown();
	int GetNumberOfIndices() { return mNumberOfIndices; };
private:
	void RenderBuffers(ID3D11DeviceContext * context);
};

#endif