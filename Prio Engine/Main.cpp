#include "Engine.h"

// Declaration of functions used to run game itself.
void GameLoop(CEngine* &engine);

void Control(CEngine* &engine, CCamera* cam);

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
	CCamera* myCam = engine->CreateCamera();
	myCam->SetPositionZ(-20.0f);
	// Process any initialisation to be done before the gameloop here.
	CModel* cube = engine->CreateModel(L"../Resources/Textures/TestTex.dds", true, PrioEngine::Primitives::cube);

	CModel* cube2 = engine->CreateModel(PrioEngine::Colours::white, PrioEngine::Primitives::cube);
	cube2->SetXPos(-2.0f);

	engine->StartTimer();

	float frameTime;

	const float kRotationSpeed = 1.0f;

	// Process anything which should happen in the game here.
	while (engine->IsRunning())
	{
		frameTime = engine->GetFrameTime();
		cube2->RotateX(kRotationSpeed * frameTime);

		Control(engine, myCam);

	}
}

void Control(CEngine* &engine, CCamera* cam)
{
	const float kMoveSpeed = 10.0f;
	const float kRotationSpeed = 1.0f;
	float frameTime = engine->GetFrameTime();
	
	// Camera control.
	if (engine->KeyHeld(PrioEngine::Key::kDown))
	{
		cam->MoveZ(-kMoveSpeed * frameTime);
	}
	else if (engine->KeyHeld(PrioEngine::Key::kUp))
	{
		cam->MoveZ(kMoveSpeed * frameTime);
	}
	if (engine->KeyHeld(PrioEngine::Key::kLeft))
	{
		cam->MoveX(-kMoveSpeed * frameTime);
	}
	else if (engine->KeyHeld(PrioEngine::Key::kRight))
	{
		cam->MoveX(kMoveSpeed * frameTime);
	}

	// If the user hits escape.
	if (engine->KeyHit(PrioEngine::Key::kEscape))
	{
		engine->Stop();
	}
}