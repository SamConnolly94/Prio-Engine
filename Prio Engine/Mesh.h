#ifndef MESH_H
#define MESH_H

// My class includes.
#include "Logger.h"

// Windows library includes.
#include <string>
#include <list>

#include "Models.h"

class CMesh
{
private:
	std::string mFilename;
	std::string mFileExtension;
	CLogger* mpLogger;

	//std::list<CModels*> mpModels;

	D3DXVECTOR3* mpVertices;
	D3DXVECTOR3* mpTexCoords;
	D3DXVECTOR3* mpNormal;
	
	typedef struct
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	} FaceType;

	FaceType* mpFaces;

	ID3D11Device* mpDevice;
public:
	CMesh(ID3D11Device* device);
	~CMesh();

	// Loads data from file into our mesh object.
	void CMesh::CreateModel();
	bool LoadMesh(char* filename);

private:
	bool LoadObj();
	bool InitialiseArrays();
	int mVertexCount;
	int mFaceCount;
	int mNormalCount;
	int mTextureCount;
};

#endif