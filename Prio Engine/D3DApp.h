#pragma once
#ifndef D3DAPP_H
#define D3DAPP_H

#include <windows.h>
#include "GameTimer.h"

class D3DApp
{
protected:
	bool InitMainWindow();
	D3DApp(HINSTANCE hInstance);
	~D3DApp();
public:
	static D3DApp* GetApp();
	virtual bool Initialise();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	static D3DApp* mApp;
	bool           mAppPaused = false;	// Boolean flag indicating if app is paused or not.

	// a timer used for multiple functions throughout a game.
	CGameTimer mTimer;
};

#endif