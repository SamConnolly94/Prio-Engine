#include "Foliage.h"



CFoliage::CFoliage()
{
	mFoliageTranslation = { 0.0f, 0.0f, 0.0f };
}


CFoliage::~CFoliage()
{
}

bool CFoliage::Initialise(ID3D11Device * device, FoliageVertexType * vertices, int numberOfVertices, unsigned int * indices, int numberOfIndices)
{
	mVertexCount = numberOfVertices;
	mIndexCount = numberOfIndices;

	///////////////////////////
	// Foliage textures
	///////////////////////////

	mpFoliageTex = new CTexture();
	if (!mpFoliageTex->Initialise(device, "Resources/Textures/Foliage/Grass.png"))
	{
		logger->GetInstance().WriteLine("Failed to load 'Resources/Textures/Foliage/Grass.png'.");
	}

	mpFoliageAlphaTex = new CTexture();
	if (!mpFoliageAlphaTex->Initialise(device, "Resources/Textures/Foliage/GrassAlpha.png"))
	{
		logger->GetInstance().WriteLine("Failed to load 'Resources/Textures/Foliage/GrassAlpha.png'.");
	}

	if (!InitialiseBuffers(device, vertices, indices))
	{
		logger->GetInstance().WriteLine("Failed to initialise foliage buffers.");
		return false;
	}

	return true;
}

void CFoliage::Shutdown()
{
	if (mpFoliageAlphaTex)
	{
		mpFoliageAlphaTex->Shutdown();
		delete mpFoliageAlphaTex;
		mpFoliageAlphaTex = nullptr;
	}

	if (mpFoliageTex)
	{
		mpFoliageTex->Shutdown();
		delete mpFoliageTex;
		mpFoliageTex = nullptr;
	}

	if (mpVertexBuffer)
	{
		mpVertexBuffer->Release();
		mpVertexBuffer = nullptr;
	}

	if (mpIndexBuffer)
	{
		mpIndexBuffer->Release();
		mpIndexBuffer = nullptr;
	}
}

void CFoliage::Render(ID3D11DeviceContext * deviceContext)
{
	RenderBuffers(deviceContext);
}

void CFoliage::Update(float updateTime)
{
	mFoliageTranslation += mWindDirection * updateTime;

	if (mFoliageTranslation.z > 0.5f || mFoliageTranslation.z < 0.0f)
	{
		mWindDirection = -mWindDirection;
	}
}

bool CFoliage::InitialiseBuffers(ID3D11Device * device, FoliageVertexType * vertices, unsigned int * indices)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	///////////////////////
	// Buffer setup
	//////////////////////

	// Set up the descriptor of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(FoliageVertexType) * mVertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * mIndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage index buffer from the buffer description.");
		return false;
	}

	return true;
}

void CFoliage::RenderBuffers(ID3D11DeviceContext * deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set the vertex buffer stride and offset.
	stride = sizeof(FoliageVertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler.
	deviceContext->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler.
	deviceContext->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Tell directx we've passed it a triangle list in the form of indices.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

int CFoliage::GetNumberOfIndices()
{
	return mIndexCount;
}
