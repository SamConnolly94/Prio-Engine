#include "Mesh.h"

CMesh::CMesh(ID3D11Device* device, HWND hwnd)
{
	// Initialise our counter variables to the default values.
	mVertexCount  = 0;
	mIndexCount = 0;

	// Store the handle to our main window.
	mHwnd = hwnd;

	// Stash away a pointer to our device.
	mpDevice = device;

	// If we were successful, write to the memory allocation log.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpDirectionalLightShader).name());

	// Set the pointer to our texture to be null, we can use this for checks to see if we're using a texture or not later.
	mpTexture = nullptr;
	
	// Allocate memory to the manager of our assimp loader.
	mpAssimpManager = new CAssimpManager();
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpAssimpManager).name());
}


CMesh::~CMesh()
{
	// If the vertices array has been allocated memory.
	if (mpVertices)
	{
		// Delete allocated memory from the back of our array.
		delete[] mpVertices;
		// Write the deallocation message to the memory log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpVertices).name());
		// Set the pointer to the vertices array to be a default value.
		mpVertices = nullptr;
	}

	// If we still have pointers to models.
	while (!mpModels.empty())
	{
		// Delete allocated memory from the back of our array.
		delete (mpModels.back());
		// Pop the model off of the list.
		mpModels.pop_back();
	}

	// If the directional light shader has been allocated memory.
	if (mpDirectionalLightShader)
	{
		// Deallocate memory.
		delete mpDirectionalLightShader;
		// Write the deallocation message to the memory log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpDirectionalLightShader).name());
		// Set the directional light shader pointer to be default value.
		mpDirectionalLightShader = nullptr;
	}

	// If the texture has been allocated memory.
	if (mpTexture)
	{
		// Deallocate memory.
		delete mpTexture;
		// Output the deallocation message to the log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTexture).name());
		// Set the texture pointer to be a default value.
		mpTexture = nullptr;
	}

	// If the UV array has been allocated memory.
	if (mpUV)
	{
		// Deallocate any memory given to this array.
		delete[] mpUV;
		// Output the deallocation message to the log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpUV).name());
		// Set the pointer to the UV's array to be a default value.
		mpUV = nullptr;
	}

	// If the normals array has been allocated memory.
	if (mpNormals)
	{
		delete[] mpNormals;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpNormals).name());
		mpNormals = nullptr;
	}

	// If our assimp manager has been allocated memory.
	if (mpAssimpManager)
	{
		// Deallocate any memory given to this manager.
		delete mpAssimpManager;
		// Output the deallocation message to the log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpAssimpManager).name());
		// Set the assimp manager pointer to be a default value.
		mpAssimpManager = nullptr;
	}

	// If the indices array has been allocated memory.
	if (mpIndices)
	{
		// Deallocate memory given to the indices array.
		delete[] mpIndices;
		// Output the deallocation message to the log.
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpIndices).name());
		// Set the pointer to the indices array to be a default value.
		mpIndices = nullptr;
	}
}

/* Load data from file into our mesh object. */
bool CMesh::LoadMesh(char* filename, WCHAR* textureName)
{
	// If no file name was passed in, then output an error into the log and return false.
	if (filename == NULL || filename == "")
	{
		mpLogger->GetLogger().WriteLine("You need to pass in a file name to load a model.");
		return false;
	}

	// Check if a texture was passed in.
	if (textureName == NULL || textureName == L"")
	{
		mpLogger->GetLogger().WriteLine("You did not pass in a texture file name with a mesh named " + static_cast<std::string>(filename) + ", will load with solid black colour.");
	}
	else
	{
		mpTexture = new CTexture();
		mpTexture->Initialise(mpDevice, textureName);
	}

	// Stash our filename for this mesh away as a member variable, it may come in handy in future.
	mFilename = filename;

	// Extract the file extension.
	std::size_t extensionLocation = mFilename.find_last_of(".");
	mFileExtension = mFilename.substr(extensionLocation, mFilename.length());

	// Allocate memory to one of our shaders, depending on whether a texture was loaded in or not.
	if (mpTexture)
	{
		// Initialise our directional light shader.
		mpDirectionalLightShader = new CDirectionalLightShader();
		// If the directional light shader is not successfully initialised.
		if (!mpDirectionalLightShader->Initialise(mpDevice, mHwnd))
		{
			// Output failure message to the log.
			mpLogger->GetLogger().WriteLine("Failed to initialise the directional light shader in mesh object.");
		}
	}
	else
	{
		// Allocate memory to the colour shader.
		mpColourShader = new CColourShader();
		// If the colour shader is not successfully initialised.
		if (!mpColourShader->Initialise(mpDevice, mHwnd))
		{
			// output failure message to the log.
			mpLogger->GetLogger().WriteLine("Failed to initialise the colour shader in mesh object.");
		}
	}

	// Check what extension we are trying to load.
	if (mFileExtension == ".sam")
	{
		mpLogger->GetLogger().WriteLine("Loading .sam file using Prio Engines built in model loader.");
		return LoadSam();
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Loading " + mFileExtension + " file using assimp model loader.");
		return LoadAssimpModel(filename);
	}
		
	// Output error message to the log.
	mpLogger->GetLogger().WriteLine("You have tried to load an unsupported file type as a mesh. The file name was: '" + mFilename + "'.");
	// Return failure.
	return false;
}

void CMesh::Render(ID3D11DeviceContext* context, D3DXMATRIX &view, D3DXMATRIX &proj, std::list<CLight*>lights)
{
	std::list<CModel*>::iterator it = mpModels.begin();
	std::list<CLight*>::iterator lightIt = lights.begin();

	while (it != mpModels.end())
	{
		(*it)->UpdateMatrices();

		(*it)->RenderBuffers(context);

		while (lightIt != lights.end())
		{
			if (mpDirectionalLightShader)
			{
				// Our number of indices isn't quite accurate, we stash indicies away in vector 3's as we should always be creating a triangle. 
				if (!mpDirectionalLightShader->Render(context, (*it)->GetNumberOfIndices(), (*it)->GetWorldMatrix(), view, proj, mpTexture->GetTexture(), (*lightIt)->GetDirection(), (*lightIt)->GetDiffuseColour()))
				{
					mpLogger->GetLogger().WriteLine("Failed to render the mesh model.");
				}
				lightIt++;
			}
			else if (mpColourShader)
			{
				// Our number of indices isn't quite accurate, we stash indicies away in vector 3's as we should always be creating a triangle. 
				if (!mpColourShader->Render(context, (*it)->GetNumberOfIndices(), (*it)->GetWorldMatrix(), view, proj))
				{
					mpLogger->GetLogger().WriteLine("Failed to render the mesh model.");
				}
				lightIt++;
			}
			else
			{
				mpLogger->GetLogger().WriteLine("Failed to find any available shader to render the instance of mesh in mesh.cpp Render function.");
			}
		}

		it++;
	}
}

/* Create an instance of this mesh. 
@Returns CModel* ptr
 */
CModel* CMesh::CreateModel()
{
	// Allocate memory to a model.
	CModel* model;
	// Create a variable equal to a vertex type.
	PrioEngine::VertexType vt;
	// Initialise the vertex type.
	if (mpDirectionalLightShader)
	{
		vt = PrioEngine::VertexType::Diffuse;
	}
	else if (mpColourShader)
	{
		vt = PrioEngine::VertexType::Colour;
	}
	else
	{
		// Failed to discover what vertex type should be used, return a nullptr
		return nullptr;
	}
	model = new CModel(mpDevice, vt);

	// Write an allocation message to our memory log.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(model).name());

	// Check the model has had space allocated to it.
	if (model == nullptr)
	{
		mpLogger->GetLogger().WriteLine("Failed to allocate space to model. ");
		return nullptr;
	}

	model->SetNumberOfVertices(mVertexCount);
	model->SetNumberOfIndices(mIndexCount);

	// If we're using diffuse light.
	if (mpDirectionalLightShader)
	{
		model->SetGeometry(mpVertices, mpIndices, mpUV, mpNormals);
	}
	else if (mpColourShader)
	{
		model->SetGeometry(mpVertices, mpIndices);
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Failed to find a valid shader being used when creating instance of mesh. ");
		return nullptr;
	}
	// Stick our models on a list to prevent losing the pointers.
	mpModels.push_back(model);

	//return model;
	return model;
}

/* Load a model using our assimp vertex manager. 
@Returns bool Success*/
bool CMesh::LoadAssimpModel(char* filename)
{
	// Load the mesh into our scene using our manager.
	mpAssimpManager->LoadModelFromFile(filename);

	// Grab the mesh object for the last mesh we loaded.
	aiMesh* mesh = mpAssimpManager->GetLastLoadedMesh();
	
	// Acquire the number of vertices we will store.
	mVertexCount = mesh->mNumVertices;
	
	// Check that the mesh has a valid number of vertices, sometimes inconsistencies in loading will cause it not to.
	if (mVertexCount > kCuttoffSize)
	{
		mpLogger->GetLogger().WriteLine("You have either tried to load a very large model or assimp has failed to load correct details for whatever reason, refusing to load a model this large.");
		return false;
	}

	// Allocate memory to the array we will store vertices in.
	mpVertices = new D3DXVECTOR3[mVertexCount];
	
	// If we failed to allocate memory to the array.
	if (!mpVertices)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the vertices array.");
		return false;
	}

	// Allocate memory to the normals array.
	mpNormals = new D3DXVECTOR3[mVertexCount];

	// Allocate memory to the UV
	mpUV = new D3DXVECTOR2[mVertexCount];

	// Copy vertices from the the assimp manager.
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		mpVertices[i].x = mesh->mVertices[i].x;
		mpVertices[i].y = mesh->mVertices[i].y;
		mpVertices[i].z = mesh->mVertices[i].z;

		// Parse the UV data while we're in the loop anyway.
		if (mesh->mTextureCoords[0])
		{
			mpUV[i].x = mesh->mTextureCoords[0][i].x;
			mpUV[i].y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			mpUV[i].x = NULL;
			mpUV[i].y = NULL;
		}

		// Parse the normals too!
		if (mesh->HasNormals())
		{
			mpNormals[i].x = mesh->mNormals[i].x;
			mpNormals[i].y = mesh->mNormals[i].y;
			mpNormals[i].z = mesh->mNormals[i].z;
		}
		else
		{
			mpNormals[i].x = NULL;
			mpNormals[i].y = NULL;
			mpNormals[i].z = NULL;
		}
	}

	// We can predict there will be 3 indices in every face as they form a triangle, so multiple the faces by 3 to calculate our total number of indices.
	mIndexCount = mesh->mNumFaces * kNumIndicesInFace;

	// Allocate memory to the indices array.
	int indiceCurrIndex = 0;
	mpIndices = new unsigned long[mIndexCount];

	// Check our indices array was successfully initialised.
	if (!mpIndices)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the indices array.");
		return false;
	}
	
	// Copy indices over to our array.
	for (int faceCount = 0; faceCount < mesh->mNumFaces; faceCount++)
	{
		// Iterate through each index contained in this face.
		for (int i = 0; i < mesh->mFaces[faceCount].mNumIndices; i++)
		{
			// Copy the index from the face into our indices array.
			mpIndices[indiceCurrIndex] = mesh->mFaces[faceCount].mIndices[i];

			indiceCurrIndex++;
		}
	}


	// Success!
	return true;
}

bool CMesh::LoadSam()
{
	// Will find the size that our array should be.
	GetSizes();

	// Define the vertices array.
	mpVertices = new D3DXVECTOR3[mVertexCount];
	// Check memory was successfully allocated.
	if (!mpVertices)
	{
		// Write failure message to the log.
		mpLogger->GetLogger().WriteLine("Failed to allocate memory to mpVertices array.");
		// Don't continue with this function any further.
		return false;
	}
	// Output the message to indicate memory allocation to the logs.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpVertices).name());

	// Define the indices array
	mpIndices = new unsigned long[mIndexCount];
	// Check memory was successfully allocated.
	if (!mpIndices)
	{
		// Write failure message to the log.
		mpLogger->GetLogger().WriteLine("Failed to allocate memory to mpIndices array.");
		// Don't continue with this function any further.
		return false;
	}
	// Output the message to indicate memory allocation to the logs.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpIndices).name());

	// Will populate the new arrays we have created.
	InitialiseArrays();

	return true;
}

/* Will find how big an array should be depending on the object file. */
bool CMesh::GetSizes()
{
	// Define an input stream to open the mesh file in.
	std::ifstream inFile;

	int vertexIndex = 0;
	int texcoordIndex = 0;
	int normalIndex = 0;
	int faceIndex = 0;

	// Open the mesh file.
	inFile.open(mFilename);

	// Check if the file has been successfully opened.
	if (!inFile.is_open())
	{
		// Output error message to logs.
		mpLogger->GetLogger().WriteLine("Failed to find file of name: '" + mFilename + "'.");
		// Return failure.
		return false;
	}

	char ch;
	// Read the first character into our ch var.
	inFile.get(ch);
	// Iterate through the rest of the file.
	while (!inFile.eof())
	{
		// Skip any lines with comments on them.
		while (ch == '#')
		{
			while (ch != '\n' && !inFile.eof())
			{
				inFile.get(ch);
			}
			inFile.get(ch);
		}

		// If this line represents a vertex.
		if (ch == 'v')
		{
			inFile.get(ch);
			if (ch == ' ')
			{
				mVertexCount++;
			}
		}
		else if (ch == 'i')
		{
			inFile.get(ch);
			if (ch == ' ')
			{
				mIndexCount += 3;
			}
		}

		// Read the first character of the next line.
		inFile.get(ch);

		// Perform a check at the end of a loop, this will raise any boolean flags ready for the loop to do a logic test.
		inFile.eof();
	}

	// Finished with the file now, close it.
	inFile.close();

	return true;
}

/* Populate buffers with geometry data. */
bool CMesh::InitialiseArrays()
{
	// Define an input stream to open the mesh file in.
	std::ifstream inFile;

	int vertexIndex = 0;
	int indiceIndex = 0;

	// Open the mesh file.
	inFile.open(mFilename);

	// Check if the file has been successfully opened.
	if (!inFile.is_open())
	{
		// Output error message to logs.
		mpLogger->GetLogger().WriteLine("Failed to find file of name: '" + mFilename + "'.");
		// Return failure.
		return false;
	}

	char ch;
	// Read the first character into our ch var.
	inFile.get(ch);
	// Iterate through the rest of the file.
	while (!inFile.eof())
	{
		// Skip any lines with comments on them.
		while (ch == '#')
		{
			while (ch != '\n' && !inFile.eof())
			{
				inFile.get(ch);
			}
			inFile.get(ch);
		}

		// If this line represents a vertex.
		if (ch == 'v')
		{
			inFile.get(ch);
			if (ch == ' ')
			{
				// If memory has been allocated to the mpMatrices variable.
				if (mpVertices)
				{
					// Read in each value.
					inFile >> mpVertices[vertexIndex].x >> mpVertices[vertexIndex].y >> mpVertices[vertexIndex].z;

					// Increment our index.
					vertexIndex++;
				}
			}
			
		}
		else if (ch == 'i')
		{
			inFile.get(ch);
			if (ch == ' ')
			{
				// If memory has been allocated to the mpMatrices variable.
				if (mpIndices)
				{
					D3DXVECTOR3	indices;
					// Read in each value.
					inFile >> indices.x >> indices.y >> indices.z;

					mpIndices[indiceIndex] = indices.x;
					indiceIndex++;
					mpIndices[indiceIndex] = indices.y;
					indiceIndex++;
					mpIndices[indiceIndex] = indices.z;
					indiceIndex++;
				}
			}
		}

		// Read the first character of the next line.
		inFile.get(ch);

		// Perform a check at the end of a loop, this will raise any boolean flags ready for the loop to do a logic test.

	}

	// Finished with the file now, close it.
	inFile.close();

	return true;
}
