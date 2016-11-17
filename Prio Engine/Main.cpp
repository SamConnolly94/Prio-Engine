#include "Engine\PrioEngineVars.h"
#include "Engine/Engine.h"

// Declaration of functions used to run game itself.
void GameLoop(CEngine* &engine);
void Control(CEngine* &engine, CCamera* cam, CTerrainGrid* grid);

// Globals
CLogger* gLogger;

// Main
int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Enable run time memory check while running in debug.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	gLogger = new CLogger();
	// Start the game engine.
	CEngine* PrioEngine;
	bool result;

	// Create the engine object.
	PrioEngine = new CEngine();
	// If failed to create the engine.
	if (!PrioEngine)
	{
		// Write a message to the log to let the user know we couldn't create the engine object.
		gLogger->WriteLine("Could not create the engine object.");
		// Return 0, we're saying we're okay, implement error codes in future versions maybe? 
		return 0;
	}
	gLogger->MemoryAllocWriteLine(typeid(PrioEngine).name());

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
	gLogger->MemoryDeallocWriteLine(typeid(PrioEngine).name());
	PrioEngine = nullptr;
	delete gLogger;

	// The singleton logger will cause a memory leak. Don't worry about it. Should be no more than 64 bytes taken by it though, more likely will only take 48 bytes.
	_CrtDumpMemoryLeaks();

	return 0;
}

/* Controls any gameplay and things that should happen when we play the game. */
void GameLoop(CEngine* &engine)
{
	// Constants.
	const float kRotationSpeed = 100.0f;
	const float kMovementSpeed = 1.0f;

	// Variables
	float frameTime;
	CCamera* myCam;
	CMesh* cubeMesh;
	CMesh* houseMesh = nullptr;

	CLight* ambientLight;
	CModel* cube = nullptr;
	CModel* house = nullptr;
	CTerrainGrid* grid = engine->CreateTerrainGrid();
	grid->CreateGrid();
	CPrimitive* colourCube = engine->CreatePrimitive(PrioEngine::Colours::red, PrioEngine::Primitives::cube);

	// Camera init.
	myCam = engine->CreateCamera();

	// Light init
	ambientLight = engine->CreateLight(D3DXVECTOR4{ 1.0f, 1.0f, 1.0f, 1.0f }, D3DXVECTOR4{ 0.15f, 0.15f, 0.15f, 1.0f });
	ambientLight->SetDirection(D3DXVECTOR3{ 0.0f, 0.0f, 1.0f });
	ambientLight->SetSpecularColour(D3DXVECTOR4{ 1.0f, 1.0f, 1.0f, 1.0f });
	ambientLight->SetSpecularPower(32.0f);

	// Mesh init
	cubeMesh = engine->LoadMesh("Resources/Models/Cube.obj", L"Resources/Textures/seafloor.dds", PrioEngine::ShaderType::Specular);
	houseMesh = engine->LoadMesh("Resources/Models/Wooden_House.fbx", L"Resources/Textures/House_Texture.png", PrioEngine::ShaderType::Diffuse);

	// Model init.
	cube = cubeMesh->CreateModel();
	house = houseMesh->CreateModel();
	cube->SetZPos(-20.0f);

	house->SetXPos(0.0f);
	house->SetXPos(5.0f);
	house->SetRotationX(90.0f);
	house->SetScale(10.0f);

	// Start the game timer running.
	engine->StartTimer();

	grid->SetDrawStyle(Solid);

	// Process anything which should happen in the game here.
	while (engine->IsRunning())
	{
		// Get hold of the time it took to draw the last frame.
		frameTime = engine->GetFrameTime();

		// Process any keys pressed this frame.
		Control(engine, myCam, grid);

		// Rotate the model which has been logged on.
		cube->RotateY(kRotationSpeed * frameTime);
	}
}

/* Control any user input here, must be called in every tick of the game loop. */
void Control(CEngine* &engine, CCamera* cam, CTerrainGrid* grid)
{
	const float kMoveSpeed = 25.0f;
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

	// If the user hits F1.
	if (engine->KeyHit(PrioEngine::Key::kF1))
	{
		if (grid->GetDrawStyle() == Solid)
		{
			grid->SetDrawStyle(Wireframe);
		}
		else
		{
			grid->SetDrawStyle(Solid);
		}
	}
}