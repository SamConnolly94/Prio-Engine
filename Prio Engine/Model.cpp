#include "Model.h"

/*
* @PARAM filename - The location of the texture and it's file name and extension from the executable file.
*/
CModel::CModel(WCHAR* filename, PrioEngine::Primitives shape)
{
	// Initialise all variables to be null.
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpTexture = nullptr;
	mpTextureFilename = nullptr;
	ResetColour();
	mUseDiffuseLighting = false;

	// Store the filename for later use.
	mpTextureFilename = filename;

	mRotationX = 0.0f;
	mRotationY = 0.0f;
	mRotationZ = 0.0f;

	mPositionX = 0.0f;
	mPositionY = 0.0f;
	mPositionZ = 0.0f;
	
	mShape = shape;

	mpVertexManager = new CVertexManager(PrioEngine::VertexType::Texture, shape);
}

CModel::CModel(WCHAR* filename, bool useLighting, PrioEngine::Primitives shape)
{
	// Initialise all variables to be null.
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpTexture = nullptr;
	mpTextureFilename = nullptr;
	ResetColour();

	// Set the flag to use diffuse lighting on our texture.
	mUseDiffuseLighting = useLighting;

	// Store the filename for later use.
	mpTextureFilename = filename;

	mRotationX = 0.0f;
	mRotationY = 0.0f;
	mRotationZ = 0.0f;

	mPositionX = 0.0f;
	mPositionY = 0.0f;
	mPositionZ = 0.0f;

	mShape = shape;

	if (useLighting)
	{
		mpVertexManager = new CVertexManager(PrioEngine::VertexType::Diffuse, shape);
	}
	else
	{
		mpVertexManager = new CVertexManager(PrioEngine::VertexType::Texture, shape);
	}
}

CModel::CModel(PrioEngine::RGBA colour, PrioEngine::Primitives shape)
{
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpTexture = nullptr;
	mpTextureFilename = nullptr;
	mUseDiffuseLighting = false;
	ResetColour();

	// Store the colour which we have passed in.
	mColour = colour;

	mRotationX = 0.0f;
	mRotationY = 0.0f;
	mRotationZ = 0.0f;

	mPositionX = 0.0f;
	mPositionY = 0.0f;
	mPositionZ = 0.0f;

	mShape = shape;

	mpVertexManager = new CVertexManager(PrioEngine::VertexType::Texture, shape);

	mpVertexManager->SetColour(colour);
}


CModel::~CModel()
{
	delete mpVertexManager;
	mpVertexManager = nullptr;
}

bool CModel::Initialise(ID3D11Device * device)
{
	mpDevice = device;
	mpVertexManager->SetDevicePtr(device);
	bool result;
	mApplyTexture = mpTextureFilename != nullptr;

	// Initialise the vertex and index buffer that hold geometry for the triangle.
	result = InitialiseBuffers(device);
	if (!result)
		return false;

	// Load the texture for this model.

	if (mApplyTexture)
	{
		result = LoadTexture(device);
		if (!result)
		{
			return false;
		}
	}

	// Return our success / failure to init the vertex and index buffer.
	return true;
}

void CModel::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();

	// Release vertex and index buffers.
	ShutdownBuffers();
}

void CModel::Render(ID3D11DeviceContext * deviceContext)
{
	// Place the vertex and index bufdfers onto the pipeline so they are ready for drawing.
	RenderBuffers(deviceContext);
}

/* Retrieves the number of index's in the model, required for the pixel shader when using colour. */
int CModel::GetIndex()
{
	return mIndexCount;
}

ID3D11ShaderResourceView * CModel::GetTexture()
{
	return mpTexture->GetTexture();
}

/* Initialises the vertex and index buffers which hold geometry for a triangle.*/
bool CModel::InitialiseBuffers(ID3D11Device * device)
{

	unsigned long* indices;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mVertexCount = GetNumberOfVertices();
	mpVertexManager->SetNumberOfVertices(mVertexCount);
	
	// Set the number of indices on the index array.
	mIndexCount = GetNumberOfIndices();

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f);

	// Create the index array.
	indices = new unsigned long[mIndexCount];
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(indices).name());
	if (!indices)
	{
		return false;
	}

	// Load the index array with data according to the predefined indicies defined in the engines namespace.
	LoadIndiceData(indices);

	// Create the vertex buffer.
	if (!mpVertexManager->CreateVertexBuffer())
	{
		return false;
	}

	/* Set up the descriptor of the index buffer. */
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	/* Give the subresource structure a pointer to the index data. */
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	mpVertexManager->CleanArrays();

	delete[] indices;
	indices = nullptr; 
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(indices).name());

	return true;
}

int CModel::GetNumberOfIndices()
{
	// Find what shape we're using.
	switch (mShape)
	{
		// Cube
	case PrioEngine::Primitives::cube:
		return PrioEngine::Cube::kNumOfIndices;
		// Triangle
	case PrioEngine::Primitives::triangle:
		return PrioEngine::Triangle::kNumOfIndices;
	};

	return 0;
}

int CModel::GetNumberOfVertices()
{
	// Find what shape we're using.
	switch (mShape)
	{
		// Cube
	case PrioEngine::Primitives::cube:
		return PrioEngine::Cube::kNumOfVertices;
		// Triangle
	case PrioEngine::Primitives::triangle:
		return PrioEngine::Triangle::kNumOfVertices;
	};

	return 0;
}

void CModel::LoadIndiceData(unsigned long* &indices)
{
	switch (mShape)
	{
	case PrioEngine::Primitives::cube:
		for (int i = 0; i < PrioEngine::Cube::kNumOfIndices; i++)
		{
			indices[i] = PrioEngine::Cube::indices[i];
		}
		return;
	case PrioEngine::Primitives::triangle:
		for (int i = 0; i < PrioEngine::Triangle::kNumOfIndices; i++)
		{
			indices[i] = PrioEngine::Triangle::indices[i];
		}
		return;
	}
}

/* Release vertex and index buffers. */
void CModel::ShutdownBuffers()
{
	if (mpIndexBuffer)
	{
		mpIndexBuffer->Release();
		mpIndexBuffer = nullptr;
	}

	if (mpVertexBuffer)
	{
		mpVertexBuffer->Release();
		mpVertexBuffer = nullptr;
	}
}

/* Place the vertex and index bufdfers onto the pipeline so they are ready for drawing. */
void CModel::RenderBuffers(ID3D11DeviceContext * deviceContext)
{
	mpVertexManager->RenderBuffers(deviceContext, mpIndexBuffer);
}

bool CModel::LoadTexture(ID3D11Device * device)
{
	bool result;

	// Create the texture object.
	mpTexture = new CTexture();

	if (!mpTexture)
	{
		mpLogger->GetLogger().WriteLine("Failed to create the texture object.");
		return false;
	}

	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(mpTexture).name());

	// Initialise the texture object.
	result = mpTexture->Initialise(device, mpTextureFilename);

	if (!result)
	{
		mpLogger->GetLogger().WriteLine("Failed to initialise the texture object. ");
	}

	return true;
}

void CModel::ReleaseTexture()
{
	// Release the texture object.
	if (mpTexture)
	{
		mpTexture->Shutdown();
		delete mpTexture;
		mpTexture = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(mpTexture).name());
	}

	if (mpTextureFilename)
	{
		mpTextureFilename = nullptr;
	}
}

void CModel::ResetColour()
{
	// Initialise everything to more a value which is not the same as null, 0.0f is the same as null and can cause issues.
	mColour.r = -1.0f;
	mColour.g = -1.0f;
	mColour.b = -1.0f;
	mColour.a = -1.0f;
}

bool CModel::HasTexture()
{
	return mpTexture != nullptr;
}

bool CModel::HasColour()
{
	return mColour.r > -1.0f && mColour.g > -1.0f && mColour.b > -1.0f;
}

bool CModel::UseDiffuseLight()
{
	return mUseDiffuseLighting;
}

void CModel::RotateX(float x)
{
	mRotationX += x;
}

void CModel::RotateY(float y)
{
	mRotationY += y;
}

void CModel::RotateZ(float z)
{
	mRotationZ += z;
}

float CModel::GetRotationX()
{
	return mRotationX;
}

float CModel::GetRotationY()
{
	return mRotationY;
}

float CModel::GetRotationZ()
{
	return mRotationZ;
}

void CModel::SetRotationX(float x)
{
	mRotationX = x;
}

void CModel::SetRotationY(float y)
{
	mRotationY = y;
}

void CModel::SetRotationZ(float z)
{
	mRotationZ = z;
}

void CModel::MoveX(float x)
{
	mPositionX += x;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

void CModel::MoveY(float y)
{
	mPositionY += y;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

void CModel::MoveZ(float z)
{
	mPositionZ += z;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

float CModel::GetPosX()
{
	return mPositionX;
}

float CModel::GetPosY()
{
	return mPositionY;
}

float CModel::GetPosZ()
{
	return mPositionZ;
}

void CModel::SetXPos(float x)
{
	mPositionX = x;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

void CModel::SetYPos(float y)
{
	mPositionY = y;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

void CModel::SetZPos(float z)
{
	mPositionZ = z;

	// Make sure the triangle is redrawn in that new position.
	mpVertexManager->RepositionVertices(mPositionX, mPositionY, mPositionZ);
}

