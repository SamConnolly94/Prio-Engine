#include "Light.h"



CLight::CLight()
{
}


CLight::~CLight()
{
}

void CLight::SetDiffuseColour(PrioEngine::RGBA colour)
{
	mDiffuseColour = D3DXVECTOR4(colour.r, colour.g, colour.b, colour.a);
}

void CLight::SetDirection(PrioEngine::Coords direction)
{
	mDirection = D3DXVECTOR3(direction.x, direction.y, direction.z);
}

D3DXVECTOR4 CLight::GetDiffuseColour()
{
	return mDiffuseColour;
}

D3DXVECTOR3 CLight::GetDirection()
{
	return mDirection;
}
