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

	// Create the engine object.
	PrioEngine = new CEngine();
	// If failed to create the engine.
	if (!PrioEngine)
	{
		// Write a message to the log to let the user know we couldn't create the engine object.
		CLogger::GetLogger().WriteLine("Could not create the engine object.");
		// Return 0, we're saying we're okay, implement error codes in future versions maybe? 
		return 0;
	}

	// Set up the engine.
	result = PrioEngine->Initialise();
	// If we successfully initialised the game engine
	if (result)
	{
		// Start the game loop.
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

	CMesh* mesh = engine->LoadMesh("../Resources/Textures/Cube.sam");
	CModels* model = mesh->CreateModel();
	model->SetPosition(0.0f, 0.0f, 0.0f);

	// Process any initialisation to be done before the gameloop here.
	CPrimitive* cube = engine->CreatePrimitive(PrioEngine::Colours::red, PrioEngine::Primitives::cube);
	cube->SetXPos(-5.0f);
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

		model->RotateY(0.1f);

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