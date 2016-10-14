#include "Models.h"


CModels::CModels(ID3D11Device * device)
{
	mpDevice = device;
	mpVertexManager = new CVertexManager(PrioEngine::VertexType::Texture);
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

bool CModels::SetGeometry(D3DXVECTOR3 * vertices, D3DXVECTOR3* texCoords, D3DXVECTOR3 * normals)
{
	mpVertices = vertices;
	mpTexCoords = texCoords;
	mpNormal = normals;


	unsigned long* indices;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Set the number of vertices in the vertex array.
	mpVertexManager->SetNumberOfVertices(GetNumberOfVertices());

	// Create a vertex array
	mpVertexManager->CreateVertexArray();

	// Create the points of the model.
	mpVertexManager->SetVertexArray(0.0f, 0.0f, 0.0f, vertices, texCoords, normals, GetNumberOfVertices(), GetTextureCount(), GetNumberOfNormals());

	// Create the index array.
	indices = new unsigned long[GetNumberOfIndices()];
	mpLogger->GetLogger().MemoryAllocWriteLine(typeid(indices).name());
	if (!indices)
	{
		return false;
	}

	// Load the index array with data according to the predefined indicies defined in the engines namespace.
	for (int i = 0; i < GetNumberOfIndices(); i++)
	{
		indices[i] = i;
	}

	// Create the vertex buffer.
	if (!mpVertexManager->CreateVertexBuffer())
	{
		return false;
	}

	/* Set up the descriptor of the index buffer. */
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * GetNumberOfIndices();
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

	delete[] indices;
	indices = nullptr;
	mpLogger->GetLogger().MemoryDeallocWriteLine(typeid(indices).name());

	return true;
}
