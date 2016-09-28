#ifndef CAMERA_H
#define CAMERA_H

#include <d3dx10math.h>

class CCamera
{
public:
	CCamera();
	~CCamera();

	void SetPosition(float x, float y, float z);
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