#include "Engine.h"

int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Enable run time memory check while running in debug.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// Start the game engine.
	CEngine* PrioEngine;
	bool result;
	CLogger* logger;

	// Create the engine object.
	PrioEngine = new CEngine();
	if (!PrioEngine)
	{
		logger->GetLogger().WriteLine("Could not create the engine object.");
		return 0;
	}

	result = PrioEngine->Initialise();
	if (result)
	{
		PrioEngine->Run();
	}

	// Shutdown and release the engine.
	PrioEngine->Shutdown();
	delete PrioEngine;
	PrioEngine = nullptr;

	// The singleton logger will cause a memory leak. Don't worry about it. Should be no more than 64 bytes taken by it though, more likely will only take 48 bytes.
	_CrtDumpMemoryLeaks();

	return 0;
}
