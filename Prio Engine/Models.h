#ifndef MODEL_H
#define MODEL_H

#include "VertexTypeManager.h"

class CModels
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
public:
	CModels(ID3D11Device * device);
	~CModels();

	bool SetGeometry(D3DXVECTOR3* vertices, D3DXVECTOR3* texCoords, D3DXVECTOR3* normals);
	void SetNumberOfVertices(int size);
	void SetTextureCount(int size);
	void SetNumberOfNormals(int size);
	void SetNumberOfIndices(int size);

	int GetNumberOfVertices() { return mVerticesCount; };
	int GetTextureCount() { return mTextureCount; }
	int GetNumberOfNormals() { return mNormalsCount; };
	int GetNumberOfIndices() { return mIndicesCount; };
};

#endif
