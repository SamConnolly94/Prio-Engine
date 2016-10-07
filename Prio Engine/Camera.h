#ifndef CAMERA_H
#define CAMERA_H

#include <d3dx10math.h>

class CCamera
{
public:
	CCamera();
	~CCamera();

	// Set the position of the camera in the world.
	void SetPosition(float x, float y, float z);
	void SetPositionX(float x);
	void SetPosizionY(float y);
	void SetPositionZ(float z);

	void MoveX(float x);
	void MoveY(float y);
	void MoveZ(float z);

	float GetX();
	float GetY();
	float GetZ();

	void SetRotation(float x, float y, float z);

	D3DXVECTOR3 GetPosition();
	D3DXVECTOR3 GetRotation();

	void Render();
	void GetViewMatrix(D3DXMATRIX& viewMatrix);

private:
	float mPosX;
	float mPosY;
	float mPosZ;
	float mRotX;
	float mRotY;
	float mRotZ;
	D3DXMATRIX mViewMatrix;
};

#endif