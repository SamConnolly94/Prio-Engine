#include "D3DApp.h"
#include <WindowsX.h>

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}



bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	return true;
}

D3DApp::D3DApp(HINSTANCE hInstance)
{
}

D3DApp::~D3DApp()
{
}

D3DApp* D3DApp::mApp = nullptr;
D3DApp * D3DApp::GetApp()
{
	return mApp;
}

/* Set up the a window using DirectX 12. */
bool D3DApp::Initialise()
{
	// Run process to open the main window. 
	if (!InitMainWindow())
	{
		// Failed to open the main window, return false so others can see init function failed.
		return false;
	}

	return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg )
	{
		// When the window is activated or deactivated. Paused becomes unactivated, unpaused becomes actiavted.
	case WM_ACTIVATE:
		if ( LOWORD( wParam ) == WA_INACTIVE )
		{
			// Pause the game when not focus.
			mAppPaused = true;

			// Prevent timer from running unecessarily in the background.
			mTimer.Stop();
		}
		else
		{
			// Unpause the game when refocused.
			mAppPaused = false;

			// Resume the timer.
			mTimer.Start();
		}
	}
	return LRESULT();
}