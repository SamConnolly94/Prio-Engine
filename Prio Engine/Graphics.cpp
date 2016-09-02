#include "Graphics.h"

CGraphics::CGraphics()
{
	// Initialise the Direct 3D class to null.
	mD3D = NULL;
}

CGraphics::~CGraphics()
{
}

bool CGraphics::Initialise(int screenWidth, int screenHeight, HWND hwnd)
{
	bool successful;

	// Create the Direct3D object.
	mD3D = new CD3D11;
	// Check for successful creation of the object.
	if (!mD3D)
	{
		// Output failure message to the log.
		mpLogger->GetLogger().WriteLine("Did not successfully create the D3D11 object.");
		//Don't continue with the init function any more.
		return false;
	}

	// Initialise the D3D object.
	successful = mD3D->Initialise(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	// If failed to init D3D, then output error.
	if (!successful)
	{
		// Write the error to the log.
		mpLogger->GetLogger().WriteLine("Failed to initialised Direct3D.");
		// Write the error to a message box too.
		MessageBox(hwnd, L"Could not initialise Direct3D.", L"Error", MB_OK);
		// Do not continue with this function any more.
		return false;
	}

	// Success!
	mpLogger->GetLogger().WriteLine("Direct3D was successfully initialised.");
	return true;
}

void CGraphics::Shutdown()
{
	// If the Direct 3D object exists.
	if (mD3D)
	{
		// Clean up the D3D object before we get rid of it.
		mD3D->Shutdown();
		delete mD3D;
		mD3D = NULL;
		// Output message to log to let us know that this object is gone.
		mpLogger->GetLogger().WriteLine("Direct3D object has been shutdown, deallocated and pointer set to null.");
	}

	return;
}

bool CGraphics::Frame()
{
	bool success;

	// Render the graphics scene.
	success = Render();

	// If we did not successfully render the scene.
	if (!success)
	{
		// Output a message to the log.
		mpLogger->GetLogger().WriteLine("Failed to render the scene.");
		// Prevent the program from continuing any further.
		return false;
	}
	return true;
}

bool CGraphics::Render()
{
	// Clear buffers so we can begin to render the scene.
	mD3D->BeginScene(0.8f, 0.4f, 0.5f, 1.0f);

	// Present the rendered scene to the screen.
	mD3D->EndScene();

	return true;
}
