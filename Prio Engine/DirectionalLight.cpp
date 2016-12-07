#include "DirectionalLight.h"



CDirectionalLight::CDirectionalLight()
{
}


CDirectionalLight::~CDirectionalLight()
{
}

void CDirectionalLight::SetAmbientColour(D3DXVECTOR4 colour)
{
	mAmbientColour = D3DXVECTOR4(colour.x, colour.y, colour.z, colour.w);
}

void CDirectionalLight::SetDiffuseColour(D3DXVECTOR4 colour)
{
	mDiffuseColour = D3DXVECTOR4(colour.x, colour.y, colour.z, colour.w);
}

void CDirectionalLight::SetDirection(D3DXVECTOR3 direction)
{
	mDirection = D3DXVECTOR3(direction.x, direction.y, direction.z);
}

D3DXVECTOR4 CDirectionalLight::GetDiffuseColour()
{
	return mDiffuseColour;
}

D3DXVECTOR3 CDirectionalLight::GetDirection()
{
	return mDirection;
}
