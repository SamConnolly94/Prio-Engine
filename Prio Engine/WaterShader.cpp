#include "WaterShader.h"


CWaterShader::CWaterShader()
{
	mpVertexShader = nullptr;
	mpPixelShader = nullptr;
	mpLayout = nullptr;
	mpMatrixBuffer = nullptr;
	mpTrilinearWrapSampleState = nullptr;
	mpWaveBuffer = nullptr;
	mpWaterBuffer = nullptr;
	mpCameraBuffer = nullptr;
	mpLightBuffer = nullptr;
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
	D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale,
	int screenWidth, int screenHeight, CCamera* camera, CLight* light)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, indexCount, worldMatrix, viewMatrix,
		projMatrix, waterMovement, strength, distortionDistance, waveScale,
		screenWidth, screenHeight, camera, light);

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
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC waveBufferDesc;
	D3D11_BUFFER_DESC waterPosBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC viewportBufferDesc;


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

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = 0;
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
	result = device->CreateSamplerState(&samplerDesc, &mpTrilinearWrapSampleState);

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
	// Wave buffer.
	////////////////////////

	waveBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waveBufferDesc.ByteWidth = sizeof(WaveBufferType);
	waveBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waveBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waveBufferDesc.MiscFlags = 0;
	waveBufferDesc.StructureByteStride = 0;

	// Create constant buffer.
	result = device->CreateBuffer(&waveBufferDesc, NULL, &mpWaveBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the wave buffer in water shader class.");
		return false;
	}

	///////////////////////
	// Water buffer.
	///////////////////////
	waterPosBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterPosBufferDesc.ByteWidth = sizeof(WaterBufferType);
	waterPosBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterPosBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterPosBufferDesc.MiscFlags = 0;
	waterPosBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&waterPosBufferDesc, NULL, &mpWaterBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the water position from provided description in water shader class.");
		return false;
	}

	/////////////////////////////////////////
	// Camera buffer.
	/////////////////////////////////////////

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&cameraBufferDesc, NULL, &mpCameraBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the camera buffer from provided description in water shader class.");
		return false;
	}

	//////////////////////////////////////
	// Light buffer
	//////////////////////////////////////
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightBufferDesc, NULL, &mpLightBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the light buffer from provided description in water shader class.");
		return false;
	}

	//////////////////////////////////
	// Viewport buffer desc.
	/////////////////////////////////
	viewportBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewportBufferDesc.ByteWidth = sizeof(ViewportBufferType);
	viewportBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewportBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewportBufferDesc.MiscFlags = 0;
	viewportBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&viewportBufferDesc, NULL, &mpViewportBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the viewport buffer from provided description in water shader class.");
		return false;
	}

	return true;
}

void CWaterShader::ShutdownShader()
{
	if (mpTrilinearWrapSampleState)
	{
		mpTrilinearWrapSampleState->Release();
		mpTrilinearWrapSampleState = nullptr;
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
	
	if (mpWaveBuffer)
	{
		mpWaveBuffer->Release();
		mpWaveBuffer = nullptr;
	}

	if (mpCameraBuffer)
	{
		mpCameraBuffer->Release();
		mpCameraBuffer = nullptr;
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
	D3DXMATRIX projMatrix, D3DXVECTOR2 waterMovement, D3DXVECTOR2 strength, float distortionDistance, float waveScale,
	int screenWidth, int screenHeight, CCamera* camera, CLight* light)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projMatrix, &projMatrix);
	
	////////////////////////////
	// Update the matrices constant buffer.
	///////////////////////////

	if (!UpdateMatrixBuffer(deviceContext, mappedResource, worldMatrix, viewMatrix, projMatrix))
	{
		logger->GetInstance().WriteLine("Failed to set the matrix constant buffer.");
		return false;
	}

	/////////////////////////////////
	// Update the wave constant buffer.
	/////////////////////////////////

	if (!UpdateWaveBuffer(deviceContext, mappedResource, 1.0f))
	{
		logger->GetInstance().WriteLine("Failed to set the wave constant buffer.");
		return false;
	}

	//////////////////////////////
	// Update the water constant buffer.
	//////////////////////////////

	if (!UpdateWaterBuffer(deviceContext, mappedResource, D3DXVECTOR2(1.0F, 1.0F), strength.x, strength.y, distortionDistance, 1.0f))
	{
		logger->GetInstance().WriteLine("Failed to set the water constant buffer.");
		return false;
	}

	//////////////////////////////
	// Camera buffer.
	/////////////////////////////
	D3DXMATRIX camWorld;
	camera->GetWorldMatrix(camWorld);
	if (!UpdateCameraBuffer(deviceContext, mappedResource, camWorld, camera->GetPosition()))
	{
		logger->GetInstance().WriteLine("Failed to set the camera constant buffer.");
		return false;
	}

	////////////////////////////
	// Light buffer
	////////////////////////////

	if (!UpdateLightBuffer(deviceContext, mappedResource, light))
	{
		logger->GetInstance().WriteLine("Failed to set the light constant buffer.");
		return false;
	}

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
	deviceContext->PSSetSamplers(0, 1, &mpTrilinearWrapSampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}

bool CWaterShader::UpdateMatrixBuffer(ID3D11DeviceContext * deviceContext, D3D11_MAPPED_SUBRESOURCE & mappedResource, D3DXMATRIX world, D3DXMATRIX view, D3DXMATRIX proj)
{
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(mpMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for matrices in WaterShader class.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType* matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	matrixBufferPtr->world = world;
	matrixBufferPtr->view = view;
	matrixBufferPtr->projection = proj;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpMatrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	int bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpMatrixBuffer);

	return true;
}

bool CWaterShader::UpdateWaveBuffer(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, float waveScale)
{
	HRESULT result = deviceContext->Map(mpWaveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the wave constant buffer in water shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	WaveBufferType* waveBufferPtr = (WaveBufferType*)mappedResource.pData;

	// Copy the current value of the reflection matrix into the constant buffer.
	waveBufferPtr->WaveScale = waveScale;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpWaveBuffer, 0);

	// Update the position of our constant buffer in the shader.
	int bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpWaveBuffer);

	return true;
}

bool CWaterShader::UpdateWaterBuffer(ID3D11DeviceContext* deviceContext,
	D3D11_MAPPED_SUBRESOURCE& mappedResource, D3DXVECTOR2 WaterMovement, float refractionStrength, 
	float reflectionStrength, float maxDistortionDistance, float WaveScale)
{
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(mpWaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for water position in WaterShader class.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	WaterBufferType* waterBufferPtr = (WaterBufferType*)mappedResource.pData;

	// Copy the water values into the constant buffer.
	waterBufferPtr->WaterMovement = WaterMovement;
	waterBufferPtr->WaveScale = WaveScale;
	waterBufferPtr->RefractionStrength = refractionStrength;
	waterBufferPtr->ReflectionStrength = reflectionStrength;
	waterBufferPtr->MaxDistortionDistance = maxDistortionDistance;
	waterBufferPtr->waterBufferPadding = D3DXVECTOR2(0.0f, 0.0f);

	// Unlock the constant buffer.
	deviceContext->Unmap(mpWaterBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	int bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpWaterBuffer);

	return true;
}

bool CWaterShader::UpdateCameraBuffer(ID3D11DeviceContext * deviceContext, D3D11_MAPPED_SUBRESOURCE & mappedResource, D3DXMATRIX CameraWorldMatrix, D3DXVECTOR3 CameraPosition)
{
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(mpCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for water position in WaterShader class.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	CameraBufferType* cameraBufferPtr = (CameraBufferType*)mappedResource.pData;

	// Copy the water values into the constant buffer.
	cameraBufferPtr->CameraMatrix = CameraWorldMatrix;
	cameraBufferPtr->CameraPos = CameraPosition;
	cameraBufferPtr->cameraBufferPadding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpCameraBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	int bufferNumber = 1;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpCameraBuffer);

	return true;
}

bool CWaterShader::UpdateLightBuffer(ID3D11DeviceContext * deviceContext, D3D11_MAPPED_SUBRESOURCE & mappedResource, CLight* light)
{	
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(mpLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for light in WaterShader class.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	LightBufferType* lightBufferPtr = (LightBufferType*)mappedResource.pData;

	// Copy the water values into the constant buffer.
	lightBufferPtr->LightPosition = light->GetPos();
	lightBufferPtr->AmbientColour = light->GetAmbientColour();
	lightBufferPtr->DiffuseColour = light->GetDiffuseColour();
	lightBufferPtr->LightDirection = light->GetDirection();
	lightBufferPtr->SpecularColor = light->GetSpecularColour();
	lightBufferPtr->SpecularPower = light->GetSpecularPower();
	lightBufferPtr->lightBufferPadding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	int bufferNumber = 2;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	return true;
}

bool CWaterShader::UpdateViewportBuffer(ID3D11DeviceContext * deviceContext, D3D11_MAPPED_SUBRESOURCE & mappedResource, int screenWidth, int screenHeight)
{
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(mpViewportBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to lock the constant buffer for light in WaterShader class.");
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	ViewportBufferType* viewportBufferPtr = (ViewportBufferType*)mappedResource.pData;

	// Copy the water values into the constant buffer.
	viewportBufferPtr->ViewportSize = D3DXVECTOR2(screenWidth, screenHeight);

	// Unlock the constant buffer.
	deviceContext->Unmap(mpViewportBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	int bufferNumber = 3;

	// Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpViewportBuffer);

	return true;
}

void CWaterShader::SetHeightMap(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * resource)
{
	deviceContext->VSSetShaderResources(0, 1, &resource);
}

void CWaterShader::SetRefractionMap(ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * resource)
{
	deviceContext->PSSetShaderResources(0, 1, &resource);
}
