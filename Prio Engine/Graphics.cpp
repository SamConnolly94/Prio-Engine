#include "Graphics.h"

CGraphics::CGraphics()
{
	// Initialise the Direct 3D class to null.
	mpD3D = nullptr;
	mpCamera = nullptr;
	mpTriangle = nullptr;
	mpColourShader = nullptr;
	mpTextureShader = nullptr;
}

CGraphics::~CGraphics()
{
}

bool CGraphics::Initialise(int screenWidth, int screenHeight, HWND hwnd)
{
	bool successful;

	// Create the Direct3D object.
	mpD3D = new CD3D11;
	// Check for successful creation of the object.
	if (!mpD3D)
	{
		// Output failure message to the log.
		mpLogger->GetLogger().WriteLine("Did not successfully create the D3D11 object.");
		//Don't continue with the init function any more.
		return false;
	}

	// Initialise the D3D object.
	successful = mpD3D->Initialise(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
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

	// Create the camera.
	mpCamera = new CCamera();
	if (!mpCamera)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the camera for DirectX.");
		return false;
	}

	// Set the initial camera position.
	mpCamera->SetPosition(0.0f, 0.0f, -10.0f);
	
	// Create the model.
	float3 red;
	red.x = 1.0f;
	red.y = 0.0f;
	red.z = 0.0f;

	//mpTriangle = new CModel(L"../Resources/Textures/TriangleTex.dds");
	mpTriangle = new CModel(red);
	if (!mpTriangle)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the model object");
		return false;
	}

	// Initialise the model object.
	successful = mpTriangle->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(hwnd, L"Could not initialise the model object. ", L"Error", MB_OK);
		return false;
	}
	
	if (mpTriangle->HasTexture())
	{
		// Create texture shader.
		mpTextureShader = new CTextureShader();
		if (!mpTextureShader)
		{
			mpLogger->GetLogger().WriteLine("Failed to create the texture shader object in graphics.cpp.");
			return false;
		}

		// Initialise the texture shader object.
		successful = mpTextureShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			mpLogger->GetLogger().WriteLine("Failed to initialise the texture shader object in graphics.cpp.");
			MessageBox(hwnd, L"Could not initialise the texture shader object.", L"Error", MB_OK);
			return false;
		}
	}
	else if (mpTriangle->HasColour())
	{	
		// Create the colour shader object.
		mpColourShader = new CColourShader();
		if (!mpColourShader)
		{
			mpLogger->GetLogger().WriteLine("Failed to create the colour shader object.");
			return false;
		}
		
		// Initialise the colour shader object.
		successful = mpColourShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the colour shader object");
			MessageBox(hwnd, L"Could not initialise the colour shader object. ", L"Error", MB_OK);
			return false;
		}
	}

	// Success!
	mpLogger->GetLogger().WriteLine("Direct3D was successfully initialised.");
	return true;
}

void CGraphics::Shutdown()
{
	if (mpTextureShader)
	{
		mpTextureShader->Shutdown();
		delete mpTextureShader;
		mpTextureShader = nullptr;
	}

	if (mpColourShader)
	{
		mpColourShader->Shutdown();
		delete mpColourShader;
		mpColourShader = nullptr;
	}

	if (mpTriangle)
	{
		mpTriangle->Shutdown();
		delete mpTriangle;
		mpTriangle = nullptr;
	}

	if (mpCamera)
	{
		delete mpCamera;
		mpCamera = nullptr;
	}

	// If the Direct 3D object exists.
	if (mpD3D)
	{
		// Clean up the D3D object before we get rid of it.
		mpD3D->Shutdown();
		delete mpD3D;
		mpD3D = NULL;
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
	D3DXMATRIX viewMatrix;
	D3DXMATRIX worldMatrix;
	D3DXMATRIX projMatrix;
	bool result;

	// Clear buffers so we can begin to render the scene.
	mpD3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate view matrix based on cameras current position.
	mpCamera->Render();

	// Get the world, view and projection matrices from the old camera and d3d objects.
	mpCamera->GetViewMatrix(viewMatrix);
	mpD3D->GetWorldMatrix(worldMatrix);
	mpD3D->GetProjectionMatrix(projMatrix);

	// put the model vertex and index buffers on the graphics pipleline to prepare them for dawing.
	mpTriangle->Render(mpD3D->GetDeviceContext());

	// Render model using texture shader.
	if (mpTriangle->HasTexture())
	{
		if (!RenderModelWithTexture(mpTriangle, worldMatrix, viewMatrix, projMatrix))
		{
			return false;
		}
	}
	else if (mpTriangle->HasColour())
	{
		if (!RenderModelWithColour(mpTriangle, worldMatrix, viewMatrix, projMatrix))
		{
			return false;
		}
	}

	// Present the rendered scene to the screen.
	mpD3D->EndScene();

	return true;
}

bool CGraphics::RenderModelWithColour(CModel* model, D3DMATRIX worldMatrix, D3DMATRIX viewMatrix, D3DMATRIX projMatrix)
{
	bool success = false;

	// Render the model using the colour shader.
	success = mpColourShader->Render(mpD3D->GetDeviceContext(), mpTriangle->GetIndex(), worldMatrix, viewMatrix, projMatrix);
	if (!success)
	{
		mpLogger->GetLogger().WriteLine("Failed to render the model using the colour shader object.");
		return false;
	}
	
	return true;
}

bool CGraphics::RenderModelWithTexture(CModel* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix)
{
	bool success = false;

	// Attempt to render the model with the texture specified.
	success = mpTextureShader->Render(mpD3D->GetDeviceContext(), model->GetIndex(), worldMatrix, viewMatrix, projMatrix, model->GetTexture());

	if (!success)
	{
		mpLogger->GetLogger().WriteLine("Failed to render the model using the texture shader in graphics.cpp.");
		return false;
	}

	return true;
}