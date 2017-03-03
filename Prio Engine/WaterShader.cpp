#include "WaterShader.h"


CWaterShader::CWaterShader()
{
	mpVertexShader = nullptr;
	mpPixelShader = nullptr;
	mpLayout = nullptr;
	mpMatrixBuffer = nullptr;
	mpSampleState = nullptr;
	mpReflectionBuffer = nullptr;
	mpWaterBuffer = nullptr;
}

CWaterShader::~CWaterShader()
{
}

bool CWaterShader::Initialise(ID3D11Device * device, HWND hwnd)
{
	bool result;

	// Initialise the vertex pixel shaders.
	result = InitialiseShader(device, hwnd, "Shaders/Water.vs.hlsl", "Shaders/Water.ps.hlsl");

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise water shader.");
		return false;
	}

	return true;
}

void CWaterShader::Shutdown()
{
	// Shutodwn the vertex and pixel shaders as well as all related objects.
	ShutdownShader();
}

bool CWaterShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projMatrix, D3DXMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTex, ID3D11ShaderResourceView* refractionTex,
	ID3D11ShaderResourceView* normalMap, float waterTranslation, float reflectRefractRatio )
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, indexCount, worldMatrix, viewMatrix, projMatrix, reflectionMatrix, reflectionTex, refractionTex,
		normalMap, waterTranslation, reflectRefractRatio);

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to set the shader parameters for water shader.");
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool CWaterShader::InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC reflectionBufferDesc;
	D3D11_BUFFER_DESC waterBufferDesc;

	// Initialise pointers in this function to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename.c_str(), NULL, NULL, "WaterVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
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
	result = D3DX11CompileFromFile(psFilename.c_str(), NULL, NULL, "WaterPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
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
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the pixel shader from the buffer.");
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
	result = device->CreateSamplerState(&samplerDesc, &mpSampleState);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the sampler state in WaterShader.cpp");
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
	
	/////////////////////////
	// Reflection buffer.
	////////////////////////

	reflectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	reflectionBufferDesc.ByteWidth = sizeof(ReflectionBufferType);
	reflectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	reflectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	reflectionBufferDesc.MiscFlags = 0;
	reflectionBufferDesc.StructureByteStride = 0;

	// Create constant buffer.
	result = device->CreateBuffer(&reflectionBufferDesc, NULL, &mpReflectionBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the reflection buffer in water shader class.");
		return false;
	}

	///////////////////////
	// Water buffer.
	///////////////////////
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&waterBufferDesc, NULL, &mpWaterBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the water buffer from provided description in water shader class.");
		return false;
	}

	return true;
}

void CWaterShader::ShutdownShader()
{
	if (mpSampleState)
	{
		mpSampleState->Release();
		mpSampleState = nullptr;
	}

	if (mpMatrixBuffer)
	{
		mpMatrixBuffer->Release();
		mpMatrixBuffer = nullptr;
	}

	if (mpReflectionBuffer)
	{
		mpReflectionBuffer->Release();
		mpReflectionBuffer = nullptr;
	}

	if (mpWaterBuffer)
	{
		mpWaterBuffer->Release();
		mpWaterBuffer = nullptr;
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

void CWaterShader::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename)
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

bool CWaterShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix,
	D3DXMATRIX projMatrix, D3DXMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTex, ID3D11ShaderResourceView* refractionTex,
	ID3D11ShaderResourceView* normalMap, float waterTranslation, float reflectRefractRatio)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* matrixBufferPtr;
	ReflectionBufferType* reflectionBufferPtr;
	WaterBufferType* waterBufferPtr;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projMatrix, &projMatrix);
	D3DXMatrixTranspose(&reflectionMatrix, &reflectionMatrix);
	
	////////////////////////////
	// Update the matrices constant buffer.
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for matrices in WaterShader class.");
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

	/////////////////////////////////
	// Update the reflection constant buffer.
	/////////////////////////////////
	result = deviceContext->Map(mpReflectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the reflection constant buffer in water shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	reflectionBufferPtr = (ReflectionBufferType*)mappedResource.pData;

	// Copy the current value of the reflection matrix into the constant buffer.
	reflectionBufferPtr->reflection = reflectionMatrix;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpReflectionBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpReflectionBuffer);

	//////////////////////////////
	// Set textures in shader.
	/////////////////////////////

	deviceContext->PSSetShaderResources(0, 1, &reflectionTex);
	deviceContext->PSSetShaderResources(1, 1, &refractionTex);
	deviceContext->PSSetShaderResources(2, 1, &normalMap);

	//////////////////////////////
	// Update the water constant buffer.
	//////////////////////////////

	result = deviceContext->Map(mpWaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the water constant buffer in water shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	waterBufferPtr = (WaterBufferType*)mappedResource.pData;

	// Copy the current water buffer values into the constant buffer.
	waterBufferPtr->waterTranslation = waterTranslation;
	waterBufferPtr->reflectRefractRatio = reflectRefractRatio;
	waterBufferPtr->waterBufferPadding = D3DXVECTOR2(0.0F, 0.0F);

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpWaterBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 0;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpWaterBuffer);
	
	return true;
}

void CWaterShader::RenderShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpPixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpSampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
