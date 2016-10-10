#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <list>
#include "D3D11.h"
#include "Camera.h"
#include "Cube.h"
#include "Triangle.h"
#include "Primitive.h"
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
	int mScreenWidth, mScreenHeight;

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
	CPrimitive* mpTriangle;
	CColourShader* mpColourShader;
	CTextureShader* mpTextureShader;
	CDiffuseLightShader* mpDiffuseLightShader;
	
	bool RenderPrimitiveWithTexture(CPrimitive* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix);
	bool RenderPrimitiveWithColour(CPrimitive* model, D3DMATRIX worldMatrix, D3DMATRIX viewMatrix, D3DMATRIX projMatrix);
	bool RenderPrimitiveWithTextureAndDiffuseLight(CPrimitive* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix);

	std::list<CPrimitive*> mpPrimitives;
	bool CreateTextureShaderForModel(CPrimitive* &model, HWND hwnd);
	bool CreateColourShaderForModel(CPrimitive* &model, HWND hwnd);
	bool CGraphics::CreateTextureAndDiffuseLightShaderFromModel(CPrimitive* &model, HWND hwnd);
	bool RenderModels(D3DXMATRIX view, D3DXMATRIX world, D3DXMATRIX proj);

	float mRotation;

	HWND mHwnd;
public:
	/* Model control from the engine. */
	CPrimitive* CreatePrimitive(WCHAR* TextureFilename, PrioEngine::Primitives shape);
	CPrimitive* CreatePrimitive(PrioEngine::RGBA colour, PrioEngine::Primitives shape);
	CPrimitive* CreatePrimitive(WCHAR* TextureFilename, bool useLighting, PrioEngine::Primitives shape);
	bool RemoveModel(CPrimitive* &model);

	/* Camera control, required by the engine. */
	CCamera* CreateCamera();
	void SetCameraPos(float x, float y, float z);
};

#endif