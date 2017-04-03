#ifndef FOLIAGEQUAD_H
#define FOLIAGEQUAD_H

#include <D3DX10math.h>
#include "PrioEngineVars.h"

class CFoliageQuad
{
public:
	struct QuadType
	{
		D3DXVECTOR3 Position[4];
		D3DXVECTOR2 UV[4];
		D3DXVECTOR3 Normal[4];
	};
public:
	CFoliageQuad();
	~CFoliageQuad();
	void GeneratePoints(D3DXVECTOR3 lowerL, D3DXVECTOR3 lowerR, D3DXVECTOR3 upperL, D3DXVECTOR3 upperR);
private:
	QuadType mFoliageRect[3];
	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mCentrePos;
	unsigned int indices[18];
public:
	void SetPosition(D3DXVECTOR3 pos);
	QuadType GetFoliageRect(int index);
	unsigned int GetIndex(int index);
	D3DXVECTOR3 GetCentrePos();
};

#endif