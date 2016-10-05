#include "Graphics.h"

CGraphics::CGraphics()
{
	// Initialise the Direct 3D class to null.
	mpD3D = nullptr;
	mpCamera = nullptr;
	mpTriangle = nullptr;
	mpColourShader = nullptr;
	mpTextureShader = nullptr;
	mpLight = nullptr;
	mpDiffuseLightShader = nullptr;
	mLightRotation = 0.0f;
}

CGraphics::~CGraphics()
{
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(this).name());
}

bool CGraphics::Initialise(int screenWidth, int screenHeight, HWND hwnd)
{
	bool successful;

	mHwnd = hwnd;

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
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpD3D).name());

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
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpCamera).name());

	// Set the initial camera position.
	mpCamera->SetPosition(0.0f, 0.0f, -10.0f);

	// Create the light object.
	mpLight = new CLight();
	if (!mpLight)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the diffuse light object.");
		return false;
	}
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpLight).name());

	// Define the colour to be used as diffuse lighting.
	float3 purple;
	purple.x = 1.0f;
	purple.y = 1.0f;
	purple.z = 1.0f;

	// Define the direction the light is pointed at.
	float3 direction;
	direction.x = 1.0f;
	direction.y = 1.0f;
	direction.z = 1.0f;

	// Set the properties of the light.
	mpLight->SetDiffuseColour(purple, 0.0f);
	mpLight->SetDirection(direction);

	// Success!
	mpLogger->GetLogger().WriteLine("Direct3D was successfully initialised.");
	return true;
}

void CGraphics::Shutdown()
{
	if (mpLight)
	{
		delete mpLight;
		mpLight = nullptr;
	}

	if (mpDiffuseLightShader)
	{
		mpDiffuseLightShader->Shutdown();
		delete mpDiffuseLightShader;
		mpDiffuseLightShader = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpDiffuseLightShader).name());
	}

	if (mpTextureShader)
	{
		mpTextureShader->Shutdown();
		delete mpTextureShader;
		mpTextureShader = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTextureShader).name());
	}

	if (mpColourShader)
	{
		mpColourShader->Shutdown();
		delete mpColourShader;
		mpColourShader = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpColourShader).name());
	}

	// Deallocate any allocated memory on the models list.
	std::list<CModel*>::iterator it;
	it = mpModels.begin();

	while (it != mpModels.end())
	{
		(*it)->Shutdown();
		delete (*it);
		(*it) = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid((*it)).name());
		it++;
	}

	while (!mpModels.empty())
	{
		mpModels.pop_back();
	}

	if (mpCamera)
	{
		delete mpCamera;
		mpCamera = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpCamera).name());
	}

	// If the Direct 3D object exists.
	if (mpD3D)
	{
		// Clean up the D3D object before we get rid of it.
		mpD3D->Shutdown();
		delete mpD3D;
		mpD3D = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpD3D).name());
		// Output message to log to let us know that this object is gone.
		mpLogger->GetLogger().WriteLine("Direct3D object has been shutdown, deallocated and pointer set to null.");
	}

	return;
}

bool CGraphics::Frame()
{
	bool success;

	mLightRotation += static_cast<float>(D3DX_PI) * 0.01f;
	if (mLightRotation > 360.0f)
	{
		mLightRotation = 0.0f;
	}

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
	mpD3D->BeginScene(0.5f, 0.0f, 0.0f, 1.0f);

	// Generate view matrix based on cameras current position.
	mpCamera->Render();

	// Get the world, view and projection matrices from the old camera and d3d objects.
	mpCamera->GetViewMatrix(viewMatrix);
	mpD3D->GetWorldMatrix(worldMatrix);
	mpD3D->GetProjectionMatrix(projMatrix);

	// Ritate the world by rotation value so the triangle spins.
	D3DXMatrixRotationY(&worldMatrix, mLightRotation);

	// Render model using texture shader.
	if (!RenderModels(viewMatrix, worldMatrix, projMatrix))
		return false;

	// Present the rendered scene to the screen.
	mpD3D->EndScene();

	return true;
}

bool CGraphics::RenderModels(D3DXMATRIX view, D3DXMATRIX world, D3DXMATRIX proj)
{
	std::list<CModel*>::iterator it;
	it = mpModels.begin();
	
	while (it != mpModels.end())
	{
		// put the model vertex and index buffers on the graphics pipleline to prepare them for dawing.
		(*it)->Render(mpD3D->GetDeviceContext());

		// Render texture with no light.
		if ((*it)->HasTexture() && !(*it)->UseDiffuseLight())
		{
			if (!RenderModelWithTexture((*it), world, view, proj))
			{
				return false;
			}
		}
		// Render texture with light.
		else if ((*it)->HasTexture() && (*it)->UseDiffuseLight())
		{
			if (!RenderModelsWithTextureAndDiffuseLight((*it), world, view, proj))
			{
				return false;
			}
		}
		// Render colour.
		else if ((*it)->HasColour())
		{
			if (!RenderModelWithColour((*it), world, view, proj))
			{
				return false;
			}
		}
		it++;
	}

	return true;
}

bool CGraphics::RenderModelsWithTextureAndDiffuseLight(CModel* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix)
{
	bool success = false;

	// Attempt to render the model with the texture specified.
	success = mpDiffuseLightShader->Render(mpD3D->GetDeviceContext(), model->GetIndex(), worldMatrix, viewMatrix, projMatrix, model->GetTexture(), mpLight->GetDirection(), mpLight->GetDiffuseColour());

	if (!success)
	{
		mpLogger->GetLogger().WriteLine("Failed to render the model using the texture shader in graphics.cpp.");
		return false;
	}

	return true;
}

bool CGraphics::RenderModelWithColour(CModel* model, D3DMATRIX worldMatrix, D3DMATRIX viewMatrix, D3DMATRIX projMatrix)
{
	bool success = false;

	// Render the model using the colour shader.
	success = mpColourShader->Render(mpD3D->GetDeviceContext(), model->GetIndex(), worldMatrix, viewMatrix, projMatrix);
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

bool CGraphics::CreateModel(CModel* &model, WCHAR* TextureFilename)
{
	bool successful;

	model = new CModel(TextureFilename);
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(model).name());
	if (!model)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the model object");
		return false;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, L"Could not initialise the model object. ", L"Error", MB_OK);
		return false;
	}

	if (model->HasTexture() && model->UseDiffuseLight())
	{
		if (!CreateTextureAndDiffuseLightShaderFromModel(model, mHwnd))
			return false;
	}
	else if (model->HasTexture())
	{
		if (!CreateTextureShaderForModel(model, mHwnd))
			return false;
	}
	// Place any created models onto the list for the engine to track.
	mpModels.push_back(model);

	return true;
}

bool CGraphics::CreateModel(CModel* &model, WCHAR* TextureFilename, bool useLighting)
{
	bool successful;

	model = new CModel(TextureFilename, useLighting);
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(model).name());
	if (!model)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the model object");
		return false;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, L"Could not initialise the model object. ", L"Error", MB_OK);
		return false;
	}

	if (model->HasTexture() && model->UseDiffuseLight())
	{
		if (!CreateTextureAndDiffuseLightShaderFromModel(model, mHwnd))
			return false;
	}
	else if (model->HasTexture())
	{
		if (!CreateTextureShaderForModel(model, mHwnd))
			return false;
	}
	// Place any created models onto the list for the engine to track.
	mpModels.push_back(model);

	return true;
}

bool CGraphics::CreateModel(CModel* &model, float3 colour)
{
	bool successful;

	model = new CModel(colour);
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(model).name());
	//mpTriangle = new CModel(red);
	if (!model)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the model object");
		return false;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, L"Could not initialise the model object. ", L"Error", MB_OK);
		return false;
	}

	if (!CreateColourShaderForModel(model, mHwnd))
		return false;

	// Place any created models onto the list for the engine to track.
	mpModels.push_back(model);

	return true;
}

bool CGraphics::CreateTextureAndDiffuseLightShaderFromModel(CModel* &model, HWND hwnd)
{
	if (mpDiffuseLightShader == nullptr)
	{
		bool successful;

		// Create texture shader.
		mpDiffuseLightShader = new CDiffuseLightShader();
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpDiffuseLightShader).name());
		if (!mpDiffuseLightShader)
		{
			mpLogger->GetLogger().WriteLine("Failed to create the texture shader object in graphics.cpp.");
			return false;
		}

		// Initialise the texture shader object.
		successful = mpDiffuseLightShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			mpLogger->GetLogger().WriteLine("Failed to initialise the texture shader object in graphics.cpp.");
			MessageBox(hwnd, L"Could not initialise the texture shader object.", L"Error", MB_OK);
			return false;
		}

	}
	return true;
}

bool CGraphics::CreateTextureShaderForModel(CModel* &model, HWND hwnd)
{
	if (mpTextureShader == nullptr)
	{
		bool successful;

		// Create texture shader.
		mpTextureShader = new CTextureShader();
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpTextureShader).name());
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
	return true;
}

bool CGraphics::CreateColourShaderForModel(CModel* &model, HWND hwnd)
{
	if (mpColourShader == nullptr)
	{
		bool successful;

		// Create the colour shader object.
		mpColourShader = new CColourShader();
		if (!mpColourShader)
		{
			mpLogger->GetLogger().WriteLine("Failed to create the colour shader object.");
			return false;
		}
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpColourShader).name());

		// Initialise the colour shader object.
		successful = mpColourShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			mpLogger->GetLogger().WriteLine("*** ERROR! *** Could not initialise the colour shader object");
			MessageBox(hwnd, L"Could not initialise the colour shader object. ", L"Error", MB_OK);
			return false;
		}

	}
	return true;
}

bool CGraphics::RemoveModel(CModel* &model)
{
	std::list<CModel*>::iterator it;
	it = mpModels.begin();

	while (it != mpModels.end())
	{
		if ((*it) == model)
		{
			model->Shutdown();
			delete model;
			(*it) = nullptr;
			mpLogger->GetLogger().MemoryDeallocWriteLine(typeid((*it)).name());
			mpModels.erase(it);
			model = nullptr;
			return true;
		}

		it++;
	}

	mpLogger->GetLogger().WriteLine("Failed to find model to delete.");
	return false;
}