#include "FoliageShader.h"

CFoliageShader::CFoliageShader()
{
	mpVertexShader = nullptr;
	mpPixelShader = nullptr;
	mpLayout = nullptr;
	mpSampleState = nullptr;
}


CFoliageShader::~CFoliageShader()
{
}

bool CFoliageShader::Initialise(ID3D11Device * device, HWND hwnd)
{
	bool result;

	// Initialise the vertex pixel shaders.
	result = InitialiseShader(device, hwnd, "Shaders/Foliage.vs.hlsl", "Shaders/Foliage.ps.hlsl");

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialsie the foliage shader class.");
		return false;
	}

	return true;
}

void CFoliageShader::Shutdown()
{
	// Shutodwn the vertex and pixel shaders as well as all related objects.
	ShutdownShader();
}

bool CFoliageShader::Render(ID3D11DeviceContext * deviceContext, int vertexCount, int instanceCount)
{
	bool result;

	// Set the shader parameters that will be used for rendering.
	result = SetShaderParameters(deviceContext);
	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to set the shader parameters in foliage shader.");
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, vertexCount, instanceCount);

	return true;
}

bool CFoliageShader::InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[6];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Initialise pointers in this function to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename.c_str(), NULL, NULL, "FoliageVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + vsFilename + "'");
			MessageBox(hwnd, vsFilename.c_str(), "Missing shader file. ", MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the vertex shader named '" + vsFilename + "'");
		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename.c_str(), NULL, NULL, "FoliagePS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else
		{
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + psFilename + "'");
			MessageBox(hwnd, psFilename.c_str(), "Missing shader file.", MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + psFilename + "'");
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mpVertexShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the vertex shader from the buffer.");
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the pixel shader from the buffer.");
		return false;
	}

	int polyIndex = 0;

	// Setup the layout of the data that goes into the shader.
	polygonLayout[polyIndex].SemanticName = "POSITION";
	polygonLayout[polyIndex].SemanticIndex = 0;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[polyIndex].InputSlot = 0;
	polygonLayout[polyIndex].AlignedByteOffset = 0;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 0;

	polyIndex = 1;

	// Position only has 2 co-ords. Only need format of R32G32.
	polygonLayout[polyIndex].SemanticName = "TEXCOORD";
	polygonLayout[polyIndex].SemanticIndex = 0;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[polyIndex].InputSlot = 0;
	polygonLayout[polyIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 0;

	polyIndex = 2;

	polygonLayout[polyIndex].SemanticName = "NORMAL";
	polygonLayout[polyIndex].SemanticIndex = 0;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[polyIndex].InputSlot = 0;
	polygonLayout[polyIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 0;
	
	polyIndex = 3;

	polygonLayout[polyIndex].SemanticName = "TEXCOORD";
	polygonLayout[polyIndex].SemanticIndex = 1;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32_UINT;
	polygonLayout[polyIndex].InputSlot = 0;
	polygonLayout[polyIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 0;

	polyIndex = 4;

	polygonLayout[polyIndex].SemanticName = "TEXCOORD";
	polygonLayout[polyIndex].SemanticIndex = 2;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32_UINT;
	polygonLayout[polyIndex].InputSlot = 0;
	polygonLayout[polyIndex].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 0;

	polyIndex = 5;

	polygonLayout[polyIndex].SemanticName = "TEXCOORD";
	polygonLayout[polyIndex].SemanticIndex = 3;
	polygonLayout[polyIndex].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[polyIndex].InputSlot = 1;
	polygonLayout[polyIndex].AlignedByteOffset = 0;
	polygonLayout[polyIndex].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	polygonLayout[polyIndex].InstanceDataStepRate = 1;

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mpLayout);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create polygon layout.");
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	if (!SetupMatrixBuffer(device))
	{
		logger->GetInstance().WriteLine("Failed to set up matrix buffer in foliage shader class.");
		return false;
	}

	// Set up the sampler state descriptor.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &mpSampleState);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the sampler state in foliage shader class.");
		return false;
	}

	////////////////////////////////
	// Light buffer
	///////////////////////////////

	D3D11_BUFFER_DESC lightBufferDesc;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &mpLightBuffer);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the buffer pointer to access the light buffer from foilage shader class.");
		return false;
	}

	////////////////////////////////
	// Compliment buffer
	///////////////////////////////

	D3D11_BUFFER_DESC foliageBufferDesc;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	foliageBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	foliageBufferDesc.ByteWidth = sizeof(FoliageBufferType);
	foliageBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	foliageBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	foliageBufferDesc.MiscFlags = 0;
	foliageBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&foliageBufferDesc, NULL, &mpFoliageBuffer);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the buffer pointer to access the foliage buffer from foilage shader class.");
		return false;
	}

	return true;
}

void CFoliageShader::ShutdownShader()
{
	if (mpSampleState)
	{
		mpSampleState->Release();
		mpSampleState = nullptr;
	}

	if (mpLayout)
	{
		mpLayout->Release();
		mpLayout = nullptr;
	}

	if (mpPixelShader)
	{
		mpPixelShader->Release();
		mpPixelShader = nullptr;
	}

	if (mpVertexShader)
	{
		mpVertexShader->Release();
		mpVertexShader = nullptr;
	}
}

void CFoliageShader::OutputShaderErrorMessage(ID3D10Blob * errorMessage, HWND hwnd, std::string shaderFilename)
{
	std::string errMsg;
	char* compileErrors;
	unsigned long bufferSize;

	// Grab pointer to the compile errors.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Reset string to store message in to be empty.
	errMsg = "";

	// Compile the error message into a string variable.
	for (unsigned int i = 0; i < bufferSize; i++)
	{
		errMsg += compileErrors[i];
	}

	// Write the error string to the logs.
	logger->GetInstance().WriteLine(errMsg);

	// Clean up the BLOB file used to store the error message.
	errorMessage->Release();
	errorMessage = nullptr;

	// Output a message box containing info describing what went wrong. Redirect to the logs.
	MessageBox(hwnd, "Error compiling the shader. Check the logs for a more detailed error message.", shaderFilename.c_str(), MB_OK);
}

bool CFoliageShader::SetShaderParameters(ID3D11DeviceContext * deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;


	bufferNumber = 0;

	if (!SetMatrixBuffer(deviceContext, bufferNumber, ShaderType::Vertex))
	{
		logger->GetInstance().WriteLine("Failed to set the matrix buffer in terrain shader.");
		return false;
	}

	/////////////////////////////
	// Light buffer
	/////////////////////////////

	// Lock the matrix buffer for writing to.
	result = deviceContext->Map(mpLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// If we did not successfully lock the constant buffer.
	if (FAILED(result))
	{
		// Output error message to the logs.
		logger->GetInstance().WriteLine("Failed to lock the light buffer before writing to it in shader class.");
		return false;
	}

	// Grab pointer to the matrix const buff.
	LightBufferType* lightBufferPtr = static_cast<LightBufferType*>(mappedResource.pData);

	// Set data in the structure.
	lightBufferPtr->AmbientColour = mAmbientColour;
	lightBufferPtr->DiffuseColour = mDiffuseColour;
	lightBufferPtr->LightDirection = mLightDirection;
	lightBufferPtr->padding = 0.0f;

	// Unlock the const buffer and write modifications to it.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Pass buffer to shader
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	/////////////////////////////
	// Foliage buffer
	/////////////////////////////

	// Lock the matrix buffer for writing to.
	result = deviceContext->Map(mpFoliageBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// If we did not successfully lock the constant buffer.
	if (FAILED(result))
	{
		// Output error message to the logs.
		logger->GetInstance().WriteLine("Failed to lock the foliage buffer before writing to it in shader class.");
		return false;
	}

	// Grab pointer to the matrix const buff.
	FoliageBufferType* foliageBufferPtr = static_cast<FoliageBufferType*>(mappedResource.pData);

	// Set data in the structure.
	foliageBufferPtr->FrameTime = mFrameTime;
	foliageBufferPtr->WindDirection = mWindDirection;
	foliageBufferPtr->WindStrength = mStrength;
	foliageBufferPtr->FoliageTranslation = mTranslation;

	// Unlock the const buffer and write modifications to it.
	deviceContext->Unmap(mpFoliageBuffer, 0);

	// Pass buffer to shader
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpFoliageBuffer);

	/////////////////////////////
	// Resources buffer
	/////////////////////////////

	// Set the shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &mpGrassTexture);
	deviceContext->PSSetShaderResources(1, 1, &mpAlphaTexture);
	deviceContext->PSSetShaderResources(2, 1, &mpReedTexture);
	deviceContext->PSSetShaderResources(3, 1, &mpReedAlphaTexture);

	return true;
}

void CFoliageShader::RenderShader(ID3D11DeviceContext * deviceContext, int vertexCount, int instanceCount)
{
	// Set the vertex input layout
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpPixelShader, NULL, 0);

	// Set sample state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpSampleState);
	
	deviceContext->DrawInstanced(vertexCount, instanceCount, 0, 0);
}

void CFoliageShader::SetGrassTexture(ID3D11ShaderResourceView * grassTexture)
{
	mpGrassTexture = grassTexture;
}

void CFoliageShader::SetGrassAlphaTexture(ID3D11ShaderResourceView * alphaTexture)
{
	mpAlphaTexture = alphaTexture;
}

void CFoliageShader::SetReedTexture(ID3D11ShaderResourceView * reedTexture)
{
	mpReedTexture = reedTexture;
}

void CFoliageShader::SetReedAlphaTexture(ID3D11ShaderResourceView * alphaTexture)
{
	mpReedAlphaTexture = alphaTexture;
}

void CFoliageShader::SetAmbientColour(D3DXVECTOR4 ambientColour)
{
	mAmbientColour = ambientColour;
}

void CFoliageShader::SetDiffuseColour(D3DXVECTOR4 diffuseColour)
{
	mDiffuseColour = diffuseColour;
}

void CFoliageShader::SetLightDirection(D3DXVECTOR3 lightDirection)
{
	mLightDirection = lightDirection;
}

void CFoliageShader::SetWindDirection(D3DXVECTOR3 direction)
{
	mWindDirection;
}

void CFoliageShader::SetFrameTime(float frameTime)
{
	mFrameTime = frameTime;
}

void CFoliageShader::SetWindStrength(float strength)
{
	mStrength = strength;
}

void CFoliageShader::SetTranslation(D3DXVECTOR3 translation)
{
	mTranslation = translation;
}
