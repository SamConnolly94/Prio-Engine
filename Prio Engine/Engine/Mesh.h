#ifndef MESH_H
#define MESH_H

// Windows library includes.
#include <string>
#include <list>

#include "Model.h"
#include "Texture.h"
#include "DiffuseLightShader.h"
#include "ColourShader.h"
#include "AssimpManager.h"
#include "Light.h"

class CMesh
{
private:
	// Handle of our main window.
	HWND mHwnd;

	// Pointer to the device object.
	ID3D11Device* mpDevice;

	// Constants.
	const int kNumIndicesInFace = 3;
	const int kCuttoffSize = 300000;

	// Shader objects.
	CDirectionalLightShader* mpDirectionalLightShader;
	CColourShader* mpColourShader;

	// File strings
	std::string mFilename;
	std::string mFileExtension;

	// A list of the instance of models belonging to this mesh.
	std::list<CModel*> mpModels;

	// Shaders

	// Loggers
	CLogger* mpLogger;

	// Manager objects for loading meshes.
	CAssimpManager* mpAssimpManager;

	// The object reffering to the texture for this mesh.
	CTexture* mpTexture;

	// Arrays to store data about vertices in.
	D3DXVECTOR3* mpVertices;
	unsigned long* mpIndices;
	D3DXVECTOR2* mpUV;
	D3DXVECTOR3* mpNormals;

public:
	CMesh(ID3D11Device* device, HWND hwnd);
	~CMesh();

	// Loads data from file into our mesh object.
	CModel* CreateModel();
	bool LoadMesh(char* filename, WCHAR* textureName);

	void Render(ID3D11DeviceContext* context, D3DXMATRIX &view, D3DXMATRIX &proj, std::list<CLight*>lights);
private:
	bool LoadAssimpModel(char* filename);
	bool LoadSam();
	bool GetSizes();
	bool InitialiseArrays();
	unsigned int mVertexCount;
	unsigned int mIndexCount;
};
#endif