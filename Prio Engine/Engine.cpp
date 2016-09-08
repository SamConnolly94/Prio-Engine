#include "Engine.h"

/* Default constructor. */
CEngine::CEngine()
{
	mInput = 0;
	mGraphics = 0;
}

/* Default destructor. */
CEngine::~CEngine()
{
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

	// Initialise the windows API.
	InitialiseWindows(screenWidth, screenHeight);

	// Initialise the input object. Used to read any input through keyboard or mouse from a user.
	mInput = new CInput();

	// Check that the input object has been successfully initialised.
	if (!mInput)
	{
		// Output failure message to the log.
		mpLogger->GetLogger().WriteLine("Failed to create the input object. ");
		
		// Prevent function from continuing any further, could not init input log.
		return false;
	}
	
	// Set up the input object for use.
	mInput->Initialise();

	mGraphics = new CGraphics();
	// Check to see if graphics object was created successfully.
	if (!mGraphics)
	{
		// Output error message to the log.
		mpLogger->GetLogger().WriteLine("Failed to create the graphics object. ");
		
		// Prevent function from continuing any further, could not init input log.
		return false;
	}

	// Initialise the graphics object
	result = mGraphics->Initialise(screenWidth, screenHeight, mHwnd);

	// Check that the graphics object was successfully initialised for use.
	if (!result)
	{
		// Output error message to the log.
		mpLogger->GetLogger().WriteLine("Failed to initialise the graphics object for use.");
		
		// Prevent the funciton from continuing any further.
		return false;
	}

	// Success! Initialised all essentials for our system.
	return true;
}

/* Clean up and free any memory our program has claimed. */
void CEngine::Shutdown()
{
	// Release the graphics object.
	if (mGraphics)
	{
		// Run the shutdown function for graphics.
		mGraphics->Shutdown();
		// Deallocate the memory given to the graphics object.
		delete mGraphics;
		// Reset the pointer to the graphics object to null.
		mGraphics = 0;
	}

	// Release the input object.
	if (mInput)
	{
		// Deallocate the memory given to the input object.
		delete mInput;
		// Reset the input object pointer to null.
		mInput = 0;
	}

	// Shutdown the window.
	ShutdownWindows();

	return;
}

/* Run our engine until we quit. */
void CEngine::Run()
{
	MSG msg;
	bool complete = false;
	bool result;

	// Initialise the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	while (!complete)
	{
		CheckWindowsMessages(msg);

		// Control what happens when windows signals to end application.
		complete = IsComplete(msg);

		// If we aren't complete yet, and don't need to process any windows messages, process away!
		if (!complete)
		{
			// Attempt to process the current frame.
			result = Frame();

			// If we failed to process the current frame then quit, something has gone wrong!
			if (!result)
			{
				complete = true;
			}
		}
	}

	return;
}

/* Handle messages from the OS. */
LRESULT CEngine::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_KEYDOWN:
		{
			mInput->KeyDown((unsigned int) wparam);
			return 0;
		}
		case WM_KEYUP:
		{
			mInput->KeyUp((unsigned int) wparam);
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

	// Check for user pressing escape
	if (mInput->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	// Process graphics for this frame;
	result = mGraphics->Frame();
	if (!result)
	{
		mpLogger->GetLogger().WriteLine("Failed to process the graphics for this frame. ");
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
	if (FULL_SCREEN)
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

		mpLogger->GetLogger().WriteLine("Successfully set the settings for fullscreen window.");
	}
	else
	{
		screenWidth = 1600;
		screenHeight = 900;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

		mpLogger->GetLogger().WriteLine("Successfully set the settings for windowed mode window.");
	}

	// Create the window.
	mHwnd = CreateWindowEx(WS_EX_APPWINDOW, mApplicationName, mApplicationName,
		WS_OVERLAPPEDWINDOW, /* other options include: WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP*/
		posX, posY, screenWidth, screenHeight, NULL, NULL, mHinstance, NULL);
	
	// Output message to the log.
	if (mHwnd)
	{
		mpLogger->GetLogger().WriteLine("Window created.");
	} 
	else
	{
		mpLogger->GetLogger().WriteLine("Window was not successfully created..");
	}

	// Focus upon the window.
	ShowWindow(mHwnd, SW_SHOW);
	SetForegroundWindow(mHwnd);
	SetFocus(mHwnd);
	mpLogger->GetLogger().WriteLine("Set focus upon the window.");

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
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
		mpLogger->GetLogger().WriteLine("Full screen display settings reset to defaults.");
	}

	// Remove the window.
	DestroyWindow(mHwnd);
	mHwnd = NULL;
	mpLogger->GetLogger().WriteLine("Window destroyed.");

	// Remove the application instance.
	UnregisterClass(mApplicationName, mHinstance);
	mHinstance = NULL;
	mpLogger->GetLogger().WriteLine("Application has been deregistered.");

	// Release the pointer to the class.
	ApplicationHandle = NULL;
	mpLogger->GetLogger().WriteLine("Application handle has been successfully released.");

	mpLogger->GetLogger().WriteLine("Shutdown of window successful.");
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
bool CEngine::IsComplete(MSG msg)
{
	// Control what happens when windows signals to end application.
	if (msg.message == WM_QUIT)
	{
		return true;
	}

	return false;
}