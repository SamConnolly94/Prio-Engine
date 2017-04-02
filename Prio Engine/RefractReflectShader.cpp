#include "RefractReflectShader.h"



CReflectRefractShader::CReflectRefractShader()
{
	mpVertexShader					= nullptr;
	mpRefractionPixelShader			= nullptr;
	mpLayout						= nullptr;
	mpTrilinearWrap					= nullptr;
	mpLightBuffer					= nullptr;
	mpViewportBuffer				= nullptr;
	mpReflectionPixelShader			= nullptr;
	mpTerrainAreaBuffer				= nullptr;
	mpPositioningBuffer				= nullptr;
	mpFoliageVertexShader			= nullptr;
	mpFoliageRefractionPixelShader  = nullptr;
}


CReflectRefractShader::~CReflectRefractShader()
{
}

bool CReflectRefractShader::Initialise(ID3D11Device * device, HWND hwnd)
{
	bool result;

	// Initialise the vertex pixel shaders.
	std::string TerrainVSName = "Shaders/RefractedModel.vs.hlsl";
	std::string TerrainPSRefractName = "Shaders/TerrainRefraction.ps.hlsl";
	std::string TerrainPSReflectName = "Shaders/TerrainReflection.ps.hlsl";
	std::string SkyboxVSName = "Shaders/SkyboxRefraction.vs.hlsl";
	std::string SkyboxPSName = "Shaders/SkyboxRefraction.ps.hlsl";
	std::string ModelReflectionPSName = "Shaders/ModelReflection.ps.hlsl";
	std::string FoliageVSName = "Shaders/FoliageRefraction.vs.hlsl";
	std::string FoliagePSName = "Shaders/FoliageRefraction.ps.hlsl";

	result = InitialiseShader(device, hwnd, TerrainVSName, TerrainPSRefractName, TerrainPSReflectName, ModelReflectionPSName, FoliageVSName, FoliagePSName);

	if (!result)
	{
		logger->GetInstance().WriteLine("Failed to initialise refraction shader.");
		return false;
	}

	return true;
}

void CReflectRefractShader::Shutdown()
{
	// Shutodwn the vertex and pixel shaders as well as all related objects.
	ShutdownShader();
}

bool CReflectRefractShader::RefractionRender(ID3D11DeviceContext* deviceContext, int indexCount)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext);

	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderRefractionShader(deviceContext, indexCount);

	return true;
}

bool CReflectRefractShader::ReflectionRender(ID3D11DeviceContext* deviceContext, int indexCount)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext);

	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderReflectionShader(deviceContext, indexCount);

	return true;
}

bool CReflectRefractShader::RenderFoliageRefraction(ID3D11DeviceContext * deviceContext, int indexCount)
{
	bool result;

	// Set the shader parameters that it will use for rendering.
	result = SetFoliageShaderParameters(deviceContext);

	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderFoliageRefractionShader(deviceContext, indexCount);

	return true;
}

bool CReflectRefractShader::InitialiseShader(ID3D11Device * device, HWND hwnd, std::string vsFilename, std::string psFilename, std::string reflectionPSFilename, std::string modelReflectionPSName, std::string FoliageVSName, std::string FoliagePSName)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	//ID3D10Blob* skyboxVertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC viewportBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC terrainAreaBufferDesc;
	D3D11_BUFFER_DESC terrainPosBufferDesc;


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

	//////////////////////////////////
	// Terrain Reflection
	//////////////////////////////////

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

	//////////////////////////////////
	// Model Reflection
	//////////////////////////////////

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(modelReflectionPSName.c_str(), NULL, NULL, "ModelReflectionPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, modelReflectionPSName);
		}
		else
		{
			std::string errMsg = "Missing shader file.";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + modelReflectionPSName + "'");
			MessageBox(hwnd, modelReflectionPSName.c_str(), errMsg.c_str(), MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + modelReflectionPSName + "'");
		return false;
	}
	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpModelReflectionPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the reflection pixel shader from the buffer.");
		return false;
	}

	///////////////////////////////////
	// Foliage refraction
	///////////////////////////////////

	// Compile the vertex shader code.
	result = D3DX11CompileFromFile(FoliageVSName.c_str(), NULL, NULL, "FoliageRefractionVS", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, FoliageVSName);
		}
		else
		{
			std::string errMsg = "Missing shader file. ";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + FoliageVSName + "'");
			MessageBox(hwnd, FoliageVSName.c_str(), errMsg.c_str(), MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the vertex shader named '" + FoliageVSName + "'");
		return false;
	}

	// Compile the pixel shader code.
	result = D3DX11CompileFromFile(FoliagePSName.c_str(), NULL, NULL, "FoliageRefractionPS", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, FoliagePSName);
		}
		else
		{
			std::string errMsg = "Missing shader file.";
			logger->GetInstance().WriteLine("Could not find a shader file with name '" + FoliagePSName + "'");
			MessageBox(hwnd, FoliagePSName.c_str(), errMsg.c_str(), MB_OK);
		}
		logger->GetInstance().WriteLine("Failed to compile the pixel shader named '" + FoliagePSName + "'");
		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mpFoliageVertexShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage refraction vertex shader from the buffer.");
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mpFoliageRefractionPixelShader);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the foliage refraction pixel shader from the buffer.");
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
	

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &mpPointClamp);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the point clamp sampler state in RefractionShader.cpp");
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

	if (!SetupMatrixBuffer(device))
	{
		logger->GetInstance().WriteLine("Failed to set up matrix buffer in reflect refract shader class.");
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
		logger->GetInstance().WriteLine("Failed to create the viewport buffer from within the refraction shader class.");
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
		logger->GetInstance().WriteLine("Failed to create the light buffer from within the refraction shader class.");
		return false;
	}

	//////////////////////////////////
	// Set up terrain area buffer
	/////////////////////////////////

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	terrainAreaBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainAreaBufferDesc.ByteWidth = sizeof(TerrainAreaBufferType);
	terrainAreaBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainAreaBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainAreaBufferDesc.MiscFlags = 0;
	terrainAreaBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&terrainAreaBufferDesc, NULL, &mpTerrainAreaBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the terrain area buffer from within the refraction shader class.");
		return false;
	}

	//////////////////////////////////
	// Set up terrain positioning buffer
	/////////////////////////////////

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	terrainPosBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainPosBufferDesc.ByteWidth = sizeof(PositioningBufferType);
	terrainPosBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainPosBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainPosBufferDesc.MiscFlags = 0;
	terrainPosBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&terrainPosBufferDesc, NULL, &mpPositioningBuffer);
	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to create the terrain positioning buffer from within the refraction shader class.");
		return false;
	}

	//////////////////////////////////
	// Set up foliage buffer
	/////////////////////////////////

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC foliageBufferDesc;
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
		logger->GetInstance().WriteLine("Failed to create the foliage buffer from within the refraction shader class.");
		return false;
	}

	return true;
}

void CReflectRefractShader::ShutdownShader()
{
	if (mpTrilinearWrap)
	{
		mpTrilinearWrap->Release();
		mpTrilinearWrap = nullptr;
	}

	if (mpPointClamp)
	{
		mpPointClamp->Release();
		mpPointClamp = nullptr;
	}

	if (mpFoliageBuffer)
	{
		mpFoliageBuffer->Release();
		mpFoliageBuffer = nullptr;
	}

	if (mpTerrainAreaBuffer)
	{
		mpTerrainAreaBuffer->Release();
		mpTerrainAreaBuffer = nullptr;
	}

	if (mpPositioningBuffer)
	{
		mpPositioningBuffer->Release();
		mpPositioningBuffer = nullptr;
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

	if (mpReflectionPixelShader)
	{
		mpReflectionPixelShader->Release();
		mpReflectionPixelShader = nullptr;
	}

	if (mpVertexShader)
	{
		mpVertexShader->Release();
		mpVertexShader = nullptr;
	}
}

void CReflectRefractShader::OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd, std::string shaderFilename)
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

bool CReflectRefractShader::SetShaderParameters(ID3D11DeviceContext* deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* matrixBufferPtr;
	LightBufferType* lightBufferPtr;
	ViewportBufferType* viewportBufferPtr;

	////////////////////////////
	// Update the matrices constant buffer.
	///////////////////////////

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	if (!SetMatrixBuffer(deviceContext, bufferNumber, ShaderType::Vertex))
	{
		logger->GetInstance().WriteLine("Failed to set the matrix buffer in reflect refract shader.");
		return false;
	}

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
	viewportBufferPtr->ViewportSize = mViewportSize;

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
	lightBufferPtr->AmbientColour = mAmbientColour;
	lightBufferPtr->DiffuseColour = mDiffuseColour;
	lightBufferPtr->LightDirection = mLightDirection;
	lightBufferPtr->mLightPosition = mLightPosition;
	lightBufferPtr->mSpecularColour = mSpecularColour;
	lightBufferPtr->mSpecularPower = mSpecularPower;
	lightBufferPtr->lightBufferPadding = 0.0f;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	/////////////////////////////////
	// Update the terrain area constant buffer.
	/////////////////////////////////
	result = deviceContext->Map(mpTerrainAreaBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the terrain area constant buffer in refraction shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	TerrainAreaBufferType* terrainAreaBufferPtr= (TerrainAreaBufferType*)mappedResource.pData;

	// Copy the current light values into the constant buffer.
	terrainAreaBufferPtr->snowHeight = mSnowHeight + mTerrainYOffset;
	terrainAreaBufferPtr->grassHeight = mGrassHeight + mTerrainYOffset;
	terrainAreaBufferPtr->dirtHeight = mDirtHeight + mTerrainYOffset;
	terrainAreaBufferPtr->sandHeight = mSandHeight + mTerrainYOffset;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpTerrainAreaBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 2;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpTerrainAreaBuffer);


	/////////////////////////////////
	// Update the terrain positioning constant buffer.
	/////////////////////////////////
	result = deviceContext->Map(mpPositioningBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the terrain positioning constant buffer in refraction shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	PositioningBufferType* terrainPosBufferPtr = (PositioningBufferType*)mappedResource.pData;

	// Copy the current light values into the constant buffer.
	terrainPosBufferPtr->yOffset = mTerrainYOffset;
	terrainPosBufferPtr->WaterPlaneY = mWaterPlaneYOffset;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpPositioningBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 3;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpPositioningBuffer);

	/////////////////////////////////
	// Update shader resources
	/////////////////////////////////
	deviceContext->PSSetShaderResources(0, 1, &mpWaterHeightMap);
	deviceContext->PSSetShaderResources(1, 2, mpDirtTexArray);
	deviceContext->PSSetShaderResources(3, 2, mpGrassTextures);
	deviceContext->PSSetShaderResources(5, 1, &mpPatchMap);
	deviceContext->PSSetShaderResources(6, 2, mpRockTextures);

	return true;
}
bool CReflectRefractShader::SetFoliageShaderParameters(ID3D11DeviceContext * deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;

	if (!SetMatrixBuffer(deviceContext, 0, CShader::ShaderType::Vertex))
	{
		logger->GetInstance().WriteLine("Failed to set the matrix buffer when setting foliage shader parameters in refraction shader.");
		return false;
	}

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
	LightBufferType* lightBufferPtr = (LightBufferType*)mappedResource.pData;

	// Copy the current light values into the constant buffer.
	lightBufferPtr->AmbientColour = mAmbientColour;
	lightBufferPtr->DiffuseColour = mDiffuseColour;
	lightBufferPtr->LightDirection = mLightDirection;
	lightBufferPtr->mLightPosition = mLightPosition;
	lightBufferPtr->mSpecularColour = mSpecularColour;
	lightBufferPtr->mSpecularPower = mSpecularPower;
	lightBufferPtr->lightBufferPadding = 0.0f;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpLightBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &mpLightBuffer);

	/////////////////////////////////
	// Update the foliage buffer
	/////////////////////////////////
	result = deviceContext->Map(mpFoliageBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	if (FAILED(result))
	{
		logger->GetInstance().WriteLine("Failed to set the foliage constant buffer in refraction shader class.");
		return false;
	}

	// Get pointer to the data inside the constant buffer.
	FoliageBufferType* foliageBufferPtr = (FoliageBufferType*)mappedResource.pData;

	// Set values.
	foliageBufferPtr->FrameTime = mFrameTime;
	foliageBufferPtr->WindDirection = mWindDirection;
	foliageBufferPtr->WindStrength = mStrength;
	foliageBufferPtr->FoliageTranslation = mTranslation;

	// Unlock the constnat buffer so we can write to it elsewhere.
	deviceContext->Unmap(mpFoliageBuffer, 0);

	// Update the position of our constant buffer in the shader.
	bufferNumber = 1;

	// Set the constant buffer in the shader.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &mpFoliageBuffer);

	/////////////////////////////////////
	// Resources
	////////////////////////////////////

	deviceContext->PSSetShaderResources(0, 1, &mpWaterHeightMap);
	deviceContext->PSSetShaderResources(1, 1, &mpGrassTexture);
	deviceContext->PSSetShaderResources(2, 1, &mpGrassAlphaTexture);

	return true;
}

void CReflectRefractShader::RenderReflectionShader(ID3D11DeviceContext * deviceContext, int indexCount)
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

void CReflectRefractShader::RenderFoliageRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(mpLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(mpFoliageVertexShader, NULL, 0);
	deviceContext->PSSetShader(mpFoliageRefractionPixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &mpTrilinearWrap);
	deviceContext->PSSetSamplers(1, 1, &mpBilinearMirror);
	deviceContext->PSSetSamplers(2, 1, &mpPointClamp);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}

void CReflectRefractShader::RenderRefractionShader(ID3D11DeviceContext * deviceContext, int indexCount)
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

	// Unbind the resources we used.
	ID3D11ShaderResourceView* nullResource = nullptr;
	for (int i = 0; i < 7; i++)
	{
		deviceContext->PSSetShaderResources(i, 1, &nullResource);
	}

	return;
}

void CReflectRefractShader::SetLightProperties(CLight * light)
{
	mAmbientColour = light->GetAmbientColour();
	mDiffuseColour = light->GetDiffuseColour();
	mLightDirection = light->GetDirection();
	mSpecularPower = light->GetSpecularPower();
	mSpecularColour = light->GetSpecularColour();
	mLightPosition = light->GetPos();
}

void CReflectRefractShader::SetViewportProperties(int screenWidth, int screenHeight)
{
	mViewportSize = D3DXVECTOR2(static_cast<float>(screenWidth), static_cast<float>(screenHeight));
}

void CReflectRefractShader::SetTerrainAreaProperties(float snowHeight, float grassHeight, float dirtHeight, float sandHeight)
{
	mSnowHeight = snowHeight;
	mGrassHeight = grassHeight;
	mDirtHeight = dirtHeight;
	mSandHeight = sandHeight;
}

void CReflectRefractShader::SetPositioningProperties(float terrainPositionY, float waterPlanePositionY)
{
	mTerrainYOffset = terrainPositionY;
	mWaterPlaneYOffset = waterPlanePositionY;
}

void CReflectRefractShader::SetWaterHeightmap(ID3D11ShaderResourceView * waterHeightMap)
{
	mpWaterHeightMap = waterHeightMap;
}

void CReflectRefractShader::SetDirtTextureArray(CTexture ** dirtTexArray)
{
	mpDirtTexArray[0] = dirtTexArray[0]->GetTexture();
	mpDirtTexArray[1] = dirtTexArray[1]->GetTexture();
}

void CReflectRefractShader::SetGrassTextureArray(CTexture ** grassTexArray)
{
	mpGrassTextures[0] = grassTexArray[0]->GetTexture();
	mpGrassTextures[1] = grassTexArray[1]->GetTexture();
}

void CReflectRefractShader::SetPatchMap(CTexture * patchMap)
{
	mpPatchMap = patchMap->GetTexture();
}

void CReflectRefractShader::SetRockTexture(CTexture ** rockTexArray)
{
	mpRockTextures[0] = rockTexArray[0]->GetTexture();
	mpRockTextures[1] = rockTexArray[1]->GetTexture();
}

void CReflectRefractShader::SetGrassTexture(ID3D11ShaderResourceView * grassTexture)
{
	mpGrassTexture = grassTexture;
}

void CReflectRefractShader::SetGrassAlphaTexture(ID3D11ShaderResourceView * alphaTexture)
{
	mpGrassAlphaTexture = alphaTexture;
}

void CReflectRefractShader::SetWindDirection(D3DXVECTOR3 direction)
{
	mWindDirection;
}

void CReflectRefractShader::SetFrameTime(float frameTime)
{
	mFrameTime = frameTime;
}

void CReflectRefractShader::SetWindStrength(float strength)
{
	mStrength = strength;
}

void CReflectRefractShader::SetTranslation(D3DXVECTOR3 translation)
{
	mTranslation = translation;
}
