#ifndef MESH_H
#define MESH_H

#include <string>
#include "Logger.h"
#include <D3DX10math.h>

class CMesh
{
private:
	std::string mFilename;
	std::string mFileExtension;
	CLogger* mpLogger;


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
public:
	CMesh();
	~CMesh();

	// Loads data from file into our mesh object.
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