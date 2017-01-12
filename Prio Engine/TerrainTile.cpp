#include "TerrainTile.h"



CTerrainTile::CTerrainTile()
{
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;
}


CTerrainTile::~CTerrainTile()
{
	//if (mpVertexBuffer != nullptr)
	//{
	//	mpVertexBuffer->Release();
	//}

	//if (mpIndexBuffer != nullptr)
	//{
	//	mpIndexBuffer->Release();
	//}
}

void CTerrainTile::Render(ID3D11DeviceContext * context)
{
	RenderBuffers(context);
}

bool CTerrainTile::UpdateBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;

	bool result;
	//if (terrainType == Snow)
	//{
	//	mVertices[0].colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	//	mVertices[1].colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	//	mVertices[2].colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	//}
	//else if (terrainType == Grass)
	//{
	//	mVertices[0].colour = { 0.0f, 1.0f, 0.0f, 1.0f };
	//	mVertices[1].colour = { 0.0f, 1.0f, 0.0f, 1.0f };
	//	mVertices[2].colour = { 0.0f, 1.0f, 0.0f, 1.0f };
	//}
	

	// Set up the descriptor of the static vertex buffer.
	//vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//vertexBufferDesc.ByteWidth = sizeof(VertexType) * 3;
	//vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vertexBufferDesc.CPUAccessFlags = 0;
	//vertexBufferDesc.MiscFlags = 0;
	//vertexBufferDesc.StructureByteStride = 0;

	//// Give the subresource structure a pointer to the vertex data.
	//vertexData.pSysMem = mVertices;
	//vertexData.SysMemPitch = 0;
	//vertexData.SysMemSlicePitch = 0;

	//// Create the vertex buffer.
	//result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	//if (FAILED(result))
	//{
	//	gLogger->WriteLine("Failed to create the vertex buffer from the buffer description in TerrainTile.");
	//	return false;
	//}
	//unsigned long indices[3] = { 0, 1, 2 };

	//// Set up the description of the static index buffer.
	//indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//indexBufferDesc.ByteWidth = sizeof(unsigned long) * 3;
	//indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//indexBufferDesc.CPUAccessFlags = 0;
	//indexBufferDesc.MiscFlags = 0;
	//indexBufferDesc.StructureByteStride = 0;

	//// Give the subresource structure a pointer to the index data.
	//indexData.pSysMem = indices;
	//indexData.SysMemPitch = 0;
	//indexData.SysMemSlicePitch = 0;

	//// Create the index buffer.
	//result = device->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	//if (FAILED(result))
	//{
	//	gLogger->WriteLine("Failed to create the index buffer from the buffer description in terrain tile..");
	//	return false;
	//}
	return true;
}

bool CTerrainTile::Compare(CTerrainTile tile)
{
	if (tile.mVertices == mVertices && 
		tile.terrainType == terrainType)
	{
		return true;
	}
	return false;
}

void CTerrainTile::RenderBuffers(ID3D11DeviceContext * context)
{
	//if (mpVertexBuffer == nullptr)
	//{
	//	return;
	//}

	//unsigned int stride;
	//unsigned int offset;

	//// Set the vertex buffer stride and offset.
	//stride = sizeof(VertexType);
	//offset = 0;

	//// Set the vertex buffer to active in the input assembler.
	//context->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	//// Set the index buffer to active in the input assembler.
	//context->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Tell directx we've passed it a triangle list in the form of indices.
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
