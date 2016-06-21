#pragma once
#ifndef D3DAPP_H
#define D3DAPP_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "GameTimer.h"
#include <DirectXMath.h>

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
private:
	CGameTimer* mTimer;
	bool mAppPaused;
public:
	D3DApp(HINSTANCE hInstance);
	~D3DApp();

	virtual bool Initialise();

	int Run();
protected:

};

#endif