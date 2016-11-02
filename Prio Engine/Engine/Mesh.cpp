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

	// Initialise any of our class buffers to be null.
	mpVertices = nullptr;
	mpIndices = nullptr;
	mpVerticeColours = nullptr;
	mpUV = nullptr;
	mpNormals = nullptr;
	
	// Allocate memory to the manager of our assimp loader.
	mpAssimpManager = new CAssimpManager();
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpAssimpManager).name());

	// Default the shader type to colour, we can change this later on circumstantially.
	mShaderType = Colour;
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
		mShaderType = DirectionalLight;
	}

	// Stash our filename for this mesh away as a member variable, it may come in handy in future.
	mFilename = filename;

	// Extract the file extension.
	std::size_t extensionLocation = mFilename.find_last_of(".");
	mFileExtension = mFilename.substr(extensionLocation, mFilename.length());

	// Allocate memory to one of our shaders, depending on whether a texture was loaded in or not.
	if (mShaderType == DirectionalLight)
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
	else if (mShaderType == Colour)
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
			if (mShaderType == DirectionalLight)
			{
				// Our number of indices isn't quite accurate, we stash indicies away in vector 3's as we should always be creating a triangle. 
				if (!mpDirectionalLightShader->Render(context, (*it)->GetNumberOfIndices(), (*it)->GetWorldMatrix(), view, proj, mpTexture->GetTexture(), (*lightIt)->GetDirection(), (*lightIt)->GetDiffuseColour()))
				{
					mpLogger->GetLogger().WriteLine("Failed to render the mesh model.");
				}
				lightIt++;
			}
			else if (mShaderType == Colour)
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
	if (mShaderType == DirectionalLight)
	{
		vt = PrioEngine::VertexType::Diffuse;
	}
	else if (mShaderType == Colour)
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
		model->SetGeometry(mpVertices, mpIndices, mpVerticeColours);
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
	mpLogger->GetLogger().WriteSubtitle(mFilename);

	mpMesh = mpAssimpManager->LoadModelFromFile(filename);
	mpLogger->GetLogger().WriteLine("");
	mpLogger->GetLogger().WriteLine("Mesh just loaded in. ");
	mpLogger->GetLogger().WriteLine("Number of faces is : " + std::to_string(mpMesh->mNumFaces) + " and the number of indices in the first face is : " + std::to_string(mpMesh->mFaces[0].mNumIndices));
	mpLogger->GetLogger().WriteLine("Number of vertices is : " + std::to_string(mpMesh->mNumVertices));
	mpLogger->GetLogger().WriteLine("Texcoords in first face are : " + std::to_string(mpMesh->mTextureCoords[0][0].x ) + ", " + std::to_string(mpMesh->mTextureCoords[0][0].y));
	
	mpLogger->GetLogger().WriteLine("");
	if (mpMesh == nullptr)
	{
		mpLogger->GetLogger().WriteLine("Model loaded through assimp was a nullptr, not continuing with this method. ");
		MessageBox(mHwnd, L"Failed to load mesh. Check logs for more details.", L"Error", MB_OK);
		return false;
	}

	// Acquire the number of vertices we will store.
	mVertexCount = mpMesh->mNumVertices;
	mpLogger->GetLogger().WriteLine("The vertex count set in the mesh with name '" + mFilename + "' is equal to: " + std::to_string(mVertexCount));
	
	// Check that the mesh has a valid number of vertices, sometimes inconsistencies in loading will cause it not to.
	if (mVertexCount > kCuttoffSize)
	{
		mpLogger->GetLogger().WriteLine("You have either tried to load a very large model or assimp has failed to load correct details for whatever reason, refusing to load a model this large.");
		return false;
	}

	mNumFaces = mpMesh->mNumFaces;
	FaceStruct* indexArray = new FaceStruct[mNumFaces];
	mpLogger->GetLogger().WriteLine("The number of faces set in the mesh with name '" + mFilename + "' is equal to: " + std::to_string(mNumFaces));

	for (int i = 0; i < mNumFaces; i++)
	{
		indexArray[i].x = 0;
		indexArray[i].y = 0;
		indexArray[i].z = 0;
	}

	mpLogger->GetLogger().WriteLine("The index array has set all elements to null.");

	// Vertices may repeat, but that's okay. It'll save us hassle in the long run.
	mIndexCount = mNumFaces * kNumIndicesInFace;
	mpLogger->GetLogger().WriteLine("The index count set in the mesh with name '" + mFilename + "' is equal to: " + std::to_string(mIndexCount));

	// Allocate memory to the array we will store vertices in.
	mpVertices = new D3DXVECTOR3[mVertexCount];
	ZeroMemory(mpVertices, mVertexCount);
	mpLogger->GetLogger().WriteLine("The mpVertices array set in the mesh with name '" + mFilename + "' has been allocated " + std::to_string(mVertexCount) + " spaces in D3DXVector3 to use.");

	// If we failed to allocate memory to the array.
	if (!mpVertices)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the vertices array.");
		return false;
	}

	// Allocate memory to the normals array.
	mpNormals = new D3DXVECTOR3[mVertexCount];
	ZeroMemory(mpNormals, mVertexCount);
	mpLogger->GetLogger().WriteLine("The normals array set in the mesh with name '" + mFilename + "' has been allocated " + std::to_string(mVertexCount) + " spaces in D3DXVector3 to use.");

	// Allocate memory to the UV
	mpUV = new D3DXVECTOR2[mVertexCount];
	ZeroMemory(mpUV, mVertexCount);
	mpLogger->GetLogger().WriteLine("The UV array set in the mesh with name '" + mFilename + "' has been allocated " + std::to_string(mVertexCount) + " spaces in D3DXVector2 to use.");

	mpVerticeColours = new D3DXVECTOR4[mVertexCount];
	ZeroMemory(mpUV, mVertexCount);
	mpLogger->GetLogger().WriteLine("The colours array set in the mesh with name '" + mFilename + "' has been allocated " + std::to_string(mVertexCount) + " spaces in D3DXVector3 to use.");

	/** BUG HERE*******/
	// To load in vertices need to use mesh->mVertices[mesh->mFaces[meshFaceCount].mIndices[mIndiceCount]];
	unsigned int vertexCount = 0;

	for (unsigned int vertexCount = 0; vertexCount < mpMesh->mNumVertices; vertexCount++)
	{
		// Parse a singular vertex info.
		const aiVector3D vertexCoords = mpMesh->mVertices[vertexCount];

		// Load the vertex info into our array.
		mpVertices[vertexCount].x = vertexCoords.x;
		mpVertices[vertexCount].y = vertexCoords.y;
		mpVertices[vertexCount].z = vertexCoords.z;

		// Parse information on the UV of a singular vertex.
		const aiVector3D textureCoords = mpMesh->mTextureCoords[0][vertexCount];

		// Only need to parse the U and V channels.
		mpUV[vertexCount].x = textureCoords.x;
		mpUV[vertexCount].y = textureCoords.y;

		// Load the normals data of this singular vertex.
		const aiVector3D normals = mpMesh->mNormals[vertexCount];

		// Store this normals data in our array.
		mpNormals[vertexCount].x = normals.x;
		mpNormals[vertexCount].y = normals.y;
		mpNormals[vertexCount].z = normals.z;
	}

	mpLogger->GetLogger().WriteLine("Successfully initialised our arrays for mesh '" + mFilename + "'. ");

	for (unsigned int faceCount = 0; faceCount < mNumFaces; faceCount++)
	{
		const aiFace& face = mpMesh->mFaces[faceCount];
		mpLogger->GetLogger().WriteLine("face number " + std::to_string(faceCount) + " has " + std::to_string(face.mNumIndices) + " indices.");

		for (unsigned int index = 0; index < kNumIndicesInFace; index++)
		{

			// Store index data too!
			// Copy the index from the face into our indices array.
			if (index == 0)
			{
				indexArray[faceCount].x = face.mIndices[index];
			}
			else if (index == 1)
			{
				indexArray[faceCount].y = face.mIndices[index];
			}
			else if (index == 2)
			{
				indexArray[faceCount].z = face.mIndices[index];
			}


			// Increment the vertex count.
			vertexCount++;
		}
	}

	mpLogger->GetLogger().WriteLine("Successfully initialised index our arrays too for mesh '" + mFilename + "'. ");

	mpIndices = new unsigned long[mNumFaces * kNumIndicesInFace];
	unsigned int count = 0;
	for (int faceCount = 0; faceCount < mNumFaces; faceCount++)
	{
		for (int index = 0; index < kNumIndicesInFace; index++)
		{
			// Copy the index from the face into our indices array.
			if (index == 0)
			{
				mpIndices[count] = indexArray[faceCount].x;
			}
			else if (index == 1)
			{
				mpIndices[count] = indexArray[faceCount].y;
			}
			else if (index == 2)
			{
				mpIndices[count] = indexArray[faceCount].z;
			}
			count++;
		}
	}

	mpLogger->GetLogger().WriteLine("Moved data index data out of struct and into unsigned long array for mesh '" + mFilename + "'. ");

	mpLogger->GetLogger().CloseSubtitle();
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
					int index1, index2, index3;
					// Read in each value.
					inFile >> index1 >> index2 >> index3;

					mpIndices[indiceIndex] = index1;
					indiceIndex++;
					mpIndices[indiceIndex] = index2;
					indiceIndex++;
					mpIndices[indiceIndex] = index3;
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
