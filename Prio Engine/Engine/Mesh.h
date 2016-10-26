#ifndef MESH_H
#define MESH_H

// Windows library includes.
#include <string>
#include <list>

#include "Model.h"
#include "Texture.h"
#include "TextureShader.h"
#include "ColourShader.h"
#include "AssimpManager.h"

class CMesh
{
private:
	const int kNumIndicesInFace = 3;
	std::string mFilename;
	std::string mFileExtension;
	CLogger* mpLogger;

	// Pointer to the device object.
	ID3D11Device* mpDevice;

	// Arrays to store data about vertices in.
	D3DXVECTOR3* mpVertices;
	unsigned long* mpIndices;
	D3DXVECTOR2* mpUV;
	D3DXVECTOR4* mpColours;

	// A list of the instance of models belonging to this mesh.
	std::list<CModel*> mpModels;

	// Define the light shaders for rendering.
	CTextureShader* mpTextureShader;

	CTexture* mpTexture;
	CAssimpManager* mpAssimpManager;

	const int kCuttoffSize = 300000;
public:
	CMesh(ID3D11Device* device, HWND hwnd);
	~CMesh();

	// Loads data from file into our mesh object.
	CModel* CreateModel();
	bool LoadMesh(char* filename, WCHAR* textureName);

	void Render(ID3D11DeviceContext* context, D3DXMATRIX &view, D3DXMATRIX &proj);
private:
	bool LoadAssimpModel(char* filename);
	bool LoadSam();
	bool GetSizes();
	bool InitialiseArrays();
	unsigned int mVertexCount;
	unsigned int mIndexCount;
};
#endif