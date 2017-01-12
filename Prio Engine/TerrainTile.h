#ifndef TERRAINTILE_H
#define TERRAINTILE_H

#include "Engine\PrioEngineVars.h"

#include <d3d11.h>
#include <d3dx10math.h>

class CTerrainTile
{
public:
	enum TerrainType
	{
		Grass,
		Snow
	};
public:
	CTerrainTile();
	~CTerrainTile();
// Leaving these public makes life so much easier. Bad practice, but so much time / code saved in the long run of things, instead of getters and setters.
public:
	TerrainType terrainType;
	D3DXVECTOR3 mVertices[3];
public:
	void Render(ID3D11DeviceContext * context);
	bool UpdateBuffers(ID3D11Device* device);
	bool Compare(CTerrainTile tile);
private:
	void RenderBuffers(ID3D11DeviceContext * context);
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
};

#endif