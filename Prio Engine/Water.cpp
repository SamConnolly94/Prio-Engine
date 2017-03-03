#include "Water.h"

CWater::CWater()
{
	mNumVertices = kNumberOfVerticesInWater;
	mNumIndices = mNumVertices;
	
	int vertex = 0;

	mpVertices[vertex].position.x = -20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = 20.0f;
	mpVertices[vertex].texCoord.x = 0.0f;
	mpVertices[vertex].texCoord.y = 0.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;

	vertex = 1;

	mpVertices[vertex].position.x = 20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = -20.0f;
	mpVertices[vertex].texCoord.x = 1.0f;
	mpVertices[vertex].texCoord.y = 1.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;

	vertex = 2;

	mpVertices[vertex].position.x = -20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = -20.0f;
	mpVertices[vertex].texCoord.x = 0.0f;
	mpVertices[vertex].texCoord.y = 1.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;

	vertex = 3;

	mpVertices[vertex].position.x = 20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = 20.0f;
	mpVertices[vertex].texCoord.x = 1.0f;
	mpVertices[vertex].texCoord.y = 0.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;

	vertex = 4;

	mpVertices[vertex].position.x = 20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = -20.0f;
	mpVertices[vertex].texCoord.x = 1.0f;
	mpVertices[vertex].texCoord.y = 1.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;

	vertex = 5;

	mpVertices[vertex].position.x = -20.0f;
	mpVertices[vertex].position.y = 0.0f;
	mpVertices[vertex].position.z = 20.0f;
	mpVertices[vertex].texCoord.x = 0.0f;
	mpVertices[vertex].texCoord.y = 0.0f;
	mpVertices[vertex].normal.x = 0.0f;
	mpVertices[vertex].normal.y = 1.0f;
	mpVertices[vertex].normal.z = 0.0f;


	for (int i = 0; i < mNumVertices; i++)
	{
		mpIndices[i] = i;
	}
}


CWater::~CWater()
{
}

bool CWater::Initialise(ID3D11Device * device, std::string filename)
{
	mpTexture = new CTexture();

	if (!mpTexture->Initialise(device, filename))
	{
		logger->GetInstance().WriteLine("Failed to initialise the texture for water model.");
		return false;
	}

	if (!InitialiseBuffers(device))
	{
		logger->GetInstance().WriteLine("Failed to initialise the buffers of the body of water.");
		return false;
	}

	logger->GetInstance().WriteLine("Successfully loaded the texture for the water model.");
	return true;
}

void CWater::Shutdown()
{
	if (mpTexture)
	{
		mpTexture->Shutdown();
		delete mpTexture;
		mpTexture = nullptr;
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

bool CWater::InitialiseBuffers(ID3D11Device * device)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * mNumVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = mpVertices;
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

	indexData.pSysMem = mpIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the body of water from the indices descriptor provided.");
		return false;
	}

	return true;
}

unsigned int CWater::GetIndexCount()
{
	return mNumIndices;
}

void CWater::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	deviceContext->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
