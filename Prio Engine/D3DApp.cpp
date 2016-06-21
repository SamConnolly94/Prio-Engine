#include "D3DApp.h"
#include <WindowsX.h>

D3DApp::D3DApp(HINSTANCE hInstance)
{
	mTimer = new CGameTimer();
	mAppPaused = false;
}

D3DApp::~D3DApp()
{
	delete (mTimer);
}

bool D3DApp::Initialise()
{
	return true;
}

int D3DApp::Run()
{
	MSG msg = { 0 };

	mTimer->Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Process a tick on the timer.
			mTimer->Tick();

			// Make sure the game isn't paused.
			if (!mAppPaused) 
			{
				
			}
		}
	}

	return (int)msg.wParam;
}
