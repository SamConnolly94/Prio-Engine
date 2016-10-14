#include "Mesh.h"



CMesh::CMesh(ID3D11Device* device)
{
	mVertexCount  = 0;
	mFaceCount    = 0;
	mNormalCount  = 0;
	mTextureCount = 0;

	mpDevice = device;
}


CMesh::~CMesh()
{
	if (mpVertices)
	{
		delete[] mpVertices;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpVertices).name());
		mpVertices = nullptr;
	}

	if (mpTexCoords)
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
	}

	//// If we have pointers to models still.
	//while (!mpModels.empty())
	//{
	//	// Delete allocated memory from the back of our array.
	//	delete (mpModels.back());
	//	// Pop the model off of the list.
	//	mpModels.pop_back();
	//}
}

/* Load data from file into our mesh object. */
bool CMesh::LoadMesh(char* filename)
{
	// If no file name was passed in, then output an error into the log and return false.
	if (filename == NULL || filename == "")
	{
		mpLogger->GetLogger().WriteLine("You need to pass in a file name to load a model.");
		return false;
	}

	// Stash our filename for this mesh away as a member variable, it may come in handy in future.
	mFilename = filename;

	// Extract the file extension.
	std::size_t extensionLocation = mFilename.find_last_of(".");
	mFileExtension = mFilename.substr(extensionLocation, mFilename.length());

	// Check what extension we are trying to load.
	if (mFileExtension == ".obj")
	{
		return LoadObj();
	}
		
	// Output error message to the log.
	mpLogger->GetLogger().WriteLine("You have tried to load an unsupported file type as a mesh. The file name was: '" + mFilename + "'.");
	// Return failure.
	return false;
}

void CMesh::CreateModel()
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
		//return false;
	}

	model->SetNumberOfVertices(mVertexCount);
	model->SetNumberOfNormals(mNormalCount);
	model->SetTextureCount(mTextureCount);
	model->SetNumberOfIndices(mVertexCount);

	model->SetGeometry(mpVertices, mpTexCoords, mpNormal);

	// Stick our models on a list to prevent losing the pointers.
	//mpModels.push_back(model);

	//return model;
}

bool CMesh::LoadObj()
{
	// Will find the size that our array should be.
	InitialiseArrays();

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

	// Allocate memory to the texture coords array.
	mpTexCoords = new D3DXVECTOR3[mTextureCount];
	
	// Check memory was successfully allocated.
	if (!mpTexCoords)
	{
		// Output failure message to logs.
		mpLogger->GetLogger().WriteLine("Failed to allocate memory to mpTexCoords array.");
		// Don't continue with this function any further.
		return false;
	}
	// Output allocation message to the memory logs.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpTexCoords).name());

	// Allocate memory to the normal array.
	mpNormal = new D3DXVECTOR3[mNormalCount];

	if (!mpNormal)
	{
		// Write failure message to the log.
		mpLogger->GetLogger().WriteLine("Failed to allocate memory to mpNormal array.");
		// Don't continue with this function any further.
		return false;
	}
	// Output the message to indicate memory allocation to the logs.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpNormal).name());

	// Allocate memory to the faces array.
	mpFaces = new FaceType[mFaceCount];

	if (!mpFaces)
	{
		// Write failure message to the log.
		mpLogger->GetLogger().WriteLine("Failed to allocate memory to mpFaces array.");
		// Don't continue with this function any further.
		return false;
	}
	// Output the message to indicate memory allocation to the logs.
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpFaces).name());

	// Will populate the new arrays we have created.
	InitialiseArrays();

	return true;
}

/* Will populate any arrays or find out the size of how big an array should be depending on the object file. */
bool CMesh::InitialiseArrays()
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

	char ch, ch2;
	// Read the first character into our ch var.
	inFile.get(ch);
	// Iterate through the rest of the file.
	while (!inFile.eof())
	{
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
					mpVertices[vertexIndex].z = mpVertices[vertexIndex].z * -1.0f;

					// Increment our index.
					vertexIndex++;
				}
				else
				{
					// Just count how many vertex's we found.
					mVertexCount++;
				}
			}
			if (ch == 't')
			{
				// If memory has been allocated to the tex coords array.
				if (mpTexCoords)
				{
					inFile >> mpTexCoords[texcoordIndex].x >> mpTexCoords[texcoordIndex].y;

					// Invert the V axis for LH system.
					mpTexCoords[texcoordIndex].y = 1.0f - mpTexCoords[texcoordIndex].y;
					
					// Increment the index.
					texcoordIndex++;
				}
				else
				{
					mTextureCount++;
				}
			}
			if (ch == 'n')
			{
				// If memory has been allocated to the normals array.
				if (mpNormal)
				{
					inFile >> mpNormal[normalIndex].x >> mpNormal[normalIndex].y >> mpNormal[normalIndex].z;

					// Invert the Z axis for LH system.
					mpNormal[normalIndex].y = mpNormal[normalIndex].z * -1.0f;

					// Increment the index.
					normalIndex++;
				}
				else
				{
					mNormalCount++;
				}
			}
		}

		// If this line represents a face.
		if (ch == 'f')
		{
			inFile.get(ch);
			if (ch == ' ')
			{
				if (mpFaces)
				{
					inFile	>> mpFaces[faceIndex].vIndex3 >> ch2 >> mpFaces[faceIndex].nIndex3 >> ch2 >> mpFaces[faceIndex].nIndex3
							>> mpFaces[faceIndex].vIndex2 >> ch2 >> mpFaces[faceIndex].nIndex2 >> ch2 >> mpFaces[faceIndex].nIndex2
							>> mpFaces[faceIndex].vIndex1 >> ch2 >> mpFaces[faceIndex].nIndex1 >> ch2 >> mpFaces[faceIndex].nIndex1;
					faceIndex++;
				}
				else
				{
					mFaceCount++;
				}
			}
		}

		// Skip any blank lines, we don't care about those.
		while (ch != '\n')
		{
			inFile.get(ch);
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
