#include "Mesh.h"


CMesh::CMesh(ID3D11Device* device, HWND hwnd)
{
	mVertexCount  = 0;

	mpDevice = device;

	mpColourShader = new CColourShader();
	mpColourShader->Initialise(mpDevice, hwnd);

	mpTexture = nullptr;
}


CMesh::~CMesh()
{
	if (mpVertices)
	{
		delete[] mpVertices;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpVertices).name());
		mpVertices = nullptr;
	}

	/*if (mpTexCoords)
	{
		delete[] mpTexCoords;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTexCoords).name());
		mpTexCoords = nullptr;
	}

	if (mpNormal)
	{
		delete[] mpNormal;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpNormal).name());
		mpNormal = nullptr;
	}

	if (mpFaces)
	{
		delete[] mpFaces;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpFaces).name());
		mpFaces = nullptr;
	}*/

	// If we have pointers to models still.
	while (!mpModels.empty())
	{
		// Delete allocated memory from the back of our array.
		delete (mpModels.back());
		// Pop the model off of the list.
		mpModels.pop_back();
	}

	delete mpColourShader;
	if (mpTexture)
	{
		delete mpTexture;
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
		return LoadSam();
	}
		
	// Output error message to the log.
	mpLogger->GetLogger().WriteLine("You have tried to load an unsupported file type as a mesh. The file name was: '" + mFilename + "'.");
	// Return failure.
	return false;
}

void CMesh::Render(ID3D11DeviceContext* context, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	std::list<CModels*>::iterator it = mpModels.begin();

	while (it != mpModels.end())
	{
		(*it)->UpdateMatrices(world, view, proj);

		(*it)->RenderBuffers(context);

		mpColourShader->Render(context, (*it)->GetNumberOfIndices(), world, view, proj);

		it++;
	}
}

CModels* CMesh::CreateModel()
{
	// Allocate memory to a model.
	CModels* model = nullptr;
	model = new CModels(mpDevice);

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

	model->SetGeometry(mpVertices, mpIndices);

	// Stick our models on a list to prevent losing the pointers.
	mpModels.push_back(model);

	//return model;
	return model;
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
	mpIndices = new D3DXVECTOR3[mIndexCount];
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
				mIndexCount++;
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

	char ch, ch2;
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

					// Invert Z vertex to allow for left hand system.
					//mpVertices[vertexIndex].z = mpVertices[vertexIndex].z * -1.0f;

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
					// Read in each value.
					inFile >> mpIndices[indiceIndex].x >> mpIndices[indiceIndex].y >> mpIndices[indiceIndex].z;

					//// Invert Z vertex to allow for left hand system.
					//mpIndices[indiceIndex].z = mpIndices[indiceIndex].z * -1.0f;

					// Increment our index.
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
