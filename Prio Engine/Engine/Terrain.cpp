#include "Terrain.h"

CTerrainGrid::CTerrainGrid(ID3D11Device* device)
{
	// Output alloc message to memory log.
	gLogger->MemoryAllocWriteLine(typeid(this).name());

	mpDevice = device;

	// Initialise pointers to nullptr.
	mpVertexBuffer = nullptr;
	mpIndexBuffer = nullptr;

	// Initialise all variables to null.
	mVertexCount = NULL;
	mIndexCount = NULL;

	mHeightMapLoaded = false;
	mpHeightMap = nullptr;

	mLowestPoint = 0.0f;
	mHighestPoint = 0.0f;

	mStyle = Solid;
}


CTerrainGrid::~CTerrainGrid()
{
	// Output dealloc message to memory log.
	gLogger->MemoryDeallocWriteLine(typeid(this).name());

	ShutdownBuffers();
}

bool CTerrainGrid::CreateGrid()
{
	bool result;

	// Set the width and height of the terrain.
	if (mWidth == NULL)
	{
		mWidth = 100;
	}
	if (mHeight == NULL)
	{
		mHeight = 100;
	}

	// Initialise the vertex buffers.
	result = InitialiseBuffers(mpDevice);
	// If we weren't successful in creating buffers.
	if (!result)
	{
		// Output error to log.
		gLogger->WriteLine("Failed to initialise buffers in Terrain.cpp.");
		return false;
	}

	return true;
}

void CTerrainGrid::Render(ID3D11DeviceContext * context)
{
	// Render the data contained in the buffers..
	RenderBuffers(context);
}

bool CTerrainGrid::InitialiseBuffers(ID3D11Device * device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index;
	float posX;
	float posZ;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;
	const int kNumTrianglesInSquare = 4;
	const int kNumLinesInSquare = 8;

	// Calculate the number of vertices in the terrain mesh.
	mVertexCount = 0;

	mVertexCount = (mWidth * mHeight);

	// Same number of indices as vertices.
	mIndexCount = (mWidth - 1) * (mHeight - 1) * 6;

	// Create the vertex array.
	vertices = new VertexType[mVertexCount];
	gLogger->MemoryAllocWriteLine(typeid(vertices).name());
	if (!vertices)
	{
		gLogger->WriteLine("Failed to create the vertex array in InitialiseBuffers function, Terrain.cpp.");
		return false;
	}

	// Create the index array.
	indices = new unsigned long[mIndexCount];
	if (!indices)
	{
		gLogger->WriteLine("Failed to create the index array in InitialiseBuffers function, Terrain.cpp.");
		return false;
	}

	index = 0;
	int vertex = 0;

	/// TODO: Change from a linelist to a triangle strip, this will allow us to draw solid colours as terrain.

	// Plot the vertices of the grid.

	for (int heightCount = 0; heightCount < mHeight; heightCount++)
	{
		for (int widthCount = 0; widthCount < mWidth; widthCount++)
		{
			float posX = static_cast<float>(widthCount);
			float posZ = static_cast<float>(heightCount);
			if (mHeightMapLoaded)
			{
				vertices[vertex].position = D3DXVECTOR3{ posX, static_cast<float>(mpHeightMap[heightCount][widthCount]), posZ };
			}
			else
			{
				vertices[vertex].position = D3DXVECTOR3{ posX, 0.0f, posZ };
			}

			vertices[vertex].colour = D3DXVECTOR4{ 1.0f, 1.0f, 1.0f, 1.0f };

			vertex++;
		}
	}

	vertex = 0;

	for (int heightCount = 0; heightCount < mHeight - 1; heightCount++)
	{
		for (int widthCount = 0; widthCount < mWidth - 1; widthCount++)
		{
			indices[index] = vertex;
			indices[index + 1] = vertex + mWidth;
			indices[index + 2] = vertex + 1;

			indices[index + 3] = vertex + 1;
			indices[index + 4] = vertex + mWidth;
			indices[index + 5] = vertex + mWidth + 1;

			index += 6;
			vertex++;
		}
		vertex++;
	}


	// Set up the descriptor of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * mVertexCount;
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
		gLogger->WriteLine("Failed to create the vertex buffer from the buffer description.");
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndexCount;
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
		gLogger->WriteLine("Failed to create the index buffer from the buffer description.");
		return false;
	}

	// Clean up the memory allocated to arrays.
	delete[] vertices;
	gLogger->MemoryDeallocWriteLine(typeid(vertices).name());
	vertices = nullptr;

	delete[] indices;
	gLogger->MemoryDeallocWriteLine(typeid(indices).name());
	indices = nullptr;

	return true;
}

void CTerrainGrid::ShutdownBuffers()
{
	// Release any memory given to the vertex buffer.
	if (mpVertexBuffer)
	{
		mpVertexBuffer->Release();
		mpVertexBuffer = nullptr;
	}

	// Release any memory given to the index buffer.
	if (mpIndexBuffer)
	{
		mpIndexBuffer->Release();
		mpIndexBuffer = nullptr;
	}
}

void CTerrainGrid::RenderBuffers(ID3D11DeviceContext * context)
{
	unsigned int stride;
	unsigned int offset;

	// Set the vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler.
	context->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler.
	context->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	/// TODO: Change from a linelist to a trianglelist here. This will enable us to render a solid colour.

	// Set the render format to line list.
	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//switch (mStyle)
	//{
	//case Solid:
	//	context->IASetPrimitiveTopology(/*D3D11_PRIMITIVE_TOPOLOGY_LINELIST*/ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	break;
	//case Wireframe:
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//	break;
	//default:
	//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//	break;
	//}
}

void CTerrainGrid::LoadHeightMap(double ** heightMap)
{
	mpHeightMap = heightMap;

	mLowestPoint = mpHeightMap[0][0];
	mHighestPoint = mpHeightMap[0][0];

	gLogger->WriteLine("Copied height map over to terrain, time to find the heights and lowest points.");
	for (unsigned int y = 0; y < mHeight; y++)
	{
		for (unsigned int x = 0; x < mWidth; x++)
		{
			if (mpHeightMap[y][x] < mLowestPoint)
			{
				mLowestPoint = static_cast<float>(mpHeightMap[y][x]);
			}
			else if (mpHeightMap[y][x] > mHighestPoint)
			{
				mHighestPoint = static_cast<float>(mpHeightMap[y][x]);
			}
		}
	}

	SetYPos(0.0f - mLowestPoint);
	SetXPos(0 - (static_cast<float>(mWidth) / 2.0f));

	mHeightMapLoaded = true;
}

/* Reinitialises the buffers so we can change from rendering in wireframe mode to solid colour mode. */
void CTerrainGrid::SetDrawStyle(DrawStyle style)
{
	// Update the style.
	mStyle = style;

	/// Update the vertex and index buffers. Will need to recreate them.
	// Deallocates the vertex and index buffers.
	ShutdownBuffers();
	// Reinitialise the vertex and index buffers.
	InitialiseBuffers(mpDevice);

	// Output a message to the log to let our user know we successfully updated the draw style.
	gLogger->WriteLine("Updated draw style in terrain.");
}


