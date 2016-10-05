#include "Light.h"



CLight::CLight()
{
}


CLight::~CLight()
{
}

void CLight::SetDiffuseColour(float3 colour, float alpha)
{
	mDiffuseColour = D3DXVECTOR4(colour.x, colour.y, colour.z, alpha);
}

void CLight::SetDirection(float3 direction)
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
