#include "Model.h"


CModels::CModels(ID3D11Device * device)
{
	mpDevice = device;
	mpVertexManager = new CVertexManager(PrioEngine::VertexType::Colour);
	mpVertexManager->SetDevicePtr(mpDevice);
	mpVertexManager->SetColour(PrioEngine::Colours::red);
}


CModels::~CModels()
{
	delete mpVertexManager;
}

void CModels::SetNumberOfVertices(int size)
{
	mVerticesCount = size;
}

void CModels::SetTextureCount(int size)
{
	mTextureCount = size;
}

void CModels::SetNumberOfNormals(int size)
{
	mNormalsCount = size;
}

void CModels::SetNumberOfIndices(int size)
{
	mIndicesCount = size;
}

void CModels::UpdateMatrices(D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& proj)
{
	// Rotation
	D3DXMATRIX matrixRotationX;
	D3DXMATRIX matrixRotationY;
	D3DXMATRIX matrixRotationZ;

	// Calculate the translation of the camera.
	D3DXMatrixTranslation(&mWorld, mPosition.x, mPosition.y, mPosition.z);

	// Calculate the rotation of the camera.
	D3DXMatrixRotationX(&matrixRotationX, mRotation.x);
	D3DXMatrixRotationY(&matrixRotationY, mRotation.y);
	D3DXMatrixRotationZ(&matrixRotationZ, mRotation.z);

	// Calculate the world matrix
	world = mWorld * matrixRotationX * matrixRotationY * matrixRotationZ;
}

void CModels::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	mpVertexManager->RenderBuffers(deviceContext, mpIndexBuffer);
}

bool CModels::SetGeometry(D3DXVECTOR3 * vertices, D3DXVECTOR3* indices)
{
	unsigned long* indicesArray;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;
	const int kNumberOfFloatsInVector3 = 3;

	// Set the number of vertices in the vertex array.
	mpVertexManager->SetNumberOfVertices(mVerticesCount);

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f, vertices, PrioEngine::Colours::green);

	// Create the index array.
	indicesArray = new unsigned long[mIndicesCount * kNumberOfFloatsInVector3];
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(indices).name());
	if (!indicesArray)
	{
		return false;
	}

	// Load the index array with data according to the predefined indicies defined in the engines namespace.
	int indicesArrayCounter = 0;
	for (int i = 0; i < mIndicesCount; i++)
	{
		indicesArray[indicesArrayCounter] = static_cast<unsigned long>(indices[i].x);
		indicesArrayCounter++;
		indicesArray[indicesArrayCounter] = static_cast<unsigned long>(indices[i].y);
		indicesArrayCounter++;
		indicesArray[indicesArrayCounter] = static_cast<unsigned long>(indices[i].z);
		indicesArrayCounter++;
	}

	// Create the vertex buffer.
	if (!mpVertexManager->CreateVertexBuffer())
	{
		return false;
	}

	/* Set up the descriptor of the index buffer. */
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (indicesArrayCounter);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	/* Give the subresource structure a pointer to the index data. */
	indexData.pSysMem = indicesArray;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = mpDevice->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	mpVertexManager->CleanArrays();

	delete[] indicesArray;
	indicesArray = nullptr;
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(indicesArray).name());

	return true;
}
