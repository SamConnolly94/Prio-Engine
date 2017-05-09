#include "Foliage.h"



CFoliage::CFoliage()
{
	mFoliageTranslation = { 0.0f, 0.0f, 0.0f };
	mpInstanceBuffer = nullptr;
	mFoliageMinCuttoff = 120.0f;
	mFoliageMaxCutoff = 125.0f;
	mWindStrength = 1.0f;
}


CFoliage::~CFoliage()
{
}

bool CFoliage::Initialise(ID3D11Device * device, CTerrainTile** terrainTiles, int terrainWidth, int terrainHeight)
{
	///////////////////////////
	// Foliage textures
	///////////////////////////

	mpQuadMesh = new CFoliageQuad();
	if (!mpQuadMesh->Initialise(device))
	{
		logger->GetInstance().WriteLine("Failed to initialise the foliage quad mesh.");
		return false;
	}

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

	mpReedsTexture = new CTexture();
	if (!mpReedsTexture->Initialise(device, "Resources/Textures/Foliage/Reeds.png"))
	{
		logger->GetInstance().WriteLine("Failed to load 'Resources/Textures/Foliage/Reeds.png'.");
	}

	mpReedsAlphaTexture = new CTexture();
	if (!mpReedsAlphaTexture->Initialise(device, "Resources/Textures/Foliage/ReedsAlpha.png"))
	{
		logger->GetInstance().WriteLine("Failed to load 'Resources/Textures/Foliage/ReedsAlpha.png'.");
	}

	if (!InitialiseBuffers(device, terrainTiles, mWidth, mHeight))
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

	if (mpReedsTexture)
	{
		mpReedsTexture->Shutdown();
		delete mpReedsTexture;
		mpReedsTexture = nullptr;
	}

	if (mpReedsAlphaTexture)
	{
		mpReedsAlphaTexture->Shutdown();
		delete mpReedsAlphaTexture;
		mpReedsAlphaTexture = nullptr;
	}

	ShutdownHeightMap();

	ShutdownQuads();

	if (mpInstanceBuffer)
	{
		mpInstanceBuffer->Release();
		mpInstanceBuffer = nullptr;
	}
}

void CFoliage::Update(float updateTime)
{
	mFoliageTranslation += (mWindDirection * mWindStrength) * updateTime;

	if ((mFoliageTranslation.z > 0.5f && mWindDirection.z > 0.0f) || (mFoliageTranslation.z < 0.0f && mWindDirection.z < 0.0f)
		|| (mFoliageTranslation.x > 0.5f && mWindDirection.x > 0.0f) || (mFoliageTranslation.x < 0.0f && mWindDirection.x < 0.0f)
		)
	{
		mWindDirection = -mWindDirection;
		//mWindDirection -= (mWindDirection * mWindStrength) * updateTime;
	}
}

bool CFoliage::InitialiseBuffers(ID3D11Device * device, CTerrainTile** terrainTiles, int terrainWidth, int terrainHeight)
{

	if (terrainWidth != mWidth && terrainHeight != mHeight)
	{
		logger->GetInstance().WriteLine("The height of the terrain height map did not match that of the foliage height map. Can not create foliage.");
		return false;
	}

	///////////////////////////////////
	// Plot vertices.
	///////////////////////////////////

	for (int heightCount = 0; heightCount < mHeight; heightCount++)
	{
		for (int widthCount = 0; widthCount < mWidth; widthCount++)
		{
			if (mpHeightMap[heightCount][widthCount] > mFoliageMinCuttoff && mpHeightMap[heightCount][widthCount] < mFoliageMaxCutoff)
			{
				D3DXVECTOR3 LL = terrainTiles[heightCount][widthCount].GetLowerLeftVertexPosition();
				D3DXVECTOR3 LR = terrainTiles[heightCount][widthCount].GetLowerRightVertexPosition();
				D3DXVECTOR3 UL = terrainTiles[heightCount][widthCount].GetUpperLeftVertexPosition();
				D3DXVECTOR3 UR = terrainTiles[heightCount][widthCount].GetUpperRightVertexPosition();
				D3DXVECTOR3 centrePos = terrainTiles[heightCount][widthCount].GetCentrePosition();

				InstanceType info;
				
				info.TileLLVertexPos = LL;
				info.TileLRVertexPos = LR;
				info.TileULVertexPos = UL;
				info.TileURVertexPos = UR;
				info.TileCentrePos = centrePos;

				//info.Rotation = rotation;
				mIstanceInfoList.push_back(info);
			}
		}
	}

	mInstanceCount = mIstanceInfoList.size();

	InstanceType* instances = new InstanceType[mInstanceCount];

	// Copy instance data.
	for (int i = 0; i < mInstanceCount; i++)
	{
		instances[i].TileCentrePos = mIstanceInfoList[i].TileCentrePos;
		instances[i].TileLLVertexPos = mIstanceInfoList[i].TileLLVertexPos;
		instances[i].TileLRVertexPos = mIstanceInfoList[i].TileLRVertexPos;
		instances[i].TileULVertexPos = mIstanceInfoList[i].TileULVertexPos;
		instances[i].TileURVertexPos = mIstanceInfoList[i].TileURVertexPos;
	}
	D3D11_BUFFER_DESC instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA instanceData;

	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType) * mInstanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &mpInstanceBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the instance buffer for foliage.");
		return false;
	}

	delete[] instances;
	instances = nullptr;

	return true;
}

void CFoliage::ShutdownQuads()
{
	mpQuadMesh->Shutdown();
	delete mpQuadMesh;
	mpQuadMesh = nullptr;
}

void CFoliage::SetWidth(int width)
{
	mWidth = width;
}

void CFoliage::SetHeight(int height)
{
	mHeight = height;
}

void CFoliage::LoadHeightMap(double ** heightMap)
{
	if (mpHeightMap != nullptr)
	{
		ShutdownHeightMap();
	}

	if (mpHeightMap == nullptr)
	{
		mpHeightMap = new double*[mHeight];

		for (int y = 0; y < mHeight; y++)
		{
			// Allocate space for the columns.
			mpHeightMap[y] = new double[mWidth];

			for (int x = 0; x < mWidth; x++)
			{
				mpHeightMap[y][x] = heightMap[y][x];
			}
		}
	}


	// Outpout a log to let the user know where we're up to in the function.
	logger->GetInstance().WriteLine("Copied height map over to foliage.");
}

bool CFoliage::LoadHeightMap(std::string filename)
{
	std::string line;
	std::ifstream inFile;

	if (mpHeightMap != nullptr)
	{
		ShutdownHeightMap();
	}

	// Open the file.
	inFile.open(filename);

	// Check we successfully opened.
	if (!inFile.is_open())
	{
		logger->GetInstance().WriteLine("Failed to open the map file with name: " + filename);
		return false;
	}

	int height = 0;
	int width = 0;
	// Calculate the array size for now.
	while (std::getline(inFile, line))
	{
		// Reset the width count.
		width = 0;

		double value;
		std::stringstream  lineStream(line);

		// Go through this line.
		while (lineStream >> value)
		{
			// One more on the width!
			width++;
		}

		// Increment height.
		height++;
	}

	// Set width and height.
	mWidth = width;
	mHeight = height;

	inFile.close();
	inFile.open(filename);

	if (!inFile.is_open())
	{
		logger->GetInstance().WriteLine("Failed to open " + filename + ", but managed to open it the first time.");
		return false;
	}

	// Create height map.
	if (!mpHeightMap)
	{
		// Allocate memory to this array.
		mpHeightMap = new double*[mHeight];
	}

	// Iterate through all the rows.
	for (int x = 0; x < mHeight; x++)
	{
		// Allocate space for the columns.
		mpHeightMap[x] = new double[mWidth];
	}

	// Create a heightmap.
	for (int y = 0; y < mHeight; y++)
	{
		// Get the line of this file.
		std::getline(inFile, line);
		std::stringstream  lineStream(line);
		double value;

		// Iterate through the width.
		for (int x = 0; x < mWidth; x++)
		{
			lineStream >> value;
			mpHeightMap[y][x] = value;
		}
	}

	return true;
}

int CFoliage::GetInstanceCount()
{
	return mInstanceCount;
}

void CFoliage::RenderBuffers(ID3D11DeviceContext * deviceContext, int quadIndex, int triangleIndex)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPtrs[2];

	strides[0] = sizeof(CFoliageQuad::FoliageVertexType);
	strides[1] = sizeof(InstanceType);

	offsets[0] = 0;
	offsets[1] = 0;
	if (quadIndex == 0)
	{
		if (triangleIndex == 0)
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect1Tri1();
		}
		else
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect1Tri2();
		}
	}
	else if (quadIndex == 1)
	{
		if (triangleIndex == 0)
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect2Tri1();
		}
		else
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect2Tri2();
		}
	}
	else
	{
		if (triangleIndex == 0)
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect3Tri1();
		}
		else
		{
			bufferPtrs[0] = mpQuadMesh->GetVertexBufferRect3Tri2();
		}
	}
	bufferPtrs[1] = mpInstanceBuffer;

	deviceContext->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool CFoliage::UpdateBuffers(ID3D11Device* device, double** heightMap, int mapWidth, int mapHeight, CTerrainTile** terrainTiles, int terrainWidth, int terrainHeight)
{
	Shutdown();
	mIstanceInfoList.clear();

	SetWidth(mapWidth);
	SetHeight(mapHeight);
	LoadHeightMap(heightMap);

	bool result = Initialise(device, terrainTiles, terrainWidth, terrainHeight);

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise the foliage buffers when updating with new height map.");
		return false;
	}
	
	return true;
}

void CFoliage::SetWindDirection(D3DXVECTOR3 windDir)
{
	mWindDirection = windDir;
}

void CFoliage::SetWindStrength(float value)
{
	mWindStrength = value;
}

float CFoliage::GetWindStrength()
{
	return mWindStrength;
}

void CFoliage::ShutdownHeightMap()
{
	if (mpHeightMap != nullptr)
	{
		for (int i = 0; i < mHeight; ++i) 
		{
			delete[] mpHeightMap[i];
			logger->GetInstance().MemoryDeallocWriteLine(typeid(mpHeightMap[i]).name());
			mpHeightMap[i] = nullptr;
		}
		delete[] mpHeightMap;
		mpHeightMap = nullptr;
		logger->GetInstance().MemoryDeallocWriteLine(typeid(mpHeightMap).name());
	}
}
