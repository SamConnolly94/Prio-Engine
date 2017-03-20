#include "RefractReflectShader.h"



CRefractionShader::CRefractionShader()
{
	mpVertexShader = nullptr;
	mpRefractionPixelShader = nullptr;
	mpLayout = nullptr;
	mpTrilinearWrap = nullptr;
	mpMatrixBuffer = nullptr;
	mpLightBuffer = nullptr;
	mpViewportBuffer = nullptr;
	mpReflectionPixelShader = nullptr;
}


CRefractionShader::~CRefractionShader()
{
}

bool CRefractionShader::Initialise(ID3D11Device * device, HWND hwnd)
{
	bool result;

	// Initialise the vertex pixel shaders.
	result = InitialiseShader(device, hwnd, "Shaders/RefractedModel.vs.hlsl", "Shaders/TerrainRefraction.ps.hlsl", "Shaders/TerrainReflection.ps.hlsl");

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise refraction shader.");
		return false;
	}

	return true;
}

void CRefractionShader::Shutdown()
{
	// Shutodwn the vertex and pixel shaders as well as all related objects.
	ShutdownShader();
}

bool CRefractionShader::RefractionRender(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour,
	D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView* waterHeightMap, ID3D11ShaderResourceView* refractionTex)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, indexCount, worldMatrix, viewMatrix, projMatrix, lightDirection, ambientColour, diffuseColour, viewportSize, waterHeightMap, refractionTex);

	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderRefractionShader(deviceContext, indexCount);

	return true;
}

bool CRefractionShader::ReflectionRender(ID3D11DeviceContext * deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour, D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView * waterHeightMap, ID3D11ShaderResourceView * refractionTex)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, indexCount, worldMatrix, viewMatrix, projMatrix, lightDirection, ambientColour, diffuseColour, viewportSize, waterHeightMap, refractionTex);

	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderReflectionShader(deviceContext, indexCount);

	return true;
}

bool CRefractionShader::InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename, std::string reflectionPSFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC viewportBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;


	// Initialise pointers in this function to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename.c_str(), NULL, NULL, "RefractionVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			std::string errMsg = "Missing shader file. ";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + vsFilename + "'");
			MessageBox(hwnd, vsFilename.c_str(), errMsg.c_str(), MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the vertex shader named '" + vsFilename + "'");
		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(psFilename.c_str(), NULL, NULL, "TerrainRefractionPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		else
		{
			std::string errMsg = "Missing shader file.";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + psFilename + "'");
			MessageBox(hwnd, psFilename.c_str(), errMsg.c_str(), MB_OK);
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
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpRefractionPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the pixel shader from the buffer.");
		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(reflectionPSFilename.c_str(), NULL, NULL, "TerrainReflectionPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, reflectionPSFilename);
		}
		else
		{
			std::string errMsg = "Missing shader file.";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + reflectionPSFilename + "'");
			MessageBox(hwnd, reflectionPSFilename.c_str(), errMsg.c_str(), MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + reflectionPSFilename + "'");
		return false;
	}
	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpReflectionPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the reflection pixel shader from the buffer.");
		return false;
	}

	/*
	* The polygonLayout.Format describes what size item should be placed in here, check if it's a float3 or float2 basically, and pass in DXGI_FORMAT_R32/G32/B32_FLOAT accordingly.
	*/

	// Setup the layout of the data that goes into the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	// Position only has 2 co-ords. Only need format of R32G32.
	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

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

	// Set up the sampler state descriptor.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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
	result = device->CreateSamplerState(&samplerDesc, &mpTrilinearWrap);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the sampler state in RefractionShader.cpp");
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &mpBilinearMirror);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the bilinear mirror sampler state in RefractionShader.cpp");
		return false;
	}

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;


	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &mpMatrixBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the buffer pointer to access the vertex shader from within the water shader class.");
		return false;
	}

	//////////////////////////////////
	// Set up viewport buffer
	/////////////////////////////////
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	viewportBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewportBufferDesc.ByteWidth = sizeof(ViewportBufferType);
	viewportBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewportBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewportBufferDesc.MiscFlags = 0;
	viewportBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&viewportBufferDesc, NULL, &mpViewportBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the viewport buffer to from within the refraction shader class.");
		return false;
	}

	//////////////////////////////////
	// Set up light buffer
	/////////////////////////////////
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
		logger->GetInstance().WriteLine("Failed to create the light buffer to from within the refraction shader class.");
		return false;
	}

	return true;
}

void CRefractionShader::ShutdownShader()
{
	if (mpTrilinearWrap)
	{
		mpTrilinearWrap->Release();
		mpTrilinearWrap = nullptr;
	}

	if (mpMatrixBuffer)
	{
		mpMatrixBuffer->Release();
		mpMatrixBuffer = nullptr;
	}

	if (mpLightBuffer)
	{
		mpLightBuffer->Release();
		mpLightBuffer = nullptr;
	}

	if (mpViewportBuffer)
	{
		mpViewportBuffer->Release();
		mpViewportBuffer = nullptr;
	}

	if (mpLayout)
	{
		mpLayout->Release();
		mpLayout = nullptr;
	}

	if (mpRefractionPixelShader)
	{
		mpRefractionPixelShader->Release();
		mpRefractionPixelShader = nullptr;
	}

	if (mpVertexShader)
	{
		mpVertexShader->Release();
		mpVertexShader = nullptr;
	}
}

void CRefractionShader::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename)
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

bool CRefractionShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projMatrix, D3DXVECTOR3 lightDirection, D3DXVECTOR4 ambientColour, D3DXVECTOR4 diffuseColour,
	D3DXVECTOR2 viewportSize, ID3D11ShaderResourceView* waterHeightMap, ID3D11ShaderResourceView* refractionTex)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* matrixBufferPtr;
	LightBufferType* lightBufferPtr;
	ViewportBufferType* viewportBufferPtr;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projMatrix, &projMatrix);

	////////////////////////////
	// Update the matrices constant buffer.
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer to write to the matrices values in refraction shader.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);

	//////////////////////////////
	// Update the viewport constant buffer.
	//////////////////////////////

	result = deviceContext->Map(mpViewportBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the clip plane constant buffer in refraction shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	viewportBufferPtr = (ViewportBufferType*)mappedResource.pData;

	// Copy the current water buffer values into the constant buffer.
	viewportBufferPtr->ViewportSize = viewportSize;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpViewportBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 0;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpViewportBuffer);

	/////////////////////////////////
	// Update the light constant buffer.
	/////////////////////////////////
	result = deviceContext->Map(mpLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the light constant buffer in refraction shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	lightBufferPtr = (LightBufferType*)mappedResource.pData;

	// Copy the current light values into the constant buffer.
	lightBufferPtr->AmbientColour = ambientColour;
	lightBufferPtr->DiffuseColour = diffuseColour;
	lightBufferPtr->LightDirection = lightDirection;
	lightBufferPtr->padding = 0.0f;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	/////////////////////////////////
	// Update shader resources
	/////////////////////////////////
	deviceContext->PSSetShaderResources(0, 1, &waterHeightMap);
	deviceContext->PSSetShaderResources(1, 1, &refractionTex);

	return true;
}

void CRefractionShader::RenderReflectionShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpReflectionPixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpTrilinearWrap);
	deviceContext->PSSetSamplers(1, 1, &mpBilinearMirror);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CRefractionShader::RenderRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpRefractionPixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpTrilinearWrap);
	deviceContext->PSSetSamplers(1, 1, &mpBilinearMirror);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
