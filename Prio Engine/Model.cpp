#include "Model.h"

CModel::CModel()
{
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;

}


CModel::~CModel()
{
}

bool CModel::Initialise(ID3D11Device * device)
{
	bool result;

	// Initialise the vertex and index buffer that hold geometry for the triangle.
	result = InitialiseBuffers(device);
	
	// Return our success / failure to init the vertex and index buffer.
	return result;
}

void CModel::Shutdown()
{
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

/* Initialises the vertex and index buffers which hold geometry for a triangle.*/
bool CModel::InitialiseBuffers(ID3D11Device * device)
{
	VertexType* vertices;
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
	vertices = new VertexType[mVertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[mIndexCount];
	if (!indices)
	{
		return false;
	}

	/* Set the vertex points for the triangle. */
	
	// Bottom left
	vertices[0].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	vertices[0].colour = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);

	// Top middle
	vertices[1].position = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	vertices[1].colour = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);

	// Bottom left
	vertices[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	vertices[2].colour = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);

	/* Load index array with data. */
	
	// Bottom left.
	indices[0] = 0;
	
	// Top middle.
	indices[1] = 1;

	// Bottom right.
	indices[2] = 2;

	/* Set up the descriptor for the vertex buffer. */

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * mVertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	/* Give the subresource struct a pointer to the vertex data.*/

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	/* Create the vertex buffer. */

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBuffer);
	if (FAILED(result))
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

	// Release the local arrays for vertex and index buffers.
	delete[] vertices;
	vertices = nullptr;

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
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case we use triangles to draw.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
