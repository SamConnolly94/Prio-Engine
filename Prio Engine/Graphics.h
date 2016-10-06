#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <list>
#include "D3D11.h"
#include "Camera.h"
#include "Model.h"
#include "ColourShader.h"
#include "TextureShader.h"
#include "DiffuseLightShader.h"
#include "Light.h"

// Global variables.
// Will the window run in full screen?
const bool FULL_SCREEN = false;
// Will VSYNC be enabled? (Caps at your monitor refresh rate)
const bool VSYNC_ENABLED = true;
// Far clip
const float SCREEN_DEPTH = 1000.0f;
// Near clip
const float SCREEN_NEAR = 0.1f;

class CGraphics
{
private:
	CLogger* mpLogger;
public:
	CGraphics();
	~CGraphics();

	bool Initialise(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();
	bool Frame();
private:
	bool Render();

	CD3D11* mpD3D;

	CLight* mpLight;

	CCamera* mpCamera;
	CModel* mpTriangle;
	CColourShader* mpColourShader;
	CTextureShader* mpTextureShader;
	CDiffuseLightShader* mpDiffuseLightShader;
	
	bool RenderModelWithTexture(CModel* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix);
	bool RenderModelWithColour(CModel* model, D3DMATRIX worldMatrix, D3DMATRIX viewMatrix, D3DMATRIX projMatrix);
	bool RenderModelsWithTextureAndDiffuseLight(CModel* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix);

	std::list<CModel*> mpModels;
	bool CreateTextureShaderForModel(CModel* &model, HWND hwnd);
	bool CreateColourShaderForModel(CModel* &model, HWND hwnd);
	bool CGraphics::CreateTextureAndDiffuseLightShaderFromModel(CModel* &model, HWND hwnd);
	bool RenderModels(D3DXMATRIX view, D3DXMATRIX world, D3DXMATRIX proj);

	float mRotation;

	HWND mHwnd;
public:

	CModel* CreateModel(WCHAR* TextureFilename);
	CModel* CreateModel(float3 colour);
	CModel* CreateModel(WCHAR* TextureFilename, bool useLighting);
	bool RemoveModel(CModel* &model);
};

#endif