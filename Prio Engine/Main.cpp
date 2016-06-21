#include "Engine.h"
#include <iostream>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Enable run time memory check while running in debug.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Start the game engine.
	CEngine* myEngine = new CEngine();

	// Clean up the engine before we exit the program.
	delete (myEngine);
	return 0;
}
