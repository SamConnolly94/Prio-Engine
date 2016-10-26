#include "Model.h"


CModel::CModel(ID3D11Device * device)
{
	mpDevice = device;
	mpVertexManager = new CVertexManager(PrioEngine::VertexType::Texture);
	mpVertexManager->SetDevicePtr(mpDevice);
	//mpVertexManager->SetColour(PrioEngine::Colours::red);
	
}


CModel::~CModel()
{
	delete mpVertexManager;
}

void CModel::SetNumberOfVertices(int size)
{
	mVerticesCount = size;
}

void CModel::SetTextureCount(int size)
{
	mTextureCount = size;
}

void CModel::SetNumberOfNormals(int size)
{
	mNormalsCount = size;
}

void CModel::SetNumberOfIndices(int size)
{
	mIndicesCount = size;
}

void CModel::UpdateMatrices(D3DXMATRIX &world)
{
	D3DXMATRIX modelWorld;

	// Calculate the translation of the camera.
	D3DXMatrixTranslation(&modelWorld, mPosition.x, mPosition.y, mPosition.z);

	// Rotation
	D3DXMATRIX matrixRotationX;
	D3DXMATRIX matrixRotationY;
	D3DXMATRIX matrixRotationZ;

	// Calculate the rotation of the camera.
	D3DXMatrixRotationX(&matrixRotationX, mRotation.x);
	D3DXMatrixRotationZ(&matrixRotationZ, mRotation.z);
	D3DXMatrixRotationY(&matrixRotationY, mRotation.y);

	// Calculate the world matrix
	world = modelWorld * matrixRotationX * matrixRotationY * matrixRotationZ;
}

void CModel::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	mpVertexManager->RenderBuffers(deviceContext, mpIndexBuffer);
}

bool CModel::SetGeometry(D3DXVECTOR3 * vertices, unsigned long* indices, D3DXVECTOR2* UV)
{
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mpVertexManager->SetNumberOfVertices(mVerticesCount);

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f, vertices, UV);

	// Create the vertex buffer.
	if (!mpVertexManager->CreateVertexBuffer())
	{
		return false;
	}

	/* Set up the descriptor of the index buffer. */
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * mIndicesCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	/* Give the subresource structure a pointer to the index data. */
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = mpDevice->CreateBuffer(&indexBufferDesc, &indexData, &mpIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	mpVertexManager->CleanArrays();

	return true;
}
