#include "Engine.h"

// Declaration of functions used to run game itself.
void GameLoop(CEngine* &engine);

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
		// Test comment
		return 0;
	}

	result = PrioEngine->Initialise();
	if (result)
	{
		GameLoop(PrioEngine);
	}

	// Shutdown and release the engine.
	PrioEngine->Shutdown();
	delete PrioEngine;
	PrioEngine = nullptr;

	// The singleton logger will cause a memory leak. Don't worry about it. Should be no more than 64 bytes taken by it though, more likely will only take 48 bytes.
	_CrtDumpMemoryLeaks();

	return 0;
}

void GameLoop(CEngine* &engine)
{
	// Process any initialisation to be done before the gameloop here.
	CModel* triangle = engine->CreateModel(L"../Resources/Textures/TestTex.dds", false, PrioEngine::Primitives::cube);

	engine->StartTimer();
	// Process anything which should happen in the game here.
	while (engine->IsRunning())
	{
		//triangle->RotateY(static_cast<float>(D3DX_PI) * 0.01f);
		//if (triangle->GetRotationY() > 360.0f)
		//{
		//	triangle->SetRotationY(0.0f);
		//}
		//triangle->MoveY(0.01f);
		//triangle->RotateY(0.01f);

	}
}