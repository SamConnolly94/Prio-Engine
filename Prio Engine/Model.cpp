#include "Model.h"


CModel::CModel()
{
	logger->GetInstance().MemoryAllocWriteLine(typeid(this).name());
}


CModel::~CModel()
{
	// Write an allocation message to our memory log.
	logger->GetInstance().MemoryAllocWriteLine(typeid(this).name());
}

void CModel::Shutdown()
{
}

void CModel::UpdateMatrices()
{
	// Rotation
	D3DXMATRIX translation;
	D3DXMATRIX scale;
	D3DXMATRIX matrixRotationX;
	D3DXMATRIX matrixRotationY;
	D3DXMATRIX matrixRotationZ;

	// Calculate the rotation of the model.
	float rotX = (GetRotationX() * PrioEngine::kPi) / 180.0f;
	float rotY = (GetRotationY() * PrioEngine::kPi) / 180.0f;
	float rotZ = (GetRotationZ() * PrioEngine::kPi) / 180.0f;

	D3DXMatrixRotationX(&matrixRotationX, rotX);
	D3DXMatrixRotationY(&matrixRotationY, rotY);
	D3DXMatrixRotationZ(&matrixRotationZ, rotZ);

	// Calculate scaling.
	D3DXMatrixScaling(&scale, GetScaleX(), GetScaleY(), GetScaleZ());

	// Calculate the translation of the model.
	D3DXMatrixTranslation(&translation, GetPosX(), GetPosY(), GetPosZ());

	// Calculate the world matrix
	mWorldMatrix = scale * matrixRotationX * matrixRotationY * matrixRotationZ * translation;
}