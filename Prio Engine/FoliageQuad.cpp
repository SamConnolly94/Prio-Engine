#include "FoliageQuad.h"



CFoliageQuad::CFoliageQuad()
{
}


CFoliageQuad::~CFoliageQuad()
{
}

void CFoliageQuad::GeneratePoints(D3DXVECTOR3 lowerL, D3DXVECTOR3 lowerR, D3DXVECTOR3 upperL, D3DXVECTOR3 upperR)
{
	// Need to define a quad in the shape of an astericks *.
	// This will result in relatively thick grass.

	D3DXVECTOR3 LL = lowerL - lowerL;
	D3DXVECTOR3 LR = lowerR - lowerL;
	D3DXVECTOR3 UL = upperL - lowerL;
	D3DXVECTOR3 UR = upperR - lowerL;

	mCentrePos = (LL + LR + UL + UR) / 4;

	//////////////////////
	// Horizontal line
	//////////////////////

	float height = 1.0f;

	// Bottom left
	mFoliageRect[0].Position[0] = (LL + UL) / 2;
	mFoliageRect[0].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[0].Normal[0] = { 0.0f, 1.0f, 0.0f };
	// Bottom right
	mFoliageRect[0].Position[1] = (LR + UR) / 2;
	mFoliageRect[0].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[0].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[0].Position[2] = { (LL.x + UL.x) / 2, ((LL.y + UL.y) / 2) + height, (LL.z + UL.z) / 2 };
	mFoliageRect[0].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[0].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[0].Position[3] = { (LR.x + UR.x) / 2, ((LR.y + UR.y) / 2) + height, (LR.z + UR.z) / 2 };
	mFoliageRect[0].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[0].Normal[3] = { 0.0f, 1.0f, 0.0f };

	////////////////////////
	// Starting point top left, diag down to right
	///////////////////////
	// Bottom left
	mFoliageRect[1].Position[0] = { UL.x, UL.y, UL.z };
	mFoliageRect[1].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[1].Normal[0] = { 0.0f, 1.0f, 0.0f };
	// Bottom right
	mFoliageRect[1].Position[1] = { LR.x, LR.y, LR.z };
	mFoliageRect[1].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[1].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[1].Position[2] = { UL.x, UL.y + height, UL.z };
	mFoliageRect[1].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[1].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[1].Position[3] = { LR.x, LR.y + height, LR.z };
	mFoliageRect[1].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[1].Normal[3] = { 0.0f, 1.0f, 0.0f };

	////////////////////////
	// Starting point bottom left, diag up to right
	///////////////////////
	// Bottom left
	mFoliageRect[2].Position[0] = { LL.x, LL.y, LL.z };
	mFoliageRect[2].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[2].Normal[0] = { 0.0f, 1.0f, 0.0f };
	// Bottom right
	mFoliageRect[2].Position[1] = { UR.x, UR.y, UR.z };
	mFoliageRect[2].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[2].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[2].Position[2] = { LL.x, LL.y + height, LL.z };
	mFoliageRect[2].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[2].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[2].Position[3] = { UR.x, UR.y + height, UR.z };
	mFoliageRect[2].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[2].Normal[3] = { 0.0f, 1.0f, 0.0f };

	// rect 1

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 3;
	indices[4] = 1;
	indices[5] = 2;

	// Rect 2

	indices[6] = 4;
	indices[7] = 5;
	indices[8] = 6;

	indices[9] = 7;
	indices[10] = 5;
	indices[11] = 6;

	// Rect 3

	indices[12] = 8;
	indices[13] = 9;
	indices[14] = 10;

	indices[15] = 11;
	indices[16] = 9;
	indices[17] = 10;
}

void CFoliageQuad::SetPosition(D3DXVECTOR3 pos)
{
	mPosition = pos;
}

CFoliageQuad::QuadType CFoliageQuad::GetFoliageRect(int index)
{
	QuadType quad = mFoliageRect[index];
	for (int i = 0; i < 4; i++)
	{
		quad.Position[i] += mPosition;
	}

	return quad;
}

unsigned int CFoliageQuad::GetIndex(int index)
{
	return indices[index];
}

D3DXVECTOR3 CFoliageQuad::GetCentrePos()
{
	return mCentrePos;
}
