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

	// Store the filename for later use.
	mpTextureFilename = filename;
}

CModel::CModel(float3 colour)
{
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpTexture = nullptr;
	mpTextureFilename = nullptr;
	ResetColour();

	// Store the colour which we have passed in.
	mColour = colour;
}


CModel::~CModel()
{
}

bool CModel::Initialise(ID3D11Device * device)
{
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

	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mVertexCount = 3;
	
	// Set the number of indices on the index array.
	mIndexCount = 3;

	// Create a vertex array
	if (HasColour())
	{
		verticesColour = new VertexColourType[mVertexCount];
		
		if (!verticesColour)
		{
			mpLogger->GetLogger().WriteLine("Failed to create a vertex array for colour.");
			return false;
		}
	} 
	else if (applyTexture)
	{
		verticesTexture = new VertexTextureType[mVertexCount];
		if (!verticesTexture)
		{
			mpLogger->GetLogger().WriteLine("Failed to create a vertex array for texture.");
			return false;
		}
	}

	// Create the index array.
	indices = new unsigned long[mIndexCount];
	if (!indices)
	{
		return false;
	}

	/* Set the vertex points for the triangle. */
	if (applyTexture)
	{
		// Bottom left
		verticesTexture[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		verticesTexture[0].texture = D3DXVECTOR2(0.0f, 1.0f);
		// Top middle
		verticesTexture[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		verticesTexture[1].texture = D3DXVECTOR2(0.5f, 0.0f);
		// Bottom right
		verticesTexture[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		verticesTexture[2].texture = D3DXVECTOR2(1.0f, 1.0f);
	}
	else if (HasColour())
	{
		// Bottom left
		verticesColour[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		verticesColour[0].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
		// Top middle
		verticesColour[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		verticesColour[1].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
		// Bottom right
		verticesColour[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		verticesColour[2].colour = D3DXVECTOR4(mColour.x, mColour.y, mColour.z, 1.0f);
	}
	else
	{
		mpLogger->GetLogger().WriteLine("Could not determine what vertex buffer to fill in Model.cpp.");
		return false;
	}


	/* Load index array with data. */
	
	// Bottom left.
	indices[0] = 0;
	
	// Top middle.
	indices[1] = 1;

	// Bottom right.
	indices[2] = 2;

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
	else
	{
		mpLogger->GetLogger().WriteLine("Failed to find the struct being used, so impossible to determine what to pass in to the vertexData.");
		return false;
	}
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	/* Create the vertex buffer. */

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	if (FAILED(result))
	{
		mpLogger->GetLogger().WriteLine("Failed to create the vertex buffer.");
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

	// Release the local arrays for vertex and index buffers.
	if (verticesColour != nullptr)
	{
		delete[] verticesColour;
		verticesColour = nullptr;
	}
	
	if (verticesTexture != nullptr)
	{
		delete[] verticesTexture;
		verticesTexture = nullptr;
	}

	delete[] indices;
	indices = nullptr;

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
	if (HasTexture())
	{
		stride = sizeof(VertexTextureType);
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

	// Set the type of primitive that should be rendered from this vertex buffer, in this case we use triangles to draw.
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
	}
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