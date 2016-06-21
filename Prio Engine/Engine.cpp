#include "Engine.h"

using namespace std;

//Constructor
CEngine::CEngine() 
{
	Initialise();
}

void CEngine::Cleanup()
{
	delete(mLogger);
}

void CEngine::Initialise()
{
	// Define the logger, we'll let that class handle if we should be logging or not.
	mLogger = new CLogger();
	mLogger->WriteLine("Completed initilisation of Prio Engine.");
}

//LRESULT CALLBACK
//MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
//	// before CreateWindow returns, and thus before mhMainWnd is valid.
//	//return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
//}

bool CEngine::OpenMainWindow()
{
	//WNDCLASS wc;
	//wc.style = CS_HREDRAW | CS_VREDRAW;
	//wc.lpfnWndProc = MainWnd
	return false;
}
