#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>

// Global variables.

class CGraphics
{
private:
	float mScreenDepth = 1000.0f;
	float mScreenNear = 0.1f;
public:
	CGraphics();
	CGraphics(const CGraphics&);
	~CGraphics();

	bool Initialise(int, int, HWND);
	void Shutdown();
	bool Frame();
private:
	bool Render();
};

#endif