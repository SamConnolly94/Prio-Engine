#ifndef MESH_H
#define MESH_H

// Windows library includes.
#include <string>
#include <list>

#include "Model.h"
#include "Texture.h"
#include "ColourShader.h"

class CMesh
{
private:
	std::string mFilename;
	std::string mFileExtension;
	CLogger* mpLogger;

	// Arrays to store data about vertices in.
	D3DXVECTOR3* mpVertices;
	D3DXVECTOR3* mpIndices;

	ID3D11Device* mpDevice;

	std::list<CModels*> mpModels;

	// Define the light shaders for rendering.
	CColourShader* mpColourShader;

	CTexture* mpTexture;
public:
	CMesh(ID3D11Device* device, HWND hwnd);
	~CMesh();

	// Loads data from file into our mesh object.
	CModels* CreateModel();
	bool LoadMesh(char* filename, WCHAR* textureName);

	void Render(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj);
private:
	bool LoadSam();
	bool GetSizes();
	bool InitialiseArrays();
	int mVertexCount;
	int mIndexCount;
	const int kNumberOfFloatsInVector3 = 3;
};

#endif