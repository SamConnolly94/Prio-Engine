#ifndef MODEL_H
#define MODEL_H

// Include DirectX libraries.
#include <d3d11.h>
#include <D3DX10math.h>
#include "PrioTypes.h"

#include "Logger.h"
#include "Texture.h"

class CModel
{
private:
	// Structure types, should reflect the vertex / pixel shader.
	struct VertexColourType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 colour;
	};
	struct VertexTextureType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};
	struct VertexDiffuseLightingType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
		D3DXVECTOR3 normal;
	};

public:
	CModel(WCHAR* textureFilename);
	CModel(WCHAR* textureFilename, bool useLighting);
	CModel(float3 colour);
	~CModel();
public:
	bool Initialise(ID3D11Device* device);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);
	ID3D11Device * mpDevice;
	int GetIndex();

	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitialiseBuffers(ID3D11Device* device, bool applyTexture);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	bool LoadTexture(ID3D11Device* device);
	void ReleaseTexture();
private:
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	int mVertexCount;
	int mIndexCount;
	CTexture* mpTexture;
	WCHAR* mpTextureFilename;

	CLogger* mpLogger;

	float3 mColour;
	void ResetColour();

	bool mUseDiffuseLighting;
	// Set the buffers for the triangle.
	bool SetTriangleBuffers(VertexColourType* &verticesColour, VertexTextureType* &verticesTexture, VertexDiffuseLightingType* &verticesDiffuse, bool applyTexture,
							float x, float y, float z);
	bool CreateVertexArray(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse, bool applyTexture);
public:
	bool HasTexture();
	bool HasColour();
	bool UseDiffuseLight();

	// Transformations.
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
private:
	float mPositionX;
	float mPositionY;
	float mPositionZ;

	float mRotationX;
	float mRotationY;
	float mRotationZ;

private:
	// Commits any changes to position to the matrix.
	void CommitToMatrix();
	void CleanVertexBuffers(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse);
	bool CreateVertexBuffer(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse);

	// Place rotation into a matrix and passes it to DirectX to draw.
	void CommitRotate();
};

#endif