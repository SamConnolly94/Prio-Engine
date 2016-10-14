#include "Engine.h"
#include "Mesh.h"

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

/* Controls any gameplay and things that should happen when we play the game. */
void GameLoop(CEngine* &engine)
{
	CCamera* myCam = engine->CreateCamera();
	myCam->SetPositionZ(-20.0f);

	CMesh* mesh = engine->LoadMesh("resources/textures/cube.obj");
	mesh->CreateModel();

	// Process any initialisation to be done before the gameloop here.
	//CPrimitive* cube = engine->CreatePrimitive(L"../Resources/Textures/TestTex.dds", true, PrioEngine::Primitives::cube);

	//CPrimitive* triangle = engine->CreatePrimitive(PrioEngine::Colours::green, PrioEngine::Primitives::triangle);
	//triangle->SetXPos(3.0f);

	engine->StartTimer();


	float frameTime;

	const float kRotationSpeed = 1.0f;

	// Process anything which should happen in the game here.
	while (engine->IsRunning())
	{
		frameTime = engine->GetFrameTime();
		//cube2->RotateX(kRotationSpeed * frameTime);

		Control(engine, myCam);

	}
}

/* Control any user input here, must be called in every tick of the game loop. */
void Control(CEngine* &engine, CCamera* cam)
{
	const float kMoveSpeed = 10.0f;
	const float kRotationSpeed = 10.0f;
	const float kCamRotationSpeed = 2.5f;
	float frameTime = engine->GetFrameTime();
	
	/// Camera control.
	// Move backwards
	if (engine->KeyHeld(PrioEngine::Key::kS))
	{
		cam->MoveLocalZ(-kMoveSpeed * frameTime);
	}
	// Move Forwards
	else if (engine->KeyHeld(PrioEngine::Key::kW))
	{
		cam->MoveLocalZ(kMoveSpeed * frameTime);
	}
	// Move Left
	if (engine->KeyHeld(PrioEngine::Key::kA))
	{
		cam->MoveLocalX(-kMoveSpeed * frameTime);
	}
	// Move Right
	else if (engine->KeyHeld(PrioEngine::Key::kD))
	{
		cam->MoveLocalX(kMoveSpeed * frameTime);
	}

	// Rotate left
	if (engine->KeyHeld(PrioEngine::Key::kLeft))
	{
		cam->RotateY(-kCamRotationSpeed * frameTime);
	}
	// Rotate right.
	else if (engine->KeyHeld(PrioEngine::Key::kRight))
	{
		cam->RotateY(kCamRotationSpeed * frameTime);
	}
	// Rotate upwards.
	if (engine->KeyHeld(PrioEngine::Key::kUp))
	{
		cam->RotateX(-kCamRotationSpeed * frameTime);
	}
	// Rotate downwards.
	else if (engine->KeyHeld(PrioEngine::Key::kDown))
	{
		cam->RotateX(kCamRotationSpeed * frameTime);
	}

	/// User controls.

	// If the user hits escape.
	if (engine->KeyHit(PrioEngine::Key::kEscape))
	{
		engine->Stop();
	}


}