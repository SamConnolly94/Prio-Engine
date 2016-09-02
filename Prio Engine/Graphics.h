#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "D3D11.h"

// Global variables.
// Will the window run in full screen?
const bool FULL_SCREEN = false;
// Will VSYNC be enabled? (Caps at 60fps)
const bool VSYNC_ENABLED = false;
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

	CD3D11* mD3D;
};

#endif