#ifndef MODEL_H
#define MODEL_H

#include "VertexTypeManager.h"
#include "ModelControl.h"

class CModels : public CModelControl
{
private:
	D3DXVECTOR3* mpVertices;
	D3DXVECTOR3* mpTexCoords;
	D3DXVECTOR3* mpNormal;

	int mVerticesCount;
	int mTextureCount;
	int mNormalsCount;
	int mIndicesCount;
	ID3D11Device* mpDevice;

	CVertexManager* mpVertexManager;
	CLogger* mpLogger;

	ID3D11Buffer* mpIndexBuffer;

	D3DXMATRIX mWorld;
public:
	CModels(ID3D11Device * device);
	~CModels();

	bool SetGeometry(D3DXVECTOR3 * vertices, D3DXVECTOR3* Indices);
	void SetNumberOfVertices(int size);
	void SetTextureCount(int size);
	void SetNumberOfNormals(int size);
	void SetNumberOfIndices(int size);

	int GetNumberOfVertices() { return mVerticesCount; };
	int GetTextureCount() { return mTextureCount; }
	int GetNumberOfNormals() { return mNormalsCount; };
	// Gets the number of indices which form a triangle, for the total number of indices, multiply by three.
	int GetNumberOfIndices() { return mIndicesCount; };

	void UpdateMatrices(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

};

#endif
