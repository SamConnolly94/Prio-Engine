#include "Water.h"

CWater::CWater()
{
	waterPos = { 0.0f, 0.0f };
	mpNormalMap = nullptr;
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
	mpReflectionMap = nullptr;
	mpRefractionMap = nullptr;
}


CWater::~CWater()
{
}

bool CWater::Initialise(ID3D11Device* device, D3DXVECTOR3 minPoint, D3DXVECTOR3 maxPoint, int screenWidth, int screenHeight, unsigned int subDivisionX, unsigned int subDivisionZ, std::string normalMap, bool useNormals, bool useUV)
{
	// Release the existing data.
	Shutdown();

	if (!InitialiseBuffers(device, minPoint, maxPoint, subDivisionX, subDivisionZ, useNormals, useUV))
	{
		logger->GetInstance().WriteLine("Failed to initialise the buffers of the body of water.");
		return false;
	}

	mpNormalMap = new CTexture();
	if (!mpNormalMap->Initialise(device, normalMap))
	{
		logger->GetInstance().WriteLine("Failed to load the normal map for water. Filename was: " + normalMap);
		return false;
	}

	if (!CreateTextureResources(device, screenWidth, screenHeight))
	{
		logger->GetInstance().WriteLine("Failed to create the texture resources for body of water in water.cpp.");
		return false;
	}

	mMovement = D3DXVECTOR2( 0.0f, 0.0f );
	D3DXVECTOR3 pos = (minPoint + maxPoint) / 2.0f;
	SetPos(pos.x, pos.y, pos.z);
	mWaveScale = 1.0f;
	mDistortionDistance = 30.0f;
	mDistortion = D3DXVECTOR2(16.0f, 20.0f);
	mStrength = D3DXVECTOR2(0.95f, 0.9f);
	mWidth = screenWidth;
	mHeight = screenHeight;

	logger->GetInstance().WriteLine("Successfully loaded the texture for the water model.");
	return true;
}

void CWater::Shutdown()
{
	if (mpNormalMap)
	{
		mpNormalMap->Shutdown();
		delete mpNormalMap;
		mpNormalMap = nullptr;
	}

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

void CWater::Render(ID3D11DeviceContext * deviceContext)
{
	RenderBuffers(deviceContext);
}

bool CWater::InitialiseBuffers(ID3D11Device * device, D3DXVECTOR3 minPoint, D3DXVECTOR3 maxPoint, unsigned int subDivisionX, unsigned int subDivisionZ, bool useNormals, bool useUV)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;
	VertexType* vertices;
	unsigned int* indices;

	mNumVertices = (subDivisionX) * (subDivisionZ);

	D3DXVECTOR3 point = minPoint;
	float xStep = (maxPoint.x - minPoint.x) / subDivisionX;
	float zStep = (maxPoint.z - minPoint.z) / subDivisionZ;

	D3DXVECTOR2 uv = { 0.0f, 1.0f };
	float uStep = 1.0f / subDivisionX;
	float vStep = 1.0f / subDivisionZ;

	vertices = new VertexType[mNumVertices];
	int vertex = 0;

	for (int z = 0; z < subDivisionZ; z++)
	{
		for (int x = 0; x < subDivisionX; x++)
		{
			vertices[vertex].position = point;
			
			if (useNormals)
			{
				vertices[vertex].normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			}

			if (useUV)
			{
				vertices[vertex].texCoord = uv;
			}

			point.x += xStep;
			uv.x += uStep;

			vertex++;
		}

		// reset the point on x back to what it was.
		point.x = minPoint.x;
		point.z += zStep;
		uv.x = 0.0f;
		uv.y -= vStep;
	}

	mNumIndices = subDivisionX * subDivisionZ * 6;
	indices = new unsigned int[mNumIndices];

	int index = 0;
	vertex = 0;

	for (int z = 0; z < subDivisionZ; z++)
	{
		for (int x = 0; x < subDivisionX; x++)
		{
			/// Calculate indices.

			/// Triangle 1.

			// Starting point.
			indices[index] = vertex;
			// Directly above.
			indices[index + 1] = vertex + subDivisionX;
			// Directly to the right.
			indices[index + 2] = vertex + 1;

			/// Triangle 2.

			// Directly to the right.
			indices[index + 3] = vertex + 1;
			// Directly above.
			indices[index + 4] = vertex + subDivisionX;
			// Above and to the right.
			indices[index + 5] = vertex + subDivisionX + 1;

			// We've added 6 indices so increment the count by 6.
			index += 6;

			// Increase the vertex which is our primary point.
			vertex++;
		}
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * mNumVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the body of water from vertex description provided.");
		return false;
	}

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * mNumIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the body of water from the indices descriptor provided.");
		return false;
	}

	delete[] vertices;
	vertices = nullptr;
	delete[] indices;
	indices = nullptr;

	return true;
}

bool CWater::CreateTextureResources(ID3D11Device* device, int screenWidth, int screenHeight)
{
	// A description for creating a texture to be used with the water. We can use this descriptor for both the refraction and reflection textures, as they're both the same size.
	D3D11_TEXTURE2D_DESC texDesc;
	
	texDesc.Width = screenWidth;
	texDesc.Height = screenHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	//////////////////////
	// Reflection resources.
	/////////////////////

	// Create the instance of the texture from the description.
	HRESULT result = device->CreateTexture2D(&texDesc, nullptr, &mpReflectionMap);

	// If we failed to create the instance of that texture.
	if (FAILED(result))
	{
		// Output error message to the log.
		logger->GetInstance().WriteLine("Failed to create the reflection map texture from description provided.");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create an instance of the render target view for the reflection texture.
	result = device->CreateRenderTargetView(mpReflectionMap, nullptr, &mpReflectionTarget);

	// If we failed to create the render target view for reflection.
	if (FAILED(result))
	{
		// Output an error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the reflection render target view from the reflection texture provided. Did it successfully initialise?");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create the shader resource for reflection map.
	result = device->CreateShaderResourceView(mpReflectionMap, nullptr, &mpReflectionResource);

	// If we failed to create the shader resource view.
	if (FAILED(result))
	{
		// Output error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the shader resource view from the reflection map provided. Was the reflection map successfully initialised?");
		// Don't continue any further, init should fail.
		return false;
	}

	//////////////////////
	// Refraction resources.
	/////////////////////

	// Create the instance of the texture from the description.
	result = device->CreateTexture2D(&texDesc, nullptr, &mpRefractionMap);

	// If we failed to create the instance of that texture.
	if (FAILED(result))
	{
		// Output error message to the log.
		logger->GetInstance().WriteLine("Failed to create the refraction map texture from description provided.");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create an instance of the render target view for the refraction texture.
	result = device->CreateRenderTargetView(mpRefractionMap, nullptr, &mpRefractionTarget);

	// If we failed to create the render target view for refraction.
	if (FAILED(result))
	{
		// Output an error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the refraction render target view from the refraction texture provided. Did it successfully initialise?");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create the shader resource for refraction map.
	result = device->CreateShaderResourceView(mpRefractionMap, nullptr, &mpRefractionResource);

	// If we failed to create the shader resource view.
	if (FAILED(result))
	{
		// Output error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the shader resource view from the refraction map provided. Was the refraction map successfully initialised?");
		// Don't continue any further, init should fail.
		return false;
	}

	//////////////////////
	// Height map resources.
	/////////////////////

	texDesc.Format = DXGI_FORMAT_R32_FLOAT;

	// Create the instance of the texture from the description.
	result = device->CreateTexture2D(&texDesc, nullptr, &mpHeightMap);

	// If we failed to create the instance of that texture.
	if (FAILED(result))
	{
		// Output error message to the log.
		logger->GetInstance().WriteLine("Failed to create the height map texture from description provided.");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create an instance of the render target view for the height map texture.
	result = device->CreateRenderTargetView(mpHeightMap, nullptr, &mpHeightMapTarget);

	// If we failed to create the render target view for height map.
	if (FAILED(result))
	{
		// Output an error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the refraction render target view from the height map texture provided. Did it successfully initialise?");
		// Don't continue any further, init should fail.
		return false;
	}

	// Create the shader resource for height map.
	result = device->CreateShaderResourceView(mpHeightMap, nullptr, &mpHeightMapResource);

	// If we failed to create the shader resource view.
	if (FAILED(result))
	{
		// Output error message to the logs.
		logger->GetInstance().WriteLine("Failed to create the shader resource view from the height map provided. Was the height map successfully initialised?");
		// Don't continue any further, init should fail.
		return false;
	}

	return true;
}

ID3D11ShaderResourceView * CWater::GetHeightMap()
{
	return mpHeightMapResource;
}

ID3D11ShaderResourceView * CWater::GetRefractionMap()
{
	return mpRefractionResource;
}

ID3D11ShaderResourceView * CWater::GetReflectionMap()
{
	return mpReflectionResource;
}

unsigned int CWater::GetNumberOfIndices()
{
	return mNumIndices;
}

CTexture * CWater::GetNormalMap()
{
	return mpNormalMap;
}

void CWater::Update(float updateTime)
{
	waterPos += updateTime * D3DXVECTOR2{ 0.01f, 0.015f };
}

void CWater::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set the vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler.
	deviceContext->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler.
	deviceContext->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Tell directx we've passed it a triangle list in the form of indices.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
