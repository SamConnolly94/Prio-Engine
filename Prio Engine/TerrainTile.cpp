#include "TerrainTile.h"



CTerrainTile::CTerrainTile()
{	
	mCentrePosition 			= { 0.0f, 0.0f, 0.0f };
	mLowerLeftVertexPosition	= { 0.0f, 0.0f, 0.0f };
	mLowerRightVertexPosition	= { 0.0f, 0.0f, 0.0f };
	mUpperLeftVertexPosition	= { 0.0f, 0.0f, 0.0f };
	mUpperRightVertexPosition	= { 0.0f, 0.0f, 0.0f };

	// Pointers to the surrounding tiles.
	mpLeft		= nullptr;
	mpRight		= nullptr;
	mpUp		= nullptr;
	mpDown		= nullptr;
}


CTerrainTile::~CTerrainTile()
{
}

void CTerrainTile::SetCentrePosition(D3DXVECTOR3 position)
{
	mCentrePosition = position;
}

void CTerrainTile::SetLeftTile(CTerrainTile * tile)
{
	mpLeft = tile;
}

void CTerrainTile::SetRightTile(CTerrainTile * tile)
{
	mpRight = tile;
}

void CTerrainTile::SetUpTile(CTerrainTile * tile)
{
	mpUp = tile;
}

void CTerrainTile::SetDownTile(CTerrainTile * tile)
{
	mpDown = tile;
}

void CTerrainTile::SetTileType(TileType tileType)
{
	mTileType = tileType;
}

void CTerrainTile::SetLowerLeftVertexPosition(D3DXVECTOR3 position)
{
	mLowerLeftVertexPosition = position;
}

void CTerrainTile::SetLowerRightVertexPosition(D3DXVECTOR3 position)
{
	mLowerRightVertexPosition = position;
}

void CTerrainTile::SetUpperLeftVertexPosition(D3DXVECTOR3 position)
{
	mUpperLeftVertexPosition = position;
}

void CTerrainTile::SetUpperRightVertexPosition(D3DXVECTOR3 position)
{
	mUpperRightVertexPosition = position;
}

D3DXVECTOR3 CTerrainTile::GetCentrePosition()
{
	return mCentrePosition;
}

CTerrainTile * CTerrainTile::GetLeftTile()
{
	return mpLeft;
}

CTerrainTile * CTerrainTile::GetRightTile()
{
	return mpRight;
}

CTerrainTile * CTerrainTile::GetUpTile()
{
	return mpUp;
}

CTerrainTile * CTerrainTile::GetDownTile()
{
	return mpDown;
}

D3DXVECTOR3 CTerrainTile::GetLowerLeftVertexPosition()
{
	return mLowerLeftVertexPosition;
}

D3DXVECTOR3 CTerrainTile::GetLowerRightVertexPosition()
{
	return mLowerRightVertexPosition;
}

D3DXVECTOR3 CTerrainTile::GetUpperLeftVertexPosition()
{
	return mUpperLeftVertexPosition;
}

D3DXVECTOR3 CTerrainTile::GetUpperRightVertexPosition()
{
	return mUpperRightVertexPosition;
}
