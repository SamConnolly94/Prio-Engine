#ifndef PRIMITIVE_H
#define PRIMITIVE_H

// Include DirectX libraries.
#include <d3d11.h>
#include <D3DX10math.h>
#include "VertexTypeManager.h"
#include "PrioEngineVars.h"
#include "Logger.h"
#include "Texture.h"

class CPrimitive
{
public:
	CPrimitive() {};
	CPrimitive(WCHAR* textureFilename);
	CPrimitive(WCHAR* textureFilename, bool useLighting);
	CPrimitive(PrioEngine::RGBA colour);
	~CPrimitive();
public:
	virtual bool Initialise(ID3D11Device* device) = 0;
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);
	int GetIndex();

	ID3D11ShaderResourceView* GetTexture();
private:
	void ShutdownBuffers();
	virtual bool InitialiseBuffers(ID3D11Device* device) = 0;
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	void ReleaseTexture();
private:

	int GetNumberOfIndices();
	int GetNumberOfVertices();
	void LoadIndiceData(unsigned long* &indices);
public:
	bool HasTexture();
	bool HasColour();
	bool UseDiffuseLight();

	// Transformations and translations..
public:
	void RotateX(float x);
	void RotateY(float y);
	void RotateZ(float z);

	float GetRotationX();
	float GetRotationY();
	float GetRotationZ();

	void SetRotationX(float x);
	void SetRotationY(float y);
	void SetRotationZ( float z);

	void MoveX(float x);
	void MoveY(float y);
	void MoveZ(float z);

	float GetPosX();
	float GetPosY();
	float GetPosZ();

	void SetXPos(float x);
	void SetYPos(float y);
	void SetZPos(float z);
protected:
	float mPositionX;
	float mPositionY;
	float mPositionZ;

	float mRotationX;
	float mRotationY;
	float mRotationZ;

protected:
	ID3D11Device * mpDevice;
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	int mVertexCount;
	int mIndexCount;
	CTexture* mpTexture;
	WCHAR* mpTextureFilename;
	bool mApplyTexture;
	PrioEngine::RGBA mColour;
	bool mUseDiffuseLighting;
	PrioEngine::Primitives mShape;
	CVertexManager* mpVertexManager;

	void ResetColour();
	bool LoadTexture(ID3D11Device* device);

	// Logger, used in all classes, parent and children.
	CLogger* mpLogger;
};

#endif