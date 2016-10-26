#include "Mesh.h"

CMesh::CMesh(ID3D11Device* device, HWND hwnd)
{
	mVertexCount  = 0;
	mIndexCount = 0;

	mpDevice = device;

	mpTextureShader = new CTextureShader();
	if (!mpTextureShader->Initialise(mpDevice, hwnd))
	{
		mpLogger->GetLogger().WriteLine("Failed to initialise texture shader in mesh object.");
	}

	mpTexture = nullptr;
	mpAssimpManager = new CAssimpManager();
}


CMesh::~CMesh()
{
	if (mpVertices)
	{
		delete[] mpVertices;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpVertices).name());
		mpVertices = nullptr;
	}

	// If we have pointers to models still.
	while (!mpModels.empty())
	{
		// Delete allocated memory from the back of our array.
		delete (mpModels.back());
		// Pop the model off of the list.
		mpModels.pop_back();
	}

	delete mpTextureShader;
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTextureShader).name());

	if (mpTexture)
	{
		delete mpTexture;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTexture).name());
	}

	if (mpUV)
	{
		delete[] mpUV;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpUV).name());
	}

	delete mpAssimpManager;
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpAssimpManager).name());

	delete[] mpIndices;
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpIndices).name());
	mpIndices = nullptr;
	
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
		mpLogger->GetLogger().WriteLine("You did not pass in a texture file name with a mesh, this might struggle a bit.");
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

void CMesh::Render(ID3D11DeviceContext* context, D3DXMATRIX &world, D3DXMATRIX &view, D3DXMATRIX &proj)
{
	std::list<CModel*>::iterator it = mpModels.begin();

	while (it != mpModels.end())
	{
		(*it)->UpdateMatrices(world);

		(*it)->RenderBuffers(context);

		// Our number of indices isn't quite accurate, we stash indicies away in vector 3's as we should always be creating a triangle. 
		if (!mpTextureShader->Render(context, (*it)->GetNumberOfIndices(), world, view, proj, mpTexture->GetTexture()))
		{
			mpLogger->GetLogger().WriteLine("Failed to render the mesh model.");
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
	model = new CModel(mpDevice);

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

	model->SetGeometry(mpVertices, mpIndices, mpUV);

	// Stick our models on a list to prevent losing the pointers.
	mpModels.push_back(model);

	//return model;
	return model;
}

/* Load a model using our assimp vertex manager. */
bool CMesh::LoadAssimpModel(char* filename)
{

	// Load the mesh into our scene using our manager.
	mpAssimpManager->LoadModelFromFile(filename);

	// Grab the mesh object for the last mesh we loaded.
	aiMesh* mesh = mpAssimpManager->GetLastLoadedMesh();
	
	// Allocate memory to the array we will store vertices in.
	mVertexCount = mesh->mNumVertices;
	mpVertices = new D3DXVECTOR3[mVertexCount];
	
	if (!mpVertices)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the vertices array.");
		return false;
	}

	mpUV = new D3DXVECTOR2[mVertexCount];

	// Copy vertices from the the assimp manager.
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		mpVertices[i].x = mesh->mVertices[i].x;
		mpVertices[i].y = mesh->mVertices[i].y;
		mpVertices[i].z = mesh->mVertices[i].z;

		// Parse the UV data while we're in the loop anyway.
		mpUV[i].x = mesh->mTextureCoords[0][i].x;
		mpUV[i].y = mesh->mTextureCoords[0][i].y;
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
		for (int i = 0; i < kNumIndicesInFace; i++)
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
