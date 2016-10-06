#include "Model.h"

/*
* @PARAM filename - The location of the texture and it's file name and extension from the executable file.
*/
CModel::CModel(WCHAR* filename)
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
}

CModel::CModel(WCHAR* filename, bool useLighting)
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
}

CModel::CModel(float3 colour)
{
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpTexture = nullptr;
	mpTextureFilename = nullptr;
	mUseDiffuseLighting = false;
	ResetColour();

	// Store the colour which we have passed in.
	mColour = colour;
}


CModel::~CModel()
{
}

bool CModel::Initialise(ID3D11Device * device)
{
	mpDevice = device;
	bool result;
	bool applyTexture = mpTextureFilename != nullptr;

	// Initialise the vertex and index buffer that hold geometry for the triangle.
	result = InitialiseBuffers(device, applyTexture);
	if (!result)
		return false;

	// Load the texture for this model.

	if (applyTexture)
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
bool CModel::InitialiseBuffers(ID3D11Device * device, bool applyTexture)
{
	VertexTextureType* verticesTexture = nullptr;
	VertexColourType* verticesColour = nullptr;
	VertexDiffuseLightingType* verticesDiffuse = nullptr;

	unsigned long* indices;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mVertexCount = 3;
	
	// Set the number of indices on the index array.
	mIndexCount = 3;

	// Create a vertex array
	CreateVertexArray(verticesTexture, verticesColour, verticesDiffuse, applyTexture);

	// Create the points of the triangle.
	SetTriangleBuffers(verticesColour, verticesTexture, verticesDiffuse, applyTexture, 0.0f, 0.0f, 0.0f);

	// Create the index array.
	indices = new unsigned long[mIndexCount];
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(indices).name());
	if (!indices)
	{
		return false;
	}


	/* Load index array with data. */
	
	// Bottom left.
	indices[0] = 0;
	
	// Top middle.
	indices[1] = 1;

	// Bottom right.
	indices[2] = 2;
	
	if (!CreateVertexBuffer(verticesTexture, verticesColour, verticesDiffuse))
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

	CleanVertexBuffers(verticesTexture, verticesColour, verticesDiffuse);

	delete[] indices;
	indices = nullptr; 
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(indices).name());

	return true;
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
	unsigned int stride;
	unsigned int offset;

	// Set the vertex buffer stride and offset.
	if (HasTexture() && !UseDiffuseLight())
	{
		stride = sizeof(VertexTextureType);
	}
	else if (HasTexture() && UseDiffuseLight())
	{
		stride = sizeof(VertexDiffuseLightingType);
	}
	else if (HasColour())
	{
		stride = sizeof(VertexColourType);
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Neither texture nor colour is being used when rendered. You're probably going to crash here when attempting to render.");
	}
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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

bool CModel::SetTriangleBuffers(VertexColourType* &verticesColour, VertexTextureType* &verticesTexture, VertexDiffuseLightingType* &verticesDiffuse, bool applyTexture,
	float x, float y, float z)
{
	/* Set the vertex points for the triangle. */
	if (applyTexture && !mUseDiffuseLighting)
	{
		// Bottom left
		verticesTexture[0].position = D3DXVECTOR3(x - 1.0f, y - 1.0f, z);
		verticesTexture[0].texture = D3DXVECTOR2(0.0f, 1.0f);
		// Top middle
		verticesTexture[1].position = D3DXVECTOR3(x, y + 1.0f, z);
		verticesTexture[1].texture = D3DXVECTOR2(0.5f, 0.0f);
		// Bottom right
		verticesTexture[2].position = D3DXVECTOR3(x + 1.0f, y - 1.0f, z);
		verticesTexture[2].texture = D3DXVECTOR2(1.0f, 1.0f);
	}
	else if (applyTexture && mUseDiffuseLighting)
	{
		// Bottom left
		verticesDiffuse[0].position = D3DXVECTOR3(x - 1.0f, y - 1.0f, z);
		verticesDiffuse[0].texture = D3DXVECTOR2(0.0f, 1.0f);
		verticesDiffuse[0].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		// Top middle
		verticesDiffuse[1].position = D3DXVECTOR3(x, y + 1.0f, z);
		verticesDiffuse[1].texture = D3DXVECTOR2(0.5f, 0.0f);
		verticesDiffuse[1].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

		// Bottom right
		verticesDiffuse[2].position = D3DXVECTOR3(x + 1.0f, y - 1.0f, z);
		verticesDiffuse[2].texture = D3DXVECTOR2(1.0f, 1.0f);
		verticesDiffuse[2].normal = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}
	else if (HasColour())
	{
		// Bottom left
		verticesColour[0].position = D3DXVECTOR3(x - 1.0f, y - 1.0f, z);
		verticesColour[0].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
		// Top middle
		verticesColour[1].position = D3DXVECTOR3(x, y + 1.0f, z);
		verticesColour[1].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
		// Bottom right
		verticesColour[2].position = D3DXVECTOR3(x + 1.0f, y - 1.0f, z);
		verticesColour[2].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Could not determine what vertex buffer to fill in Model.cpp.");
		return false;
	}

	return true;
}

void CModel::ResetColour()
{
	// Initialise everything to more a value which is not the same as null, 0.0f is the same as null and can cause issues.
	mColour.x = -1.0f;
	mColour.y = -1.0f;
	mColour.z = -1.0f;
}

bool CModel::HasTexture()
{
	return mpTexture != nullptr;
}

bool CModel::HasColour()
{
	return mColour.x > -1.0f && mColour.y > -1.0f && mColour.z > -1.0f;
}

bool CModel::CreateVertexArray(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse, bool applyTexture)
{
	applyTexture = applyTexture || HasTexture();
	if (HasColour())
	{
		verticesColour = new VertexColourType[mVertexCount];
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(verticesColour).name());

		if (!verticesColour)
		{
			mpLogger->GetLogger().WriteLine("Failed to create a vertex array for colour.");
			return false;
		}
	}
	else if (applyTexture && !mUseDiffuseLighting)
	{
		verticesTexture = new VertexTextureType[mVertexCount];
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(verticesTexture).name());
		if (!verticesTexture)
		{
			mpLogger->GetLogger().WriteLine("Failed to create a vertex array for texture.");
			return false;
		}
	}
	else if (applyTexture && mUseDiffuseLighting)
	{
		verticesDiffuse = new VertexDiffuseLightingType[mVertexCount];
		mpLogger->GetLogger().MemoryAllocWriteLine(typeid(verticesDiffuse).name());
		if (!verticesDiffuse)
		{
			mpLogger->GetLogger().WriteLine("Failed to create a vertex array for texture using diffuse lighting.");
			return false;
		}
	}

	return true;
}

bool CModel::CreateVertexBuffer(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	bool result;
	D3D11_SUBRESOURCE_DATA vertexData;

	/* Set up the descriptor for the vertex buffer. */

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	if (verticesTexture != nullptr)
	{
		vertexBufferDesc.ByteWidth = sizeof(VertexTextureType) * mVertexCount;
	}
	else if (verticesColour != nullptr)
	{
		vertexBufferDesc.ByteWidth = sizeof(VertexColourType) * mVertexCount;
	}
	else if (verticesDiffuse != nullptr)
	{
		vertexBufferDesc.ByteWidth = sizeof(VertexDiffuseLightingType) * mVertexCount;
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Failed to find a vertex array to use when setting up descriptor for the vertex buffer in Model.cpp.");
		return false;
	}
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	/* Give the subresource struct a pointer to the vertex data.*/
	if (verticesTexture != nullptr)
	{
		vertexData.pSysMem = verticesTexture;
	}
	else if (verticesColour != nullptr)
	{
		vertexData.pSysMem = verticesColour;
	}
	else if (verticesDiffuse != nullptr)
	{
		vertexData.pSysMem = verticesDiffuse;
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Failed to find the struct being used, so impossible to determine what to pass in to the vertexData.");
		return false;
	}
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	/* Create the vertex buffer. */

	result = mpDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	if (FAILED(result))
	{
		mpLogger->GetLogger().WriteLine("Failed to create the vertex buffer.");
		return false;
	}

	return true;
}

void CModel::CleanVertexBuffers(VertexTextureType* &verticesTexture, VertexColourType* &verticesColour, VertexDiffuseLightingType* &verticesDiffuse)
{
	// Release the local arrays for vertex and index buffers.
	if (verticesColour != nullptr)
	{
		delete[] verticesColour;
		verticesColour = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(verticesColour).name());
	}

	if (verticesTexture != nullptr)
	{
		delete[] verticesTexture;
		verticesTexture = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(verticesTexture).name());
	}

	if (verticesDiffuse != nullptr)
	{
		delete[] verticesDiffuse;
		verticesDiffuse = nullptr;
		mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(verticesDiffuse).name());
	}
}

bool CModel::UseDiffuseLight()
{
	return mUseDiffuseLighting;
}

void CModel::CommitToMatrix()
{
	VertexTextureType* verticesTexture = nullptr;
	VertexColourType* verticesColour = nullptr;
	VertexDiffuseLightingType* verticesDiffuse = nullptr;

	CreateVertexArray(verticesTexture, verticesColour, verticesDiffuse, HasTexture());

	// Set up the buffer data which will be passed to DirectX.
	SetTriangleBuffers(verticesColour, verticesTexture, verticesDiffuse, HasTexture(), mPositionX, mPositionY, mPositionZ);

	// Write the vertex points to the matrix.
	CreateVertexBuffer(verticesTexture, verticesColour, verticesDiffuse);

	// Delete any allocated memory.
	CleanVertexBuffers(verticesTexture, verticesColour, verticesDiffuse);

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

void CModel::CommitRotate()
{
	// Define the matrices.
	D3DXMATRIX rotXMatrix;
	D3DXMATRIX rotYMatrix;
	D3DXMATRIX rotZMatrix;

	// Use Direct X to rotate the matrices and pass the matrix after rotation back into the rotation matrix we defined.
	D3DXMatrixRotationX(&rotXMatrix, mRotationX);
	D3DXMatrixRotationX(&rotYMatrix, mRotationY);
	D3DXMatrixRotationX(&rotZMatrix, mRotationZ);

	// Combine the matrices.
	D3DXMATRIX rotationMatrix = rotXMatrix * rotYMatrix * rotZMatrix;

}

void CModel::MoveX(float x)
{
	mPositionX += x;

	// Make sure the triangle is redrawn in that new position.
	CommitToMatrix();
}

void CModel::MoveY(float y)
{
	mPositionY += y;

	// Make sure the triangle is redrawn in that new position.
	CommitToMatrix();
}

void CModel::MoveZ(float z)
{
	mPositionZ += z;

	// Make sure the triangle is redrawn in that new position.
	CommitToMatrix();
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
	CommitToMatrix();
}

void CModel::SetYPos(float y)
{
	mPositionY = y;

	// Make sure the triangle is redrawn in that new position.
	CommitToMatrix();
}

void CModel::SetZPos(float z)
{
	mPositionZ = z;

	// Make sure the triangle is redrawn in that new position.
	CommitToMatrix();
}

