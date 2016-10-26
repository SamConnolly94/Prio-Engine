#ifndef LIGHTCLASS_H
#define LIGHTCLASS_H

#include <D3DX10math.h>
#include "PrioEngineVars.h"
#include "ModelControl.h"

class CLight : CModelControl
{
public:
	CLight();
	~CLight();

	void SetDiffuseColour(PrioEngine::RGBA colour);
	void SetDirection(D3DXVECTOR3 direction);

	D3DXVECTOR4 GetDiffuseColour();
	D3DXVECTOR3 GetDirection();

private:
	D3DXVECTOR4 mDiffuseColour;
	D3DXVECTOR3 mDirection;
};

#endif