#include "ModelControl.h"



CModelControl::CModelControl()
{
	mPosition.x = 0.0f;
	mPosition.y = 0.0f;
	mPosition.z = 0.0f;

	mRotation.x = 0.0f;
	mRotation.y = 0.0f;
	mRotation.z = 0.0f;
}


CModelControl::~CModelControl()
{
}

void CModelControl::RotateX(float x)
{
	mRotation.x += x;
}

void CModelControl::RotateY(float y)
{
	mRotation.y += y;
}

void CModelControl::RotateZ(float z)
{
	mRotation.z += z;
}

float CModelControl::GetRotationX()
{
	return mRotation.x;
}

float CModelControl::GetRotationY()
{
	return mRotation.y;
}

float CModelControl::GetRotationZ()
{
	return mRotation.z;
}

D3DXVECTOR3 CModelControl::GetRotation()
{
	return mRotation;
}

void CModelControl::SetRotationX(float x)
{
	mRotation.x = x;
}

void CModelControl::SetRotationY(float y)
{
	mRotation.y = y;
}

void CModelControl::SetRotationZ(float z)
{
	mRotation.z = z;
}

void CModelControl::SetRotation(float x, float y, float z)
{
	mRotation.x = x;
	mRotation.y = y;
	mRotation.z = z;
}

void CModelControl::MoveX(float x)
{
	mPosition.x += x;
}

void CModelControl::MoveY(float y)
{
	mPosition.y += y;
}

void CModelControl::MoveZ(float z)
{
	mPosition.z += z;
}

float CModelControl::GetPosX()
{
	return mPosition.x;
}

float CModelControl::GetPosY()
{
	return mPosition.y;
}

float CModelControl::GetPosZ()
{
	return mPosition.z;
}

D3DXVECTOR3 CModelControl::GetPos()
{
	return mPosition;
}

void CModelControl::SetXPos(float x)
{
	mPosition.x = x;
}

void CModelControl::SetYPos(float y)
{
	mPosition.y = y;
}

void CModelControl::SetZPos(float z)
{
	mPosition.z = z;
}

void CModelControl::SetPos(float x, float y, float z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}
