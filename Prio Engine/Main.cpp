#include "Engine.h"
#include <iostream>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Start the game engine.
	CEngine* myEngine;
	myEngine->getInstance();

	myEngine->getInstance().Cleanup();
	return 0;
}

