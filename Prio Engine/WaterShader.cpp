#include "WaterShader.h"

CWaterShader::CWaterShader()
{
	mpVertexShader = nullptr;
	mpSurfacePixelShader = nullptr;
	mpHeightPixelShader = nullptr;
	mpLayout = nullptr;
	mpTrilinearWrap = nullptr;
	mpMatrixBuffer = nullptr;
	mpWaterBuffer = nullptr;
	mpCameraBuffer = nullptr;
	mpViewportBuffer = nullptr;
	mpLightBuffer = nullptr;
}

CWaterShader::~CWaterShader()
{
}

bool CWaterShader::Initialise(ID3D11Device * device, HWND hwnd)
{
	bool result;

	// Initialise the vertex pixel shaders.
	result = InitialiseShader(device, hwnd, "Shaders/WaterSurface.vs.hlsl", "Shaders/WaterSurface.ps.hlsl", "Shaders/WaterHeight.ps.hlsl");

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise the vertex and pixel shaders when initialising the terrain shader class.");
		return false;
	}

	return true;
}

void CWaterShader::Shutdown()
{
	// Shutodwn the vertex and pixel shaders as well as all related objects.
	ShutdownShader();
}

bool CWaterShader::RenderSurface(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
	D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion, float maxDistortion,
	float refractionStrength, float reflectionStrength, D3DXMATRIX cameraMatrix, D3DXVECTOR3 cameraPosition, D3DXVECTOR2 viewportSize, D3DXVECTOR4 ambientColour,
	D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* normalHeightMap, ID3D11ShaderResourceView* refractionMap, ID3D11ShaderResourceView* reflectionMap)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetSurfaceShaderParameters(deviceContext, worldMatrix, viewMatrix, projMatrix, waterSize, waterSize, waterTranslation, waveHeight, waveScale, 
		refractionDistortion, reflectionDistortion, maxDistortion, refractionStrength, reflectionStrength, cameraMatrix, cameraPosition, viewportSize, ambientColour, diffuseColour, lightDirection,
		normalHeightMap, refractionMap, reflectionMap);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderSurfaceShader(deviceContext, indexCount);

	return true;
}

bool CWaterShader::RenderHeight(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
	D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion,
	float maxDistortion, float refractionStrength, float reflectionStrength, ID3D11ShaderResourceView* normalHeightMap)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetHeightShaderParameters(deviceContext, worldMatrix, viewMatrix, projMatrix, waterSize, waterSize, waterTranslation, waveHeight, waveScale,
		refractionDistortion, reflectionDistortion, maxDistortion, refractionStrength, reflectionStrength, normalHeightMap);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderHeightShader(deviceContext, indexCount);

	return true;
}

bool CWaterShader::InitialiseShader(ID3D11Device* device, HWND hwnd, std::string vsFilename, std::string surfacePsFilename, std::string heightPsFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	const int kNumberOfPolygonElements = 3;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[kNumberOfPolygonElements];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC waterBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC viewportBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Initialise pointers in this function to null.
	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	/////////////////////////////////
	// Surface shaders
	////////////////////////////////

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(vsFilename.c_str(), NULL, NULL, "WaterSurfaceVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS || (1 << 0), 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename.c_str());
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
	result = D3DX11CompileFromFile(surfacePsFilename.c_str(), NULL, NULL, "WaterSurfacePS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS || (1 << 0), 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, surfacePsFilename.c_str());
		}
		else
		{
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + surfacePsFilename + "'");
			MessageBox(hwnd, surfacePsFilename.c_str(), "Missing shader file.", MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + surfacePsFilename + "'");
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
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpSurfacePixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the surface pixel shader from the buffer.");
		return false;
	}

	/////////////////////////////////
	// Height pixel shader.
	////////////////////////////////

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(heightPsFilename.c_str(), NULL, NULL, "WaterHeightPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS || (1 << 0), 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, heightPsFilename.c_str());
		}
		else
		{
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + heightPsFilename + "'");
			MessageBox(hwnd, heightPsFilename.c_str(), "Missing shader file.", MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + heightPsFilename + "'");
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpHeightPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the water height pixel shader from the buffer.");
		return false;
	}

	/*
	* The polygonLayout.Format describes what size item should be placed in here, check if it's a float3 or float2 basically, and pass in DXGI_FORMAT_R32/G32/B32_FLOAT accordingly.
	*/

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

	// Get a count of the elements in the layout.
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mpLayout);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create polygon layout in Water Shader class.");
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
		logger->GetInstance().WriteLine("Failed to create the trilinear wrap sampler state in WaterShader.cpp");
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
		logger->GetInstance().WriteLine("Failed to create the bilinear mirror sampler state in WaterShader.cpp");
		return false;
	}

	////////////////////////////
	// Matrix buffer
	////////////////////////////

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
		logger->GetInstance().WriteLine("Failed to create the matrix buffer in WaterShader.cpp.");
		return false;
	}

	////////////////////////////
	// Water buffer
	////////////////////////////

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	waterBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterBufferDesc.MiscFlags = 0;
	waterBufferDesc.StructureByteStride = 0;


	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&waterBufferDesc, NULL, &mpWaterBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the water buffer in WaterShader.cpp.");
		return false;
	}

	////////////////////////////
	// Camera buffer
	////////////////////////////

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;


	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&cameraBufferDesc, NULL, &mpCameraBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the camera buffer in WaterShader.cpp.");
		return false;
	}

	////////////////////////////
	// Viewport buffer
	////////////////////////////

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
		logger->GetInstance().WriteLine("Failed to create the viewport buffer in WaterShader.cpp.");
		return false;
	}

	////////////////////////////
	// Light buffer
	////////////////////////////

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
		logger->GetInstance().WriteLine("Failed to create the light buffer in WaterShader.cpp.");
		return false;
	}

	return true;
}

void CWaterShader::ShutdownShader()
{
	if (mpTrilinearWrap)
	{
		mpTrilinearWrap->Release();
		mpTrilinearWrap = nullptr;
	}

	if (mpBilinearMirror)
	{
		mpBilinearMirror->Release();
		mpBilinearMirror = nullptr;
	}

	if (mpMatrixBuffer)
	{
		mpMatrixBuffer->Release();
		mpMatrixBuffer = nullptr;
	}

	if (mpWaterBuffer)
	{
		mpWaterBuffer->Release();
		mpWaterBuffer = nullptr;
	}

	if (mpCameraBuffer)
	{
		mpCameraBuffer->Release();
		mpCameraBuffer = nullptr;
	}

	if (mpViewportBuffer)
	{
		mpViewportBuffer->Release();
		mpViewportBuffer = nullptr;
	}

	if (mpLightBuffer)
	{
		mpLightBuffer->Release();
		mpLightBuffer = nullptr;
	}

	if (mpLayout)
	{
		mpLayout->Release();
		mpLayout = nullptr;
	}

	if (mpHeightPixelShader)
	{
		mpHeightPixelShader->Release();
		mpHeightPixelShader = nullptr;
	}

	if (mpSurfacePixelShader)
	{
		mpSurfacePixelShader->Release();
		mpSurfacePixelShader = nullptr;
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

bool CWaterShader::SetSurfaceShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
	D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion, float maxDistortion,
	float refractionStrength, float reflectionStrength, D3DXMATRIX cameraMatrix, D3DXVECTOR3 cameraPosition, D3DXVECTOR2 viewportSize, D3DXVECTOR4 ambientColour,
	D3DXVECTOR4 diffuseColour, D3DXVECTOR3 lightDirection, ID3D11ShaderResourceView* normalHeightMap, ID3D11ShaderResourceView* refractionMap, ID3D11ShaderResourceView* reflectionMap)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projMatrix, &projMatrix);

	///////////////////////////
	// Matrix buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType * matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);

	///////////////////////////
	// Water buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpWaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	WaterBufferType * waterBufferPtr = (WaterBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	waterBufferPtr->WaterSize = waterSize;
	waterBufferPtr->WaterSpeed = waterSpeed;
	waterBufferPtr->WaterTranslation = waterTranslation;
	waterBufferPtr->WaveHeight = waveHeight;
	waterBufferPtr->WaveScale = waveScale;
	waterBufferPtr->RefractionDistortion = refractionDistortion;
	waterBufferPtr->ReflectionDistortion = reflectionDistortion;
	waterBufferPtr->MaxDistortionDistance = maxDistortion;
	waterBufferPtr->RefractionStrength = refractionStrength;
	waterBufferPtr->ReflectionStrength = reflectionStrength;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpWaterBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 1;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpWaterBuffer);
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpWaterBuffer);

	///////////////////////////
	// Camera buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	CameraBufferType * cameraBufferPtr = (CameraBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	cameraBufferPtr->CameraMatrix = cameraMatrix;
	cameraBufferPtr->CameraPosition = cameraPosition;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpCameraBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 2;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpCameraBuffer);

	///////////////////////////
	// Viewport buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpViewportBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	ViewportBufferType * viewportBufferPtr = (ViewportBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	viewportBufferPtr->ViewportSize = viewportSize;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpViewportBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 3;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpViewportBuffer);

	///////////////////////////
	// Light buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	LightBufferType * lightBufferPtr = (LightBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	lightBufferPtr->AmbientColour = ambientColour;
	lightBufferPtr->DiffuseColour = diffuseColour;
	lightBufferPtr->LightDirection = lightDirection;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 4;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	///////////////////////////
	// Shader resources
	///////////////////////////
	deviceContext->VSSetShaderResources(0, 1, &normalHeightMap);
	deviceContext->PSSetShaderResources(0, 1, &normalHeightMap);
	deviceContext->PSSetShaderResources(1, 1, &refractionMap);
	deviceContext->PSSetShaderResources(2, 1, &reflectionMap);

	return true;
}

bool CWaterShader::SetHeightShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projMatrix,
	D3DXVECTOR4 waterSize, D3DXVECTOR4 waterSpeed, D3DXVECTOR2 waterTranslation, float waveHeight, float waveScale, float refractionDistortion, float reflectionDistortion,
	float maxDistortion, float refractionStrength, float reflectionStrength, ID3D11ShaderResourceView* normalHeightMap)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projMatrix, &projMatrix);

	///////////////////////////
	// Matrix buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType * matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = worldMatrix;
	matrixBufferPtr->view = viewMatrix;
	matrixBufferPtr->projection = projMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);

	///////////////////////////
	// Water buffer
	///////////////////////////

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(mpWaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	WaterBufferType * waterBufferPtr = (WaterBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	waterBufferPtr->WaterSize = waterSize;
	waterBufferPtr->WaterSpeed = waterSpeed;
	waterBufferPtr->WaterTranslation = waterTranslation;
	waterBufferPtr->WaveHeight = waveHeight;
	waterBufferPtr->WaveScale = waveScale;
	waterBufferPtr->RefractionDistortion = refractionDistortion;
	waterBufferPtr->ReflectionDistortion = reflectionDistortion;
	waterBufferPtr->MaxDistortionDistance = maxDistortion;
	waterBufferPtr->RefractionStrength = refractionStrength;
	waterBufferPtr->ReflectionStrength = reflectionStrength;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpWaterBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 1;

	// Now set the constant buffer in the vertex and pixel shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpWaterBuffer);

	///////////////////////////
	// Shader resources
	///////////////////////////
	deviceContext->VSSetShaderResources(0, 1, &normalHeightMap);

	return true;
}

void CWaterShader::RenderSurfaceShader(ID3D11DeviceContext * deviceContext, int indexCount)
{	
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpSurfacePixelShader, NULL, 0);

	// Set the sampler states
	deviceContext->VSSetSamplers(0, 1, &mpTrilinearWrap);
	deviceContext->PSSetSamplers(0, 1, &mpTrilinearWrap);
	deviceContext->PSSetSamplers(1, 1, &mpBilinearMirror);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

void CWaterShader::RenderHeightShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpHeightPixelShader, NULL, 0);

	// Set the sampler state in the vertex shader.
	deviceContext->VSSetSamplers(0, 1, &mpTrilinearWrap);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
