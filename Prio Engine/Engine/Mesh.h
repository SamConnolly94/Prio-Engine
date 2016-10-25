#ifndef MESH_H
#define MESH_H

// Windows library includes.
#include <string>
#include <list>

#include "Model.h"
#include "Texture.h"
#include "ColourShader.h"
#include "AssimpManager.h"

class CMesh
{
private:
	std::string mFilename;
	std::string mFileExtension;
	CLogger* mpLogger;

	// Arrays to store data about vertices in.
	D3DXVECTOR3* mpVertices;
	
	unsigned long* mpIndices;

	ID3D11Device* mpDevice;

	std::list<CModel*> mpModels;

	// Define the light shaders for rendering.
	CColourShader* mpColourShader;

	CTexture* mpTexture;
	CAssimpManager* mpAssimpManager;
public:
	CMesh(ID3D11Device* device, HWND hwnd);
	~CMesh();

	// Loads data from file into our mesh object.
	CModel* CreateModel();
	bool LoadMesh(char* filename, WCHAR* textureName);

	void Render(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj);
private:
	bool LoadAssimpModel(char* filename);
	bool LoadSam();
	bool GetSizes();
	bool InitialiseArrays();
	unsigned int mVertexCount;
	unsigned int mIndexCount;
};
#endif