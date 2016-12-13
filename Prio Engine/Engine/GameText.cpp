#include "GameText.h"



CGameText::CGameText()
{
	mpFont = nullptr;
	mpFontShader = nullptr;
}


CGameText::~CGameText()
{
}

bool CGameText::Initialise(ID3D11Device * device, ID3D11DeviceContext * deviceContext, HWND hWnd, int screenWidth, int screenHeight, D3DXMATRIX baseViewMatrix)
{
	bool result;

	// Store the screen width and height.
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	// Store the base matrix view.
	mBaseViewMatrix = baseViewMatrix;

	// Create font obj.
	mpFont = new CGameFont();
	if (!mpFont)
	{
		gLogger->WriteLine("Failed to allocate memory to the font pointer in CGameText.");
		return false;
	}
	gLogger->MemoryAllocWriteLine(typeid(mpFont).name());
	char* fontDataFile = "Resources/Fonts/defaultFontData.txt";
	WCHAR* fontTextureFile = L"Resources/Fonts/font.dds";

	// Initailise the font.
	result = mpFont->Initialise(device, fontDataFile, fontTextureFile);

	if (!result)
	{
		gLogger->WriteLine("Failed to load Fonts/defaultData.txt or Fonts/font.dds or both.");
		return false;
	}

	// Initialise the font shader.
	mpFontShader = new CFontShader();
	result = mpFontShader->Initialise(device, hWnd);

	if (!result)
	{
		gLogger->WriteLine("Failed to initialise the font object in GameText.cpp.");
		return false;
	}
	
	SentenceType* sentence;

	result = InitialiseSentence(sentence, 300, device);
	if (!result)
	{
		gLogger->WriteLine("Failed to initialise sentences.");
		return false;
	}

	result = UpdateSentence(sentence, "Hello, is it me yo lookin fo?", 100, 100, 1.0f, 1.0f, 1.0f, deviceContext);

	if (!result)
	{
		gLogger->WriteLine("Failed to update sentence.");
		return false;
	}

	return true;
}

void CGameText::Shutdown()
{
	// Release all sentences.
	std::list<SentenceType*>::iterator it = mpSentences.begin();
	while (it != mpSentences.end())
	{
		ReleaseSentence((*it));
		(*it) = nullptr;
		it++;
	}

	if (mpFontShader)
	{
		mpFontShader->Shutdown();
		delete mpFontShader;
		mpFontShader = nullptr;
	}

	if (mpFont)
	{
		mpFont->Shutdown();
		delete mpFont;
		mpFont = nullptr;
	}
}

// Attempt to render all texts.
bool CGameText::Render(ID3D11DeviceContext * deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	// Boolean flag to check whetehr things are succesfully rendered.
	bool result = false;

	// Go through list and render all sentences.
	std::list<SentenceType*>::iterator it = mpSentences.begin();
	
	// While we haven't hit the end of the list.
	while (it != mpSentences.end())
	{
		// Attempt to render the sentence and track the results.
		result = RenderSentence(deviceContext, (*it), worldMatrix, orthoMatrix);

		// If we failed the result.
		if (!result)
		{
			// Output error message to the text log.
			gLogger->WriteLine("Failed to render text.");
			// Return false.
			return false;
		}
		it++;
	}

	// Success!
	return true;
}

bool CGameText::InitialiseSentence(SentenceType* &sentence, int maxLength, ID3D11Device * device)
{
	vertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	// Allocate memory to the sentence object.
	sentence = new SentenceType();
	if (!sentence)
	{
		gLogger->WriteLine("Failed to allocate memory to a sentence object in GameText.cpp.");
		return false;
	}
	gLogger->MemoryAllocWriteLine(typeid(sentence).name());

	// Track the pointer on our sentences list, don't want to lose this.
	mpSentences.push_back(sentence);

	// Initialise buffer properties to be null.
	(sentence)->vertexBuffer = nullptr;
	(sentence)->indexBuffer = nullptr;

	// Set max length of the sentence.
	(sentence)->maxLength = maxLength;

	// Set number of vertices in vertex array.
	(sentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array to be equal to the number of vertices.
	(sentence)->indexCount = (sentence)->vertexCount;

	// Create vertex array.
	vertices = new vertexType[(sentence)->vertexCount];
	if (!vertices)
	{
		gLogger->WriteLine("Failed to allocate memory to the vertices buffer in GameText.cpp.");
		return false;
	}

	// Create the index array.
	indices = new unsigned long[(sentence)->indexCount];
	if (!indices)
	{
		gLogger->WriteLine("Failed to allocate memory to the indices buffer in GameText.cpp.");
		return false;
	}
	// Set vertex array to 0's to begin with.
	memset(vertices, 0, (sizeof(vertexType) * (sentence)->vertexCount));

	// Init index array.
	for (int i = 0; i < (sentence)->indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up descriptor for vertex buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(vertexType) * (sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Pass subresource ptr to vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the buffer and store it in sentence objects properties.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(sentence)->vertexBuffer);

	if (FAILED(result))
	{
		gLogger->WriteLine("Failed to create the vertex buffer in DirectX after passing it vertex data in GameText.cpp.");
		return false;
	}

	// Set up descriptor for index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give subresource pointer to index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(sentence)->indexBuffer);

	if (FAILED(result))
	{
		gLogger->WriteLine("Faield to create the index buffer in DirectX after passing it index data in GameText.cpp.");
		return false;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

bool CGameText::UpdateSentence(SentenceType * sentence, char * text, int posX, int posY, float red, float green, float blue, ID3D11DeviceContext * deviceContext)
{
	int numberOfLetters;
	vertexType* vertices;
	float x;
	float y;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	vertexType* verticesPtr;

	// Store the colour which the sentence will be drawn in.
	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	// Get the number of letters in the sentence.
	numberOfLetters = static_cast<int>(strlen(text));
	
	// Check for buffer overflow.
	if (numberOfLetters > sentence->maxLength)
	{
		gLogger->WriteLine("Sentence was too long. Would have caused buffer overflow.");
		return false;
	}

	// Create vertex array.
	vertices = new vertexType[sentence->vertexCount];
	
	// Check vertices array was allocated memory.
	if (!vertices)
	{
		gLogger->WriteLine("Failed to allocate memory to vertices array in GameText.cpp.");
		return false;
	}

	// Initialise the array to be 0.
	memset(vertices, 0, (sizeof(vertexType) * sentence->vertexCount));

	// Calculate positions which we will draw the text.
	x = static_cast<float>(((mScreenWidth / 2) * -1) + posX);
	y = static_cast<float>((mScreenHeight / 2) + posY);

	// Build the vertex array.
	mpFont->BuildVertexArray(vertices, text, x, y);

	// Lock vertex array.
	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		gLogger->WriteLine("Failed to map the resource when updating sentence in GameText.cpp.");
		return false;
	}

	// Grab pointer to data in vertex buffer.
	verticesPtr = (vertexType*)mappedResource.pData;

	// Copy the data into vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(vertexType) * sentence->vertexCount));

	// Unlock vertex buff.
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	// Release vertex array.
	delete[] vertices;
	vertices = nullptr;

	return true;
}

void CGameText::ReleaseSentence(SentenceType* sentence)
{
	if (sentence)
	{
		// Vertex buffer isn't null.
		if ((sentence)->vertexBuffer)
		{
			//delete[](sentence)->vertexBuffer;
			sentence->vertexBuffer->Release();
			(sentence)->vertexBuffer = nullptr;
		}

		// Index buffer isn't null.
		if ((sentence)->indexBuffer)
		{
			//delete[](sentence)->indexBuffer;
			sentence->indexBuffer->Release();
			(sentence)->indexBuffer = nullptr;
		}

		delete (sentence);
		(sentence) = nullptr;
	}
}

bool CGameText::RenderSentence(ID3D11DeviceContext * deviceContext, SentenceType * sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	unsigned int stride;
	unsigned int offset;
	D3DXVECTOR4 pixelColour;
	bool result;

	// Set vertex buffer stride.
	stride = sizeof(vertexType);
	offset = 0;

	// Set vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

	// Set index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set type of primitives to be used when rendering.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create a pixel colour with sentence input colour.
	pixelColour = D3DXVECTOR4{ sentence->red, sentence->green, sentence->blue, 1.0f };

	// Render using font shader.
	result = mpFontShader->Render(deviceContext, sentence->indexCount, worldMatrix, mBaseViewMatrix, orthoMatrix, mpFont->GetTexture(), pixelColour);

	if (!result)
	{
		gLogger->WriteLine("Failed to render text in GameText.cpp.");
		return false;
	}

	return true;
}
