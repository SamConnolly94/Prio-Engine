#include "Engine.h"

/* Default constructor. */
CEngine::CEngine()
{
	mpInput = nullptr;
	mpGraphics = nullptr;
	mTimer = new CGameTimer();
	gLogger->MemoryAllocWriteLine(typeid(mTimer).name());
	mStopped = false;
	for (int i = 0; i < 256; i++)
	{
		mKeyRecentlyHit[i] = false;
	}
	mTimeSinceLastKeyPress = 0.0f;
	mWireframeEnabled = false;
}

/* Default destructor. */
CEngine::~CEngine()
{
	gLogger->WriteLine("Engine destructor called.");
}

/* Initialise our engine. */
bool CEngine::Initialise()
{
	// The width we have to work with on our monitor.
	int screenWidth = 0;
	// The height we have to work with on our monitor.
	int screenHeight = 0;

	// Boolean to store the result of attempting to initialise DirectX in our application.
	bool result;

	// Our graphics object will need to be created so we can check for fullscreen, if not it will cause errors.
	mpGraphics = new CGraphics();

	// Initialise the windows API.
	InitialiseWindows(screenWidth, screenHeight);

	// Initialise the input object. Used to read any input through keyboard or mouse from a user.
	mpInput = new CInput();

	// Check that the input object has been successfully initialised.
	if (!mpInput)
	{
		// Output failure message to the log.
		gLogger->WriteLine("Failed to create the input object. ");
		
		// Prevent function from continuing any further, could not init input log.
		return false;
	}

	gLogger->MemoryAllocWriteLine(typeid(mpInput).name());
	
	// Set up the input object for use.
	mpInput->Initialise();
	// Check to see if graphics object was created successfully.
	if (!mpGraphics)
	{
		// Output error message to the log.
		gLogger->WriteLine("Failed to create the graphics object. ");
		
		// Prevent function from continuing any further, could not init input log.
		return false;
	}

	gLogger->MemoryAllocWriteLine(typeid(mpGraphics).name());

	// Initialise the graphics object
	result = mpGraphics->Initialise(screenWidth, screenHeight, mHwnd);

	// Check that the graphics object was successfully initialised for use.
	if (!result)
	{
		// Output error message to the log.
		gLogger->WriteLine("Failed to initialise the graphics object for use.");
		
		// Prevent the funciton from continuing any further.
		return false;
	}

	// Initialise the message structure.
	ZeroMemory(&mMsg, sizeof(MSG));

	// Reset the timer so we are starting from 0.
	mTimer->Reset();

	// Success! Initialised all essentials for our system.
	return true;
}

/* Clean up and free any memory our program has claimed. */
void CEngine::Shutdown()
{
	// Release the input object.
	if (mpInput)
	{
		// Deallocate the memory given to the input object.
		delete mpInput;
		gLogger->MemoryDeallocWriteLine(typeid(mpInput).name());
		// Reset the input object pointer to null.
		mpInput = nullptr;
	}

	if (mTimer)
	{
		// Deallocate the memory give to the timer.
		delete mTimer;
		gLogger->MemoryDeallocWriteLine(typeid(mTimer).name());
		mTimer = nullptr;
	}

	// Shutdown the window.
	ShutdownWindows();

	// Release the graphics object.
	if (mpGraphics)
	{
		// Run the shutdown function for graphics.
		mpGraphics->Shutdown();
		// Deallocate the memory given to the graphics object.
		delete mpGraphics;
		// Reset the pointer to the graphics object to null.
		mpGraphics = nullptr;
		gLogger->MemoryDeallocWriteLine(typeid(mpGraphics).name());
	}

	return;
}

/* Run our engine until we quit. */
bool CEngine::IsRunning()
{
	// Check for quit messages from user from last loop.
	if (mStopped)
		return false;

	// Loop until there is a quit message from the window or the user.
	CheckWindowsMessages(mMsg);

	// Control what happens when windows signals to end application.
	mIsRunning = ProcessWindowsMessages();

	// If we aren't complete yet, and don't need to process any windows messages, process away!
	if (mIsRunning)
	{
		// Attempt to process the current frame.
		result = Frame();

		// If we failed to process the current frame then quit, something has gone wrong!
		if (!result)
		{
			mIsRunning = false;
			// Return false here to avoid following through until the code which returns true.
			return false;
		}
	}
	else
	{
		// We recieved the quit message from windows, stop!
		return false;
	}
	mTimer->Tick();

	mFrameTime = mTimer->DeltaTime();

	return mIsRunning;
}

/* Find the time that it has taken to draw this frame. */
float CEngine::GetFrameTime()
{
	return mFrameTime;
}

/* Handle messages from the OS. */
LRESULT CEngine::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_KEYDOWN:
		{
			mpInput->KeyDown((unsigned int) wparam);
			return 0;
		}
		case WM_KEYUP:
		{
			mpInput->KeyUp((unsigned int) wparam);
			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, umsg, wparam, lparam);
		}
	}
}

/* Process anything that we need to this frame. */
bool CEngine::Frame()
{
	bool result;

	// Process graphics for this frame;
	result = mpGraphics->Frame();
	if (!result)
	{
		gLogger->WriteLine("Failed to process the graphics for this frame. ");
		return false;
	}

	return true;
}

/* Initialise the window and engine ready for us to use DirectX. */
void CEngine::InitialiseWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// Get external pointer to this object.
	ApplicationHandle = this;

	// Get the instance of this application
	mHinstance = GetModuleHandle(NULL);

	// Give the application a name.
	mApplicationName = L"Prio-Engine";

	// Setup the window class with the default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mHinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = mApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the windows class in the OS.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings for either full screen or windowed.
	if (mpGraphics->IsFullscreen())
	{
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 64;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = 0;
		posY = 0;

		gLogger->WriteLine("Successfully set the settings for fullscreen window.");
	}
	else
	{
		screenWidth = 1600;
		screenHeight = 900;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

		gLogger->WriteLine("Successfully set the settings for windowed mode window.");
	}

	// Create the window.
	mHwnd = CreateWindowEx(WS_EX_APPWINDOW, mApplicationName, mApplicationName,
		WS_OVERLAPPEDWINDOW, /* other options include: WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP*/
		posX, posY, screenWidth, screenHeight, NULL, NULL, mHinstance, NULL);
	
	// Output message to the log.
	if (mHwnd)
	{
		gLogger->WriteLine("Window created.");
	} 
	else
	{
		gLogger->WriteLine("Window was not successfully created..");
	}

	// Focus upon the window.
	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd);
	SetFocus(mHwnd);
	gLogger->WriteLine("Set focus upon the window.");

	// Hide the mouse cursor.
	ShowCursor(true);

	return;
}

/* Tidy up function for when the engine is closed. */
void CEngine::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix display settings when leaving full screen mode.
	if (mpGraphics->IsFullscreen())
	{
		ChangeDisplaySettings(NULL, 0);
		gLogger->WriteLine("Full screen display settings reset to defaults.");
	}

	// Remove the window.
	DestroyWindow(mHwnd);
	mHwnd = NULL;
	gLogger->WriteLine("Window destroyed.");

	// Remove the application instance.
	UnregisterClass(mApplicationName, mHinstance);
	mHinstance = NULL;
	gLogger->WriteLine("Application has been deregistered.");

	// Release the pointer to the class.
	ApplicationHandle = NULL;
	gLogger->WriteLine("Application handle has been successfully released.");

	gLogger->WriteLine("Shutdown of window successful.");

	return;
}

/* Call back for when recieving a message from the OS. */
LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umsg, wparam, lparam);
		}
	}
}

/* Checks the messages our window has recieved from the OS. */
void CEngine::CheckWindowsMessages(MSG &msg)
{
	// Handle any inbound windows messages.
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/* Check whether our engine should still be ran or it is time to quit. */
bool CEngine::ProcessWindowsMessages()
{
	// Control what happens when windows signals to end application.
	if (mMsg.message == WM_QUIT)
	{
		return false;
	}
	if (TwEventWin(mMsg.hwnd, mMsg.message, mMsg.wParam, mMsg.lParam))
		return true; // Event has been handled by AntTweakBar

	return true;
}

/* Start the game timer running. */
void CEngine::StartTimer()
{
	mTimer->Start();
}

/* Creates a camera which we will use to view the world from. */
CCamera* CEngine::CreateCamera()
{
	return mpGraphics->CreateCamera();
}

void CEngine::ToggleWireframe()
{
	mpGraphics->ToggleWireframe();
}

SentenceType * CEngine::CreateText(std::string text, int posX, int posY, int maxLength)
{
	return mpGraphics->CreateSentence(text, posX, posY, maxLength);
}

bool CEngine::UpdateText(SentenceType *& sentence, std::string text, int posX, int posY, PrioEngine::RGB colour)
{
	return mpGraphics->UpdateSentence(sentence, text, posX, posY, colour);
}

bool CEngine::RemoveText(SentenceType *& sentence)
{
	return mpGraphics->RemoveSentence(sentence);
}

/* Detects if a key has been pressed once. 
You can find a list of keys in PrioEngine::Key:: namespace.*/
bool CEngine::KeyHit(const unsigned int key)
{
	if (mpInput->KeyHit(key))
	{
		mpInput->KeyUp(key);
		return true;
	}

	// Key wasn't hit or had been pressed too soon since the last time it was hit.
	return false;
}

/* Detects whether a key is being held. 
You can find a list of keys in PrioEngine::Key:: namespace.*/
bool CEngine::KeyHeld(const unsigned int key)
{
	return mpInput->KeyHeld(key);
}

/* Prevent the engine from running for any longer. */
void CEngine::Stop()
{
	mStopped = true;
}

C2DImage * CEngine::CreateUIImages(WCHAR* filename, int width, int height, int posX, int posY)
{
	return mpGraphics->CreateUIImages(filename, width, height, posX, posY);
}

bool CEngine::RemoveUIImage(C2DImage *& element)
{
	return mpGraphics->RemoveUIImage(element);
}

bool CEngine::UpdateTerrainBuffers(CTerrain *& terrain, double ** heightmap, int width, int height)
{
	return mpGraphics->UpdateTerrainBuffers(terrain, heightmap, width, height);
}

bool CEngine::ToggleFullscreen( unsigned int fullscreenKey)
{
	mpInput->KeyUp(fullscreenKey);
	return mpGraphics->SetFullscreen(!mpGraphics->IsFullscreen());
}

/* Create a primitive shape and place it in our world. For use with a texture and no diffuse lighting specified.*/
CPrimitive* CEngine::CreatePrimitive(WCHAR* textureFilename, PrioEngine::Primitives shape)
{
	return mpGraphics->CreatePrimitive(textureFilename, shape);
}

/* Creates an instance of a light object which is managed by the engine.
@Returns CLight* */
CLight * CEngine::CreateLight(D3DXVECTOR4 diffuseColour, D3DXVECTOR4 ambientColour)
{
	return mpGraphics->CreateLight(diffuseColour, ambientColour);
}

bool CEngine::RemoveLight(CLight *& light)
{
	return mpGraphics->RemoveLight(light);
}

CTerrain * CEngine::CreateTerrain(std::string mapFile)
{
	CTerrain* terrainPtr = mpGraphics->CreateTerrain(mapFile);

	return terrainPtr;
}

CTerrain * CEngine::CreateTerrain(double ** heightMap, int mapWidth, int mapHeight)
{
	CTerrain* terrainPtr = mpGraphics->CreateTerrain(heightMap, mapWidth, mapHeight);

	return terrainPtr;
}

bool CEngine::RemovePrimitive(CPrimitive * model)
{
	mpGraphics->RemovePrimitive(model);
	return mpGraphics->RemovePrimitive(model);
}

bool CEngine::RemoveMesh(CMesh * mesh)
{
	return mpGraphics->RemoveMesh(mesh);
}

CMesh* CEngine::LoadMesh(char * filename, WCHAR * textureFilename)
{
	return mpGraphics->LoadMesh(filename, textureFilename);
}

CMesh* CEngine::LoadMesh(char * filename, WCHAR * textureFilename, PrioEngine::ShaderType shaderType)
{
	return mpGraphics->LoadMesh(filename, textureFilename, shaderType);
}

/* Create a primitive shape and place it in our world, may pass in diffuse lighting boolean to indicate wether it should be used. */
CPrimitive* CEngine::CreatePrimitive(WCHAR* textureFilename, bool useLighting, PrioEngine::Primitives shape)
{
	return mpGraphics->CreatePrimitive(textureFilename, useLighting, shape);
}

/* Create a primitive shape and apply a solid colour to it. Acces colours through PrioEngine::Colour::*/
CPrimitive* CEngine::CreatePrimitive(PrioEngine::RGBA colour, PrioEngine::Primitives shape)
{
	return mpGraphics->CreatePrimitive(colour, shape);
}