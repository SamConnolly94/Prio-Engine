#ifndef MODELCONTROL_H
#define MODELCONTROL_H

#include <D3DX10math.h>
class CModelControl
{
protected:
	D3DXVECTOR3 mPosition;
	D3DXVECTOR3 mRotation;

public:
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	float GetRotationX();
	float GetRotationY();
	float GetRotationZ();
	D3DXVECTOR3 GetRotation();

	void SetRotationX(float x);
	void SetRotationY(float y);
	void SetRotationZ(float z);
	void SetRotation(float x, float y, float z);

	void MoveX(float x);
	void MoveY(float y);
	void MoveZ(float z);

	float GetPosX();
	float GetPosY();
	float GetPosZ();
	D3DXVECTOR3 GetPos();

	void SetXPos(float x);
	void SetYPos(float y);
	void SetZPos(float z);
	void SetPos(float x, float y, float z);
public:
	CModelControl();
	~CModelControl();
};

#endif