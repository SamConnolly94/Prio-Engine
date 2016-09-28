#include "Camera.h"



CCamera::CCamera()
{
	// Initialise position of camera.
	mPosX = 0.0f;
	mPosY = 0.0f;
	mPosZ = 0.0f;

	// Initialise rotation of camera.
	mRotX = 0.0f;
	mRotY = 0.0f;
	mRotZ = 0.0f;
}

CCamera::~CCamera()
{
}

void CCamera::SetPosition(float x, float y, float z)
{
	mPosX = x;
	mPosY = y;
	mPosZ = z;
}

void CCamera::SetRotation(float x, float y, float z)
{
	mRotX = x;
	mRotY = y;
	mRotZ = z;
}

D3DXVECTOR3 CCamera::GetPosition()
{
	return D3DXVECTOR3(mPosX, mPosY, mPosZ);
}

D3DXVECTOR3 CCamera::GetRotation()
{
	return D3DXVECTOR3(mRotX, mRotY, mRotZ);
}

void CCamera::Render()
{
	D3DXVECTOR3 up;
	D3DXVECTOR3 position;
	D3DXVECTOR3 defaultAngle;
	float yaw;
	float pitch;
	float roll;
	D3DXMATRIX rotationMatrix;

	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Setup the position of the camera in the world.
	position.x = mPosX;
	position.y = mPosY;
	position.z = mPosZ;

	// Setup where the camera looks by default.
	defaultAngle.x = 0.0f;
	defaultAngle.y = 0.0f;
	defaultAngle.z = 1.0f;

	// Set the yaw (Y) pitch (X) and roll (Z) rotations in radians.
	pitch = mRotX * 0.0174532925f;
	yaw = mRotY * 0.0174532925f;
	roll = mRotZ * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch and roll values.
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

	// Transform the defaultAngle and up vector by the rotation matrix so view is rotated at the origin.
	D3DXVec3TransformCoord(&defaultAngle, &defaultAngle, &rotationMatrix);
	D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

	// Translate the rotated camera pos to the location of the viewer.
	defaultAngle = position + defaultAngle;

	// Create the view matrix from the three updated vectors.
	D3DXMatrixLookAtLH(&mViewMatrix, &position, &defaultAngle, &up);
}

void CCamera::GetViewMatrix(D3DXMATRIX & viewMatrix)
{
	viewMatrix = mViewMatrix;
}
