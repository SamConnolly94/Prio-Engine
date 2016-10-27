#include "Model.h"


CModel::CModel(ID3D11Device * device, PrioEngine::VertexType vertexType)
{
	mpDevice = device;
	mpVertexManager = new CVertexManager(vertexType);
	mpVertexManager->SetDevicePtr(mpDevice);
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

void CModel::UpdateMatrices()
{
	// Rotation
	D3DXMATRIX translation;
	D3DXMATRIX scale;
	D3DXMATRIX matrixRotationX;
	D3DXMATRIX matrixRotationY;
	D3DXMATRIX matrixRotationZ;

	// Calculate the rotation of the model.
	D3DXMatrixRotationX(&matrixRotationX, GetRotationX());
	D3DXMatrixRotationY(&matrixRotationY, GetRotationY());
	D3DXMatrixRotationZ(&matrixRotationZ, GetRotationZ());

	// Calculate scaling.
	D3DXMatrixScaling(&scale, GetScaleX(), GetScaleY(), GetScaleZ());

	// Calculate the translation of the model.
	D3DXMatrixTranslation(&translation, GetPosX(), GetPosY(), GetPosZ());

	// Calculate the world matrix
	mWorldMatrix = scale * matrixRotationX * matrixRotationY * matrixRotationZ * translation;
}

void CModel::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	mpVertexManager->RenderBuffers(deviceContext, mpIndexBuffer);
}

bool CModel::SetGeometry(D3DXVECTOR3 * vertices, unsigned long* indices, D3DXVECTOR2* UV, D3DXVECTOR3* normals)
{
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mpVertexManager->SetNumberOfVertices(mVerticesCount);

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f, vertices, UV, normals);

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

bool CModel::SetGeometry(D3DXVECTOR3 * vertices, unsigned long* indices, D3DXVECTOR4* colours)
{
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mpVertexManager->SetNumberOfVertices(mVerticesCount);

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f, vertices, colours);

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
