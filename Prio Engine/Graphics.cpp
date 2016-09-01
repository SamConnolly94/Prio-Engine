#include "Graphics.h"

CGraphics::CGraphics()
{
}

CGraphics::CGraphics(const CGraphics &)
{
}


CGraphics::~CGraphics()
{
}

bool CGraphics::Initialise(int, int, HWND)
{
	return true;
}

void CGraphics::Shutdown()
{
	return;
}

bool CGraphics::Frame()
{
	return true;
}

bool CGraphics::Render()
{
	return true;
}
