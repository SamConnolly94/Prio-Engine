#include "Graphics.h"

CGraphics::CGraphics()
{
	// Initialise the Direct 3D class to null.
	mpD3D = nullptr;
	mpCamera = nullptr;
	mpTriangle = nullptr;
	mpColourShader = nullptr;
	mpTextureShader = nullptr;
	mpDiffuseLightShader = nullptr;
	mpTerrainShader = nullptr;
	mWireframeEnabled = false;
	mFieldOfView = static_cast<float>(D3DX_PI / 4);
	mpText = nullptr;
	mFullScreen = false;
	mpFrustum = nullptr;
	mpSkyboxShader = nullptr;
	mpWaterShader = nullptr;
	mpTerrain = nullptr;
	mpSkybox = nullptr;
	mFrameTime = 0.0f;
}

CGraphics::~CGraphics()
{
}

bool CGraphics::Initialise(int screenWidth, int screenHeight, HWND hwnd)
{
	bool successful;

	mHwnd = hwnd;

	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	// Create the Direct3D object.
	mpD3D = new CD3D11;
	// Check for successful creation of the object.
	if (!mpD3D)
	{
		// Output failure message to the log.
		logger->GetInstance().WriteLine("Did not successfully create the D3D11 object.");
		//Don't continue with the init function any more.
		return false;
	}
	logger->GetInstance().MemoryAllocWriteLine(typeid(mpD3D).name());

	// Initialise the D3D object.
	successful = mpD3D->Initialise(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, mFullScreen, SCREEN_DEPTH, SCREEN_NEAR);
	// If failed to init D3D, then output error.
	if (!successful)
	{
		// Write the error to the log.
		logger->GetInstance().WriteLine("Failed to initialised Direct3D.");
		// Write the error to a message box too.
		MessageBox(hwnd, "Could not initialise Direct3D.", "Error", MB_OK);
		// Do not continue with this function any more.
		return false;
	}

	// Create a colour shader now, it's necessary for terrain.
	CreateColourShader(hwnd);
	CreateTextureShaderForModel(hwnd);
	CreateTextureAndDiffuseLightShaderFromModel(hwnd);
	CreateTerrainShader(hwnd);

	mpFrustum = new CFrustum();

	mpCamera = CreateCamera();
	mpCamera->Render();

	/// SET UP TEXT FROM CAMERA POS.
	mpText = new CGameText();

	if (!mpText)
	{
		logger->GetInstance().WriteLine("Failed to allocate memory to the GameText var in Graphics.cpp.");
		return false;
	}

	D3DXMATRIX baseView;
	mpCamera->GetViewMatrix(baseView);
	if (!mpText->Initialise(mpD3D->GetDevice(), mpD3D->GetDeviceContext(), mHwnd, mScreenWidth, mScreenHeight, baseView))
	{
		logger->GetInstance().WriteLine("Failed to initailise the text object in graphics.cpp.");
		return false;
	}
	mBaseView = baseView;

	logger->GetInstance().WriteLine("Setting up AntTweakBar.");
	TwInit(TW_DIRECT3D11, mpD3D->GetDevice());
	TwWindowSize(mScreenWidth, mScreenHeight);
	logger->GetInstance().WriteLine("AntTweakBar successfully initialised. ");

	//////////////////////////////////////////////
	// Skybox shader

	mpSkyboxShader = new CSkyboxShader();
	if (!mpSkyboxShader)
	{
		logger->GetInstance().WriteLine("Failed to allocate memory to the skybox shader.");
		return false;
	}

	bool result = mpSkyboxShader->Initialise(mpD3D->GetDevice(), hwnd);
	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise the skybox shader.");
		return false;
	}
	D3DXVECTOR4 horizonColour = { 1.0f, 0.44f, 0.11f, 1.0f };

	// Set up the diffuse colour for the light.
	D3DXVECTOR4 diffuseColour = horizonColour;

	// Copy the colour of the horizon for the ambient colour.
	D3DXVECTOR4 ambientColour = horizonColour;

	const float ambientMultiplier = 0.4f;
	ambientColour.x *= ambientMultiplier;
	ambientColour.y *= ambientMultiplier;
	ambientColour.z *= ambientMultiplier;
	ambientColour.w = 1.0f;

	// Set the direction of our light.
	D3DXVECTOR3 direction(0.0f, 0.0f, 0.0f);

	// Set up specular vairables for our light.
	float specularPower = 0.01f;
	D3DXVECTOR4 specularColour = ambientColour;

	mpSceneLight = CreateLight(diffuseColour, specularColour, specularPower, ambientColour, direction);
	mpWaterLight = CreateLight(D3DXVECTOR4(1.0f, 0.7f, 0.1f, 1.0f) * 150.0f, D3DXVECTOR4(1.0f, 0.7f, 0.1f, 1.0f) * 150.0f, 100.0f, D3DXVECTOR4(1.0f, 0.7f, 0.1f, 1.0f) * 150.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	mpWaterLight->SetPos(0.0f, 20.0f, 50.0f);

	CreateSkybox(horizonColour);

	////////////////////////////
	// Water initialisation
	////////////////////////////

	mpWaterShader = new CWaterShader();
	if (!mpWaterShader->Initialise(mpD3D->GetDevice(), mHwnd))
	{
		logger->GetInstance().WriteSubtitle("Critical Error!");
		logger->GetInstance().WriteLine("Failed to initialise the water shader, shutting down.");
		logger->GetInstance().CloseSubtitle();

		return false;
	}

	mpRefractionShader = new CRefractionShader();

	if (!mpRefractionShader->Initialise(mpD3D->GetDevice(), hwnd))
	{
		logger->GetInstance().WriteSubtitle("Critical Error!");
		logger->GetInstance().WriteLine("Failed to initialise the refraction shader, shutting down.");
		logger->GetInstance().CloseSubtitle();
		return false;
	}

	mpWater = new CWater();

	//if (!mpWater->Initialise(mpD3D->GetDevice(), D3DXVECTOR3(-100.0f, 10.0f, 0.0f), D3DXVECTOR3(99.0f, 10.0f, 200.0f), 100, 100, "Resources/Textures/WaterNormalHeight.png", mScreenWidth, mScreenHeight))

	if (!mpWater->Initialise(mpD3D->GetDevice(), D3DXVECTOR3(-50.0f, 7.0f, 0.0f), D3DXVECTOR3(99.0f, 7.0f, 200.0f), 200, 200, "Resources/Textures/WaterNormalHeight.png", mScreenWidth, mScreenHeight))
	{
		logger->GetInstance().WriteLine("Failed to initialise the body of water.");
		return false;
	}

	// Success!
	logger->GetInstance().WriteLine("Direct3D was successfully initialised.");
	return true;
}

void CGraphics::Shutdown()
{
	if (mpWater != nullptr)
	{
		mpWater->Shutdown();
		delete mpWater;
	}

	if (mpRefractionShader)
	{
		mpRefractionShader->Shutdown();
		delete mpRefractionShader;
		mpRefractionShader = nullptr;
	}


	if (mpSkybox)
	{
		mpSkybox->Shutdown();
		delete mpSkybox;
		mpSkybox = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpSkybox).name());
	}

	if (mpSkyboxShader)
	{
		mpSkyboxShader->Shutdown();
		delete mpSkyboxShader;
		mpSkyboxShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpSkyboxShader).name());
	}

	if (mpWaterShader)
	{
		mpWaterShader->Shutdown();
		delete mpWaterShader;
		mpWaterShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpWaterShader).name());
	}

	if (mpText)
	{
		mpText->Shutdown();
		delete mpText;
		mpText = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpText).name());
	}

	for (auto image : mpUIImages)
	{
		image->Shutdown();
		delete image;
		image = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(image).name());
	}

	if (mpFrustum != nullptr)
	{
		delete mpFrustum;
	}

	mpUIImages.clear();

	if (mpDiffuseLightShader)
	{
		mpDiffuseLightShader->Shutdown();
		delete mpDiffuseLightShader;
		mpDiffuseLightShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpDiffuseLightShader).name());
	}

	if (mpTerrainShader)
	{
		mpTerrainShader->Shutdown();
		delete mpTerrainShader;
		mpTerrainShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpTerrainShader).name());
	}

	if (mpTextureShader)
	{
		mpTextureShader->Shutdown();
		delete mpTextureShader;
		mpTextureShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpTextureShader).name());
	}

	if (mpColourShader)
	{
		mpColourShader->Shutdown();
		delete mpColourShader;
		mpColourShader = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpColourShader).name());
	}

	// Deallocate any allocated memory on the primitives list.
	std::list<CPrimitive*>::iterator it;
	it = mpPrimitives.begin();

	while (it != mpPrimitives.end())
	{
		(*it)->Shutdown();
		delete (*it);
		(*it) = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid((*it)).name());
		it++;
	}

	while (!mpPrimitives.empty())
	{
		mpPrimitives.pop_back();
	}

	// Deallocate any allocated memroy on the mesh list.
	for (auto mesh : mpMeshes)
	{
		mesh->Shutdown();
		delete mesh;
	}
	mpMeshes.clear();

	if (mpSceneLight)
	{
		delete mpSceneLight;
		mpSceneLight = nullptr;
	}

	if (mpTerrain)
	{
		delete mpTerrain;
		mpTerrain = nullptr;
	}

	// Remove camera.

	if (mpCamera)
	{
		delete mpCamera;
		mpCamera = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpCamera).name());
	}

	// If the Direct 3D object exists.
	if (mpD3D)
	{
		// Clean up the D3D object before we get rid of it.
		mpD3D->Shutdown();
		delete mpD3D;
		mpD3D = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpD3D).name());
		// Output message to log to let us know that this object is gone.
		logger->GetInstance().WriteLine("Direct3D object has been shutdown, deallocated and pointer set to null.");
	}

	return;
}

bool CGraphics::Frame(float updateTime)
{
	bool success;

	UpdateScene(updateTime);

	// Render the graphics scene.
	success = Render();

	// If we did not successfully render the scene.
	if (!success)
	{
		// Output a message to the log.
		logger->GetInstance().WriteLine("Failed to render the scene.");
		// Prevent the program from continuing any further.
		return false;
	}
	return true;
}

void CGraphics::UpdateScene(float updateTime)
{
	mpCamera->Render();

	for (auto primitive : mpPrimitives)
	{
		primitive->UpdateMatrices();
	}

	if (mpTerrain)
	{
		mpTerrain->UpdateMatrices();
		if (mpWater)
		{
			//mpWater->SetYPos(mpTerrain->GetPosY() +  mpWater->GetDepth());
			mpWater->Update(updateTime);
			mpWater->UpdateMatrices();
		}
	}
}

bool CGraphics::Render()
{
	D3DXMATRIX viewMatrix;
	D3DXMATRIX worldMatrix;
	D3DXMATRIX projMatrix;
	D3DXMATRIX viewProj;
	D3DXMATRIX orthoMatrix;
	mpD3D->GetOrthogonalMatrix(orthoMatrix);

	// Set the back buffer as the render target
	mpD3D->SetBackBufferRenderTarget();
	mpD3D->GetDeviceContext()->ClearDepthStencilView(mpD3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Clear buffers so we can begin to render the scene.
	mpD3D->BeginScene(0.6f, 0.9f, 1.0f, 1.0f);

	// Get the world, view and projection matrices from the old camera and d3d objects.
	mpCamera->GetViewMatrix(viewMatrix);
	mpD3D->GetWorldMatrix(worldMatrix);
	mpD3D->GetProjectionMatrix(projMatrix);
	mpD3D->GetOrthogonalMatrix(orthoMatrix);
	mpCamera->GetViewProjMatrix(viewProj);

	mpFrustum->ConstructFrustum(SCREEN_DEPTH, projMatrix, viewMatrix);

	if (!RenderBitmaps(mBaseView, mBaseView, orthoMatrix))
		return false;

	if (!RenderSkybox(worldMatrix, viewMatrix, projMatrix))
		return false;

	if (!RenderModels(worldMatrix, viewMatrix, projMatrix))
		return false;

	if (!RenderText(worldMatrix, mBaseView, orthoMatrix))
		return false;

	TwDraw();

	// Present the rendered scene to the screen.
	mpD3D->EndScene();

	return true;
}

/* Renders any primitive shapes on the scene. */
bool CGraphics::RenderPrimitives(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	std::list<CPrimitive*>::iterator primitivesIt;
	primitivesIt = mpPrimitives.begin();

	while (primitivesIt != mpPrimitives.end())
	{
		(*primitivesIt)->GetWorldMatrix(world);

		// put the model vertex and index buffers on the graphics pipleline to prepare them for dawing.
		(*primitivesIt)->Render(mpD3D->GetDeviceContext());

		// Render texture with no light.
		if ((*primitivesIt)->HasTexture() && !(*primitivesIt)->UseDiffuseLight())
		{
			if (!RenderPrimitiveWithTexture((*primitivesIt), world, view, proj))
			{
				return false;
			}
		}
		// Render texture with light.
		else if ((*primitivesIt)->HasTexture() && (*primitivesIt)->UseDiffuseLight())
		{
			//if (!RenderPrimitiveWithTextureAndDiffuseLight((*primitivesIt), world, view, proj))
			//{
			//	return false;
			//}
		}
		// Render colour.
		else if ((*primitivesIt)->HasColour())
		{
			if (!RenderPrimitiveWithColour((*primitivesIt), world, view, proj))
			{
				return false;
			}
		}
		primitivesIt++;
	}

	return true;
}

/* Render any meshes / instances of meshes which we have created on the scene. */
bool CGraphics::RenderMeshes(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	// Render any models which belong to each mesh. Do this in batches to make it faster.
	for (auto mesh : mpMeshes)
	{
		mesh->Render(mpD3D->GetDeviceContext(), mpFrustum, mpDiffuseLightShader, view, proj, mpSceneLight);
	}

	return true;
}

/* Render the terrain and all areas inside of it. */
bool CGraphics::RenderTerrains(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	// If we haven't actually initialised our terrain yet.
	if (!mpTerrain)
	{
		// Output a message to the logger.
		logger->GetInstance().WriteLine("Terrain has not yet been initialised, skipping render pass.");
		// Completed.
		return true;
	}


	// Update the world matrix and perform operations on the world matrix of this object.
	mpTerrain->GetWorldMatrix(world);

	mpTerrain->Render(mpD3D->GetDeviceContext());

	// Iterate through each light that we have on our scene.
	if (mpSceneLight)
	{
		// Render the terrain area with the diffuse light shader.
		if (!mpTerrainShader->Render(mpD3D->GetDeviceContext(),
			mpTerrain->GetIndexCount(), world, view, proj,
			mpTerrain->GetTexturesArray(),
			mpTerrain->GetNumberOfTextures(),
			mpTerrain->GetGrassTextureArray(),
			mpTerrain->GetNumberOfGrassTextures(),
			mpTerrain->GetRockTextureArray(),
			mpTerrain->GetNumberOfRockTextures(),
			mpSceneLight->GetDirection(),
			mpSceneLight->GetDiffuseColour(),
			mpSceneLight->GetAmbientColour(),
			mpTerrain->GetHighestPoint(),
			mpTerrain->GetLowestPoint(),
			mpTerrain->GetPos(),
			mpTerrain->GetSnowHeight(),
			mpTerrain->GetGrassHeight(),
			mpTerrain->GetDirtHeight(),
			mpTerrain->GetSandHeight()
		))
		{
			logger->GetInstance().WriteSubtitle("Critical error.");
			logger->GetInstance().WriteLine("Failed to render the terrain with terrain render shader.");
			logger->GetInstance().CloseSubtitle();

			// If we failed to render, return false.
			return false;
		}
	}

	// Successfully rendered the terrain.
	return true;

}

bool CGraphics::RenderSkybox(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	if (!mpSkybox)
	{
		logger->GetInstance().WriteLine("No skybox exists, so skipping render pass for skybox.");
		return true;
	}

	D3DXVECTOR3 cameraPosition;

	// Get the position of the camera.
	cameraPosition = mpCamera->GetPosition();

	// Translate the sky dome to be centered around the camera position.
	D3DXMatrixTranslation(&world, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	// Turn off back face culling.
	mpD3D->TurnOffBackFaceCulling();

	// Turn off the Z buffer.
	mpD3D->DisableZBuffer();

	// Render the sky dome using the sky dome shader.
	mpSkybox->Render(mpD3D->GetDeviceContext());
	mpSkyboxShader->Render(mpD3D->GetDeviceContext(), mpSkybox->GetIndexCount(), world, view, proj,
		mpSkybox->GetApexColor(), mpSkybox->GetCenterColor());

	// Turn back face culling back on.
	mpD3D->TurnOnBackFaceCulling();

	// Turn the Z buffer back on.
	mpD3D->EnableZBuffer();

	// Reset the world matrix.
	mpD3D->GetWorldMatrix(world);

	return true;
}

/* Renders physical entities within the scene. */
bool CGraphics::RenderModels(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	if (!RenderWater(world, view, proj))
		return false;

	if (!RenderPrimitives(world, view, proj))
		return false;

	if (!RenderMeshes(world, view, proj))
		return false;

	if (!RenderTerrains(world, view, proj))
		return false;

	return true;
}

bool CGraphics::RenderText(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX ortho)
{
	mpD3D->DisableZBuffer();
	mpD3D->EnableAlphaBlending();

	bool result = mpText->Render(mpD3D->GetDeviceContext(), world, ortho);

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to render text.");
		return false;
	}


	mpD3D->EnableZBuffer();
	mpD3D->DisableAlphaBlending();

	return true;
}

bool CGraphics::RenderBitmaps(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX ortho)
{
	mpD3D->DisableZBuffer();
	mpD3D->EnableAlphaBlending();

	bool result;

	for (auto image : mpUIImages)
	{
		result = image->Render(mpD3D->GetDeviceContext(), image->GetX(), image->GetY());

		if (!result)
		{
			logger->GetInstance().WriteLine("Failed to render.");
			return false;
		}
	}

	for (auto image : mpUIImages)
	{
		result = mpTextureShader->Render(mpD3D->GetDeviceContext(), image->GetNumberOfIndices(), world, view, ortho, image->GetTexture());

		if (!result)
		{
			logger->GetInstance().WriteLine("Failed to render with texture shader.");
			return false;
		}
	}

	mpD3D->EnableZBuffer();
	mpD3D->DisableAlphaBlending();

	return true;
}

SentenceType * CGraphics::CreateSentence(std::string text, int posX, int posY, int maxLength)
{
	return mpText->CreateSentence(mpD3D->GetDevice(), mpD3D->GetDeviceContext(), text, posX, posY, maxLength);
}

bool CGraphics::UpdateSentence(SentenceType *& sentence, std::string text, int posX, int posY, PrioEngine::RGB colour)
{
	return mpText->UpdateSentence(sentence, text, posX, posY, colour.r, colour.g, colour.b, mpD3D->GetDeviceContext());
}

bool CGraphics::RemoveSentence(SentenceType *& sentence)
{
	return mpText->RemoveSentence(sentence);
}

C2DImage * CGraphics::CreateUIImages(std::string filename, int width, int height, int posX, int posY)
{
	/// Set up image.

	C2DImage* image = new C2DImage();

	bool successful = image->Initialise(mpD3D->GetDevice(), mScreenWidth, mScreenHeight, filename, width, height);

	image->SetX(posX);
	image->SetY(posY);

	if (!successful)
	{
		logger->GetInstance().WriteLine("Failed to initialise C2DSprite in Graphics.cpp.");
		return false;
	}
	logger->GetInstance().MemoryAllocWriteLine(typeid(image).name());

	// Push image to the list.
	mpUIImages.push_back(image);

	return image;

}

bool CGraphics::RemoveUIImage(C2DImage *& element)
{
	std::list<C2DImage*>::iterator it = mpUIImages.begin();

	while (it != mpUIImages.end())
	{
		if ((*it) == element)
		{
			(*it)->Shutdown();
			delete (*it);
			logger->GetInstance().MemoryDeallocWriteLine(typeid(*it).name());
			(*it) = nullptr;
			mpUIImages.erase(it);
			element = nullptr;
			return true;
		}
	}

	return false;
}

bool CGraphics::UpdateTerrainBuffers(CTerrain *& terrain, double ** heightmap, int width, int height)
{
	return terrain->UpdateBuffers(mpD3D->GetDevice(), mpD3D->GetDeviceContext(), heightmap, width, height);
}

bool CGraphics::IsFullscreen()
{
	return mFullScreen;
}

bool CGraphics::SetFullscreen(bool isEnabled)
{
	mFullScreen = isEnabled;

	// Set to windowed mode before shutting down or swap chain throws an exception.
	if (!mpD3D->ToggleFullscreen(mFullScreen))
		return false;

	return true;
}

CSkyBox * CGraphics::CreateSkybox(D3DXVECTOR4 ambientColour)
{
	CSkyBox* skybox;
	logger->GetInstance().WriteLine("Setting up skybox.");
	skybox = new CSkyBox();
	if (!skybox)
	{
		logger->GetInstance().WriteLine("Failed to allocate memory to skybox.");
		return false;
	}

	bool result = skybox->Initialise(mpD3D->GetDevice(), ambientColour);
	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to create the skybox.");
		return false;
	}

	// Deallocate the old one before copying it over.
	if (mpSkybox)
	{
		mpSkybox->Shutdown();
		delete mpSkybox;
		mpSkybox = nullptr;
		logger->GetInstance().WriteLine("Deallocating the skybox as a new one is being created, this should avoid memory leaks.");
	}

	mpSkybox = skybox;

	return skybox;
}

CWater * CGraphics::GetWater()
{
	return mpWater;
}


bool CGraphics::RenderWater(D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	bool result = true;

	// Reset the world matrix.
	mpD3D->GetWorldMatrix(world);
	mpD3D->GetProjectionMatrix(proj);
	mpCamera->Render();
	mpCamera->GetViewMatrix(view);

	/////////////////////////////////
	// Height
	////////////////////////////////

	D3DXMATRIX camWorld;
	mpWaterShader->SetMatrices(world, view, proj);
	mpWaterShader->SetWaterMovement(mpWater->GetMovement());
	mpWaterShader->SetWaveHeight(mpWater->GetWaveHeight());
	mpWaterShader->SetWaveScale(mpWater->GetWaveScale());
	mpWaterShader->SetDistortion(mpWater->GetRefractionDistortion(), mpWater->GetReflectionDistortion());
	mpWaterShader->SetMaxDistortion(mpWater->GetMaxDistortionDistance());
	mpWaterShader->SetRefractionStrength(mpWater->GetRefractionStrength());
	mpWaterShader->SetReflectionStrength(mpWater->GetReflectionStrength());
	mpCamera->GetWorldMatrix(camWorld);
	mpWaterShader->SetCameraMatrix(camWorld);
	mpWaterShader->SetCameraPosition(mpCamera->GetPosition());
	mpWaterShader->SetViewportSize(mScreenWidth, mScreenHeight);
	mpWaterShader->SetLightProperties(mpWaterLight);
	mpWaterShader->SetNormalMap(mpWater->GetNormalMap());

	// Set render target to the height texture map.
	mpWater->SetHeightMapRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView());
	
	mpWater->GetHeightTexture()->ClearRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 0.0f);
	mpD3D->GetDeviceContext()->ClearDepthStencilView(mpD3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Render the water height map.
	mpWater->Render(mpD3D->GetDeviceContext());
	mpWater->GetWorldMatrix(world);
	mpWaterShader->SetMatrices(world, view, proj);
	result = mpWaterShader->RenderHeight(mpD3D->GetDeviceContext(), mpWater->GetNumberOfIndices());

	if (!result)
	{
		return false;
	}

	// Two requirements for refraction and refleciton are a light and terrain.
	if (mpTerrain && mpSceneLight)
	{
		//////////////////////////////
		// Refraction
		//////////////////////////////

		mpD3D->GetWorldMatrix(world);
		// Reset the terrain world matrix
		mpTerrain->GetWorldMatrix(world);

		mpWater->SetRefractionRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView());
		mpWater->GetRefractionTexture()->ClearRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 0.0f);
		mpD3D->GetDeviceContext()->ClearDepthStencilView(mpD3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Place our refract / reflect properties into the refract reflect shader.
		mpRefractionShader->SetWorld(world);
		mpRefractionShader->SetView(view);
		mpRefractionShader->SetProj(proj);
		mpRefractionShader->SetLightProperties(mpSceneLight);
		mpRefractionShader->SetViewportProperties(mScreenWidth, mScreenHeight);
		mpRefractionShader->SetTerrainAreaProperties(mpTerrain->GetSnowHeight(), mpTerrain->GetGrassHeight(), mpTerrain->GetDirtHeight(), mpTerrain->GetSandHeight());
		mpRefractionShader->SetPositioningProperties(mpTerrain->GetPosY(), mpWater->GetPosY());
		mpRefractionShader->SetWaterHeightmap(mpWater->GetHeightTexture()->GetShaderResourceView());
		mpRefractionShader->SetDirtTextureArray(mpTerrain->GetTexturesArray());
		mpRefractionShader->SetGrassTextureArray(mpTerrain->GetGrassTextureArray());
		mpRefractionShader->SetPatchMap(mpTerrain->GetPatchMap());
		mpRefractionShader->SetRockTexture(mpTerrain->GetRockTextureArray());

		mpTerrain->Render(mpD3D->GetDeviceContext());
		result = mpRefractionShader->RefractionRender(mpD3D->GetDeviceContext(), mpTerrain->GetIndexCount());

		if (!result)
		{
			logger->GetInstance().WriteLine("Failed to render the refraction shader for water. ");
			return false;
		}

		/////////////////////////////////
		// Reflection
		/////////////////////////////////
		mpD3D->TurnOffBackFaceCulling();

		// Reset the terrain world matrix
		mpTerrain->GetWorldMatrix(world);
		
		mpWater->SetReflectionRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView());
		mpWater->GetReflectionTexture()->ClearRenderTarget(mpD3D->GetDeviceContext(), mpD3D->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 0.0f);
		mpD3D->GetDeviceContext()->ClearDepthStencilView(mpD3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		
		// Place vertices onto render pipeline.
		mpTerrain->Render(mpD3D->GetDeviceContext());
		
		// Render vertices using reflection shader.
		result = mpRefractionShader->ReflectionRender(mpD3D->GetDeviceContext(), mpTerrain->GetIndexCount());
		if (!result)
		{
			logger->GetInstance().WriteLine("Failed to render the reflection shader for water. ");
			return false;
		}

		mpD3D->TurnOnBackFaceCulling();
	}

	/////////////////////////////////
	// Water Model
	/////////////////////////////////

	mpD3D->SetBackBufferRenderTarget();
	mpD3D->GetDeviceContext()->ClearDepthStencilView(mpD3D->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	mpWater->Render(mpD3D->GetDeviceContext());

	//mpWaterShader->SetNormalMap(mpWater->GetNormalMap());
	mpWaterShader->SetRefractionMap(mpWater->GetRefractionTexture()->GetShaderResourceView());
	mpWaterShader->SetReflectionMap(mpWater->GetReflectionTexture()->GetShaderResourceView());

	result = mpWaterShader->RenderSurface(mpD3D->GetDeviceContext(), mpWater->GetNumberOfIndices());


	if (!result)
	{
		return false;
	}

	return true;
}

bool CGraphics::RenderPrimitiveWithColour(CPrimitive* model, D3DMATRIX worldMatrix, D3DMATRIX viewMatrix, D3DMATRIX projMatrix)
{
	bool success = false;

	// Render the model using the colour shader.
	success = mpColourShader->Render(mpD3D->GetDeviceContext(), model->GetIndex(), worldMatrix, viewMatrix, projMatrix);
	if (!success)
	{
		logger->GetInstance().WriteLine("Failed to render the model using the colour shader object.");
		return false;
	}

	return true;
}

bool CGraphics::RenderPrimitiveWithTexture(CPrimitive* model, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix)
{
	bool success = false;

	// Attempt to render the model with the texture specified.
	success = mpTextureShader->Render(mpD3D->GetDeviceContext(), model->GetIndex(), worldMatrix, viewMatrix, projMatrix, model->GetTexture());

	if (!success)
	{
		logger->GetInstance().WriteLine("Failed to render the model using the texture shader in graphics.cpp.");
		return false;
	}

	return true;
}

CPrimitive* CGraphics::CreatePrimitive(std::string TextureFilename, PrioEngine::Primitives shape)
{
	CPrimitive* model;
	bool successful;

	switch (shape)
	{
	case PrioEngine::Primitives::cube:
		model = new CCube(TextureFilename);
		logger->GetInstance().MemoryAllocWriteLine(typeid(model).name());
		break;
	case PrioEngine::Primitives::triangle:
		model = new CTriangle(TextureFilename);
		logger->GetInstance().MemoryAllocWriteLine(typeid(model).name());
		break;
	default:
		return nullptr;
	}

	if (!model)
	{
		logger->GetInstance().WriteLine("Failed to create the model object");
		return nullptr;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		logger->GetInstance().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, "Could not initialise the model object. ", "Error", MB_OK);
		return nullptr;
	}

	if (model->HasTexture() && model->UseDiffuseLight())
	{
		if (!CreateTextureAndDiffuseLightShaderFromModel(mHwnd))
			return nullptr;
	}
	else if (model->HasTexture())
	{
		if (!CreateTextureShaderForModel(mHwnd))
			return nullptr;
	}
	// Place any created models onto the list for the engine to track.
	mpPrimitives.push_back(model);

	return model;
}

CPrimitive* CGraphics::CreatePrimitive(std::string TextureFilename, bool useLighting, PrioEngine::Primitives shape)
{
	CPrimitive* model;
	bool successful;

	switch (shape)
	{
	case PrioEngine::Primitives::cube:
		model = new CCube(TextureFilename, useLighting);
		break;
	case PrioEngine::Primitives::triangle:
		model = new CTriangle(TextureFilename, useLighting);
		break;
	default:
		return nullptr;
	}


	logger->GetInstance().MemoryAllocWriteLine(typeid(model).name());
	if (!model)
	{
		logger->GetInstance().WriteLine("Failed to create the model object");
		return nullptr;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		logger->GetInstance().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, "Could not initialise the model object. ", "Error", MB_OK);
		return nullptr;
	}

	if (model->HasTexture() && model->UseDiffuseLight())
	{
		if (!CreateTextureAndDiffuseLightShaderFromModel(mHwnd))
			return nullptr;
	}
	else if (model->HasTexture())
	{
		if (!CreateTextureShaderForModel(mHwnd))
			return nullptr;
	}
	// Place any created models onto the list for the engine to track.
	mpPrimitives.push_back(model);

	return model;
}

CPrimitive* CGraphics::CreatePrimitive(PrioEngine::RGBA colour, PrioEngine::Primitives shape)
{
	CPrimitive* model;
	bool successful;

	switch (shape)
	{
	case PrioEngine::Primitives::cube:
		model = new CCube(colour);
		logger->GetInstance().MemoryAllocWriteLine(typeid(model).name());
		break;
	case PrioEngine::Primitives::triangle:
		model = new CTriangle(colour);
		logger->GetInstance().MemoryAllocWriteLine(typeid(model).name());
		break;
	default:
		return nullptr;
	}

	if (!model)
	{
		logger->GetInstance().WriteLine("Failed to create the model object");
		return nullptr;
	}

	// Initialise the model object.
	successful = model->Initialise(mpD3D->GetDevice());
	if (!successful)
	{
		logger->GetInstance().WriteLine("*** ERROR! *** Could not initialise the model object");
		MessageBox(mHwnd, "Could not initialise the model object. ", "Error", MB_OK);
		return nullptr;
	}

	if (!CreateColourShader(mHwnd))
		return nullptr;

	// Place any created models onto the list for the engine to track.
	mpPrimitives.push_back(model);

	return model;
}

bool CGraphics::CreateTextureAndDiffuseLightShaderFromModel(HWND hwnd)
{
	if (mpDiffuseLightShader == nullptr)
	{
		bool successful;

		// Create texture shader.
		mpDiffuseLightShader = new CDiffuseLightShader();
		logger->GetInstance().MemoryAllocWriteLine(typeid(mpDiffuseLightShader).name());
		if (!mpDiffuseLightShader)
		{
			logger->GetInstance().WriteLine("Failed to create the texture shader object in graphics.cpp.");
			return false;
		}

		// Initialise the texture shader object.
		successful = mpDiffuseLightShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			logger->GetInstance().WriteLine("Failed to initialise the texture shader object in graphics.cpp.");
			MessageBox(hwnd, "Could not initialise the texture shader object.", "Error", MB_OK);
			return false;
		}

	}
	return true;
}

bool CGraphics::CreateTerrainShader(HWND hwnd)
{
	if (mpTerrainShader == nullptr)
	{
		bool successful;

		// Create texture shader.
		mpTerrainShader = new CTerrainShader();
		logger->GetInstance().MemoryAllocWriteLine(typeid(mpTerrainShader).name());
		if (!mpTerrainShader)
		{
			logger->GetInstance().WriteLine("Failed to create the texture shader object in graphics.cpp.");
			return false;
		}

		// Initialise the texture shader object.
		successful = mpTerrainShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			logger->GetInstance().WriteLine("Failed to initialise the mpTerrainShader shader object in graphics.cpp.");
			MessageBox(hwnd, "Could not initialise the mpTerrainShader shader object.", "Error", MB_OK);
			return false;
		}

	}
	return true;
}


bool CGraphics::CreateTextureShaderForModel(HWND hwnd)
{
	if (mpTextureShader == nullptr)
	{
		bool successful;

		// Create texture shader.
		mpTextureShader = new CTextureShader();
		logger->GetInstance().MemoryAllocWriteLine(typeid(mpTextureShader).name());
		if (!mpTextureShader)
		{
			logger->GetInstance().WriteLine("Failed to create the texture shader object in graphics.cpp.");
			return false;
		}

		// Initialise the texture shader object.
		successful = mpTextureShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			logger->GetInstance().WriteLine("Failed to initialise the texture shader object in graphics.cpp.");
			MessageBox(hwnd, "Could not initialise the texture shader object.", "Error", MB_OK);
			return false;
		}

	}
	return true;
}

bool CGraphics::CreateColourShader(HWND hwnd)
{
	if (mpColourShader == nullptr)
	{
		bool successful;

		// Create the colour shader object.
		mpColourShader = new CColourShader();
		if (!mpColourShader)
		{
			logger->GetInstance().WriteLine("Failed to create the colour shader object.");
			return false;
		}
		logger->GetInstance().MemoryAllocWriteLine(typeid(mpColourShader).name());

		// Initialise the colour shader object.
		successful = mpColourShader->Initialise(mpD3D->GetDevice(), hwnd);
		if (!successful)
		{
			logger->GetInstance().WriteLine("*** ERROR! *** Could not initialise the colour shader object");
			MessageBox(hwnd, "Could not initialise the colour shader object. ", "Error", MB_OK);
			return false;
		}

	}
	return true;
}

bool CGraphics::RemovePrimitive(CPrimitive* &model)
{
	std::list<CPrimitive*>::iterator it;
	it = mpPrimitives.begin();

	while (it != mpPrimitives.end())
	{
		if ((*it) == model)
		{
			model->Shutdown();
			delete model;
			(*it) = nullptr;
			logger->GetInstance().MemoryDeallocWriteLine(typeid((*it)).name());
			mpPrimitives.erase(it);
			model = nullptr;
			return true;
		}

		it++;
	}

	logger->GetInstance().WriteLine("Failed to find model to delete.");
	return false;
}

CMesh* CGraphics::LoadMesh(std::string filename)
{
	// Allocate the mesh memory.
	CMesh* mesh = new CMesh(mpD3D->GetDevice());

	logger->GetInstance().MemoryAllocWriteLine(typeid(mesh).name());

	// If we failed to load the mesh, then delete the object and return a nullptr.
	if (!mesh->LoadMesh(filename))
	{
		// Deallocate memory.
		delete mesh;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mesh).name());
		logger->GetInstance().WriteLine("Failed to load the mesh with name " + filename);
		return nullptr;
	}

	// Push the pointer onto a member variable list so that we don't lose it.
	mpMeshes.push_back(mesh);

	return mesh;
}

/* Deletes any allocated memory to a mesh and removes it from the list. */
bool CGraphics::RemoveMesh(CMesh *& mesh)
{
	// Define an iterator.
	std::list<CMesh*>::iterator it;
	// Initialise it to the start of the meshes list.
	it = mpMeshes.begin();

	// Iterate through the list.
	while (it != mpMeshes.end())
	{
		// If the current position of our list is the same as the mesh parameter passed in.
		if ((*it) == mesh)
		{
			// Deallocate memory.
			delete mesh;
			// Reset the pointer to be null.
			(*it) = nullptr;
			// Output the memory deallocation message to the memory log.
			logger->GetInstance().MemoryDeallocWriteLine(typeid((*it)).name());
			// Erase this element off of the list.
			mpMeshes.erase(it);
			// Set the value of the parameter to be null as well, so we have NO pointers to this area of memory any more.
			mesh = nullptr;
			// Complete! Return success.
			return true;
		}
		// Increment the iterator.
		it++;
	}

	// If we got to this point, the mesh that was passed in was not found on the list. Output failure message to the log.
	logger->GetInstance().WriteLine("Failed to find mesh to delete.");

	// Return failure.
	return false;
}

CTerrain * CGraphics::CreateTerrain(std::string mapFile)
{
	if (mpTerrain)
	{
		logger->GetInstance().WriteLine("Found a previously initialised instance of terrain, deleting it now. It will be reinitialised without memory leaks.");
		delete mpTerrain;
		mpTerrain = nullptr;
	}

	CTerrain* terrain = new CTerrain(mpD3D->GetDevice());
	logger->GetInstance().WriteLine("Created terrain from the graphics object.");
	mpTerrain = terrain;

	// Check a map file was actually passed in.
	if (mapFile != "")
	{
		// Attempt to load the height map passed in.
		if (!terrain->LoadHeightMapFromFile(mapFile))
		{
			logger->GetInstance().WriteLine("Failed to load height map with name: " + mapFile);
		}
	}
	else
	{
		logger->GetInstance().WriteLine("No map file was passed in, not attempting to load.");
	}

	// Initialise the terrain.
	terrain->CreateTerrain(mpD3D->GetDevice());

	return terrain;
}

CTerrain * CGraphics::CreateTerrain(double ** heightMap, int mapWidth, int mapHeight)
{
	if (mpTerrain)
	{
		logger->GetInstance().WriteLine("Found a previously initialised instance of terrain, deleting it now. It will be reinitialised without memory leaks.");
		delete mpTerrain;
		mpTerrain = nullptr;
	}

	CTerrain* terrain = new CTerrain(mpD3D->GetDevice());
	logger->GetInstance().WriteLine("Created terrain from the graphics object.");
	mpTerrain = terrain;

	// Loading height map
	terrain->SetWidth(mapWidth);
	terrain->SetHeight(mapHeight);
	terrain->LoadHeightMap(heightMap);

	// Initialise the terrain.
	terrain->CreateTerrain(mpD3D->GetDevice());

	return terrain;
}

/* Create an instance of a light and return a pointer to it. */
CLight * CGraphics::CreateLight(D3DXVECTOR4 diffuseColour, D3DXVECTOR4 specularColour, float specularPower, D3DXVECTOR4 ambientColour, D3DXVECTOR3 direction)
{
	CLight* light = new CLight();
	if (!light)
	{
		// Output error string to the message log.
		logger->GetInstance().WriteLine("Failed to create the light object. ");
		return nullptr;
	}
	logger->GetInstance().MemoryAllocWriteLine(typeid(light).name());

	// Set the colour to our colour variable passed in.
	light->SetDiffuseColour(diffuseColour);
	light->SetAmbientColour(ambientColour);
	light->SetDirection(direction);
	light->SetSpecularColour(specularColour);
	light->SetSpecularPower(specularPower);

	// Output success message.
	logger->GetInstance().WriteLine("Light successfully created.");

	// Returns a pointer to the light.
	return light;
}

CCamera* CGraphics::CreateCamera()
{
	// Create the camera.
	mpCamera = new CCamera(mScreenWidth, mScreenWidth, mFieldOfView, SCREEN_NEAR, SCREEN_DEPTH);
	if (!mpCamera)
	{
		logger->GetInstance().WriteLine("Failed to create the camera for DirectX.");
		return nullptr;
	}
	logger->GetInstance().MemoryAllocWriteLine(typeid(mpCamera).name());

	// Set the initial camera position.
	mpCamera->SetPosition(0.0f, 0.0f, -1.0f);

	return mpCamera;
}

void CGraphics::SetCameraPos(float x, float y, float z)
{
	mpCamera->SetPosition(x, y, z);
}

void CGraphics::ToggleWireframe()
{
	mWireframeEnabled = !mWireframeEnabled;

	if (!mWireframeEnabled)
	{
		mpD3D->EnableSolidFill();
	}
	else
	{
		mpD3D->EnableWireframeFill();
	}
}
