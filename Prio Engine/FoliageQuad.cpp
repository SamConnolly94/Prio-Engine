#include "FoliageQuad.h"



CFoliageQuad::CFoliageQuad()
{
}


CFoliageQuad::~CFoliageQuad()
{
}

bool CFoliageQuad::Initialise(ID3D11Device* device)
{
	GeneratePoints();

	if (!InitialiseBuffers(device))
	{
		logger->GetInstance().WriteLine("Failed to initialise the buffers for foliage quad.");
		return false;
	}
}

void CFoliageQuad::Shutdown()
{
	if (mpVertexBufferRect1Tri1)
	{
		mpVertexBufferRect1Tri1->Release();
		mpVertexBufferRect1Tri1 = nullptr;
	}

	if (mpVertexBufferRect1Tri2)
	{
		mpVertexBufferRect1Tri2->Release();
		mpVertexBufferRect1Tri2 = nullptr;
	}

	if (mpVertexBufferRect2Tri1)
	{
		mpVertexBufferRect2Tri1->Release();
		mpVertexBufferRect2Tri1 = nullptr;
	}

	if (mpVertexBufferRect2Tri2)
	{
		mpVertexBufferRect2Tri2->Release();
		mpVertexBufferRect2Tri2 = nullptr;
	}

	if (mpVertexBufferRect3Tri1)
	{
		mpVertexBufferRect3Tri1->Release();
		mpVertexBufferRect3Tri1 = nullptr;
	}

	if (mpVertexBufferRect3Tri2)
	{
		mpVertexBufferRect3Tri2->Release();
		mpVertexBufferRect3Tri2 = nullptr;
	}
}

void CFoliageQuad::GeneratePoints()
{
	//////////////////////
	// Horizontal line
	//////////////////////

	float height = 1.0f;

	// Bottom left
	mFoliageRect[0].Position[0] = { 0.0f, 0.0f, 0.5f };
	mFoliageRect[0].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[0].Normal[0] = { 0.0f, 1.0f, 0.5f };
	// Bottom right
	mFoliageRect[0].Position[1] = { 1.0f, 0.0f, 0.5f };
	mFoliageRect[0].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[0].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[0].Position[2] = { 0.0f, 1.0f, 0.5f };
	mFoliageRect[0].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[0].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[0].Position[3] = { 1.0f, 1.0f, 0.5f };
	mFoliageRect[0].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[0].Normal[3] = { 0.0f, 1.0f, 0.0f };

	////////////////////////
	// Starting point top left, diag down to right
	///////////////////////
	// Bottom left
	mFoliageRect[1].Position[0] = { 0.0f, 0.0f, 1.0f };
	mFoliageRect[1].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[1].Normal[0] = { 0.0f, 1.0f, 0.0f };
	// Bottom right
	mFoliageRect[1].Position[1] = { 1.0f, 0.0f, 0.0f };
	mFoliageRect[1].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[1].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[1].Position[2] = { 0.0f, 1.0f, 1.0f };
	mFoliageRect[1].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[1].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[1].Position[3] = { 1.0f, 1.0f, 0.0f };
	mFoliageRect[1].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[1].Normal[3] = { 0.0f, 1.0f, 0.0f };

	////////////////////////
	// Starting point bottom left, diag up to right
	///////////////////////
	// Bottom left
	mFoliageRect[2].Position[0] = { 0.0f, 0.0f, 0.0f };
	mFoliageRect[2].UV[0] = { 0.0f, 1.0f };
	mFoliageRect[2].Normal[0] = { 0.0f, 1.0f, 0.0f };
	// Bottom right
	mFoliageRect[2].Position[1] = { 1.0f, 0.0f, 1.0f };
	mFoliageRect[2].UV[1] = { 1.0f, 1.0f };
	mFoliageRect[2].Normal[1] = { 0.0f, 1.0f, 0.0f };
	// Top left
	mFoliageRect[2].Position[2] = { 0.0f, 1.0f, 0.0f };
	mFoliageRect[2].UV[2] = { 0.0f, 0.0f };
	mFoliageRect[2].Normal[2] = { 0.0f, 1.0f, 0.0f };
	// Top right
	mFoliageRect[2].Position[3] = { 1.0f, 1.0f, 1.0f };
	mFoliageRect[2].UV[3] = { 1.0f, 0.0f };
	mFoliageRect[2].Normal[3] = { 0.0f, 1.0f, 0.0f };
}

bool CFoliageQuad::InitialiseBuffers(ID3D11Device * device)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	HRESULT result;

	FoliageVertexType* vertices = new FoliageVertexType[mVertexCount];

	///////////////////////////////////////////
	// Rect 1, Triangle 1
	///////////////////////////////////////////

	// Lower left
	vertices[0].position		= mFoliageRect[0].Position[0];
	vertices[0].UV				= mFoliageRect[0].UV[0];
	vertices[0].normal			= mFoliageRect[0].Normal[0];
	vertices[0].Type			= 0;
	vertices[0].IsTopVertex		= 0;

	// Lower right
	vertices[1].position		= mFoliageRect[0].Position[1];
	vertices[1].UV				= mFoliageRect[0].UV[1];
	vertices[1].normal			= mFoliageRect[0].Normal[1];
	vertices[1].Type			= 0;
	vertices[1].IsTopVertex		= 0;

	// Upper left
	vertices[2].position		= mFoliageRect[0].Position[2];
	vertices[2].UV				= mFoliageRect[0].UV[2];
	vertices[2].normal			= mFoliageRect[0].Normal[2];
	vertices[2].Type			= 0;
	vertices[2].IsTopVertex		= 1;

	//////////////////////////////////////////
	// Set up vertex buffer desc
	// This can be reused as most of the details are the same.
	//////////////////////////////////////////

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
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect1Tri1);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	///////////////////////////////////////////
	// Rect 1, Triangle 2
	///////////////////////////////////////////

	vertices[0].position	= mFoliageRect[0].Position[1];
	vertices[0].UV			= mFoliageRect[0].UV[1];
	vertices[0].normal		= mFoliageRect[0].Normal[1];
	vertices[0].Type		= 0;
	vertices[0].IsTopVertex = 0;

	// Lower right
	vertices[1].position	= mFoliageRect[0].Position[2];
	vertices[1].UV			= mFoliageRect[0].UV[2];
	vertices[1].normal		= mFoliageRect[0].Normal[2];
	vertices[1].Type		= 0;
	vertices[1].IsTopVertex = 1;

	// Upper left
	vertices[2].position	= mFoliageRect[0].Position[3];
	vertices[2].UV			= mFoliageRect[0].UV[3];
	vertices[2].normal		= mFoliageRect[0].Normal[3];
	vertices[2].Type		= 0;
	vertices[2].IsTopVertex = 1;


	// Set up the descriptor of the static vertex buffer.
	vertexData.pSysMem = vertices;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect1Tri2);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	///////////////////////////////////////////
	// Rect 2, Triangle 1
	///////////////////////////////////////////

	// Lower left
	vertices[0].position	= mFoliageRect[1].Position[0];
	vertices[0].UV			= mFoliageRect[1].UV[0];
	vertices[0].normal		= mFoliageRect[1].Normal[0];
	vertices[0].Type		= 0;
	vertices[0].IsTopVertex = 0;

	// Lower right
	vertices[1].position	= mFoliageRect[1].Position[1];
	vertices[1].UV			= mFoliageRect[1].UV[1];
	vertices[1].normal		= mFoliageRect[1].Normal[1];
	vertices[1].Type		= 0;
	vertices[1].IsTopVertex = 0;

	// Upper left
	vertices[2].position	= mFoliageRect[1].Position[2];
	vertices[2].UV			= mFoliageRect[1].UV[2];
	vertices[2].normal		= mFoliageRect[1].Normal[2];
	vertices[2].Type		= 0;
	vertices[2].IsTopVertex = 1;

	// Set up the descriptor of the static vertex buffer.
	vertexData.pSysMem = vertices;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect2Tri1);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	///////////////////////////////////////////
	// Rect 2, Triangle 2
	///////////////////////////////////////////

	vertices[0].position	= mFoliageRect[1].Position[1];
	vertices[0].UV			= mFoliageRect[1].UV[1];
	vertices[0].normal		= mFoliageRect[1].Normal[1];
	vertices[0].Type		= 0;
	vertices[0].IsTopVertex = 0;
	
	vertices[1].position	= mFoliageRect[1].Position[2];
	vertices[1].UV			= mFoliageRect[1].UV[2];
	vertices[1].normal		= mFoliageRect[1].Normal[2];
	vertices[1].Type		= 0;
	vertices[1].IsTopVertex = 1;

	vertices[2].position	= mFoliageRect[1].Position[3];
	vertices[2].UV			= mFoliageRect[1].UV[3];
	vertices[2].normal		= mFoliageRect[1].Normal[3];
	vertices[2].Type		= 0;
	vertices[2].IsTopVertex = 1;


	// Set up the descriptor of the static vertex buffer.
	vertexData.pSysMem = vertices;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect2Tri2);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	///////////////////////////////////////////
	// Rect 3, Triangle 1
	///////////////////////////////////////////

	// Lower left
	vertices[0].position	= mFoliageRect[2].Position[0];
	vertices[0].UV			= mFoliageRect[2].UV[0];
	vertices[0].normal		= mFoliageRect[2].Normal[0];
	vertices[0].Type		= 0;
	vertices[0].IsTopVertex = 0;

	// Lower right
	vertices[1].position	= mFoliageRect[2].Position[1];
	vertices[1].UV			= mFoliageRect[2].UV[1];
	vertices[1].normal		= mFoliageRect[2].Normal[1];
	vertices[1].Type		= 0;
	vertices[1].IsTopVertex = 0;

	// Upper left
	vertices[2].position	= mFoliageRect[2].Position[2];
	vertices[2].UV			= mFoliageRect[2].UV[2];
	vertices[2].normal		= mFoliageRect[2].Normal[2];
	vertices[2].Type		= 0;
	vertices[2].IsTopVertex = 1;

	// Set up the descriptor of the static vertex buffer.
	vertexData.pSysMem = vertices;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect3Tri1);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	///////////////////////////////////////////
	// Rect 3, Triangle 2
	///////////////////////////////////////////

	vertices[0].position	= mFoliageRect[2].Position[1];
	vertices[0].UV			= mFoliageRect[2].UV[1];
	vertices[0].normal		= mFoliageRect[2].Normal[1];
	vertices[0].Type		= 0;
	vertices[0].IsTopVertex = 0;

	vertices[1].position	= mFoliageRect[2].Position[2];
	vertices[1].UV			= mFoliageRect[2].UV[2];
	vertices[1].normal		= mFoliageRect[2].Normal[2];
	vertices[1].Type		= 0;
	vertices[1].IsTopVertex = 1;

	vertices[2].position	= mFoliageRect[2].Position[3];
	vertices[2].UV			= mFoliageRect[2].UV[3];
	vertices[2].normal		= mFoliageRect[2].Normal[3];
	vertices[2].Type		= 0;
	vertices[2].IsTopVertex = 1;


	// Set up the descriptor of the static vertex buffer.
	vertexData.pSysMem = vertices;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &mpVertexBufferRect3Tri2);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage vertex buffer from the buffer description.");
		return false;
	}

	delete[] vertices;

	return true;
}

void CFoliageQuad::SetPosition(D3DXVECTOR3 pos)
{
	mPosition = pos;
}

D3DXVECTOR3 CFoliageQuad::GetCentrePos()
{
	return mCentrePos;
}
