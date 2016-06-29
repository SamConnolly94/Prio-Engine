#include "Engine.h"

CEngine::CEngine(HINSTANCE hInstance)
{	
	mhInstance = hInstance;
	// Initialise the Direct X Application and store the response code.
	mD3DInitCode = InitialiseD3DApp(hInstance);
	// Check that the response code is what we expected.
	if (mD3DInitCode != 0)
	{
		std::string errorMsg = "";
		switch (mD3DInitCode)
		{
		case 12:
			errorMsg = "Initialisation of direct 3D Application failed, called by CEngine Constructor. ";
			break;
		}

		// Write the error code to the logs.
		mLogger->GetLogger().WriteLine("Initialisation of the D3DApp failed, response code was: " + std::to_string(mD3DInitCode) + " '" + errorMsg + "'");

		// Show a warning that the D3DApp was not initialised correctly.
		MessageBox(0, L"Could not initialise the D3DApp, check the logs for response code.", L"Error initialising D3DApp!", MB_OK);
	}
	mLogger->GetLogger().WriteLine("Engine constructor completed.");
}

CEngine & CEngine::GetEngine(HINSTANCE hInstance)
{
	static CEngine instance(hInstance);
	return instance;
}

int CEngine::InitialiseD3DApp(HINSTANCE hInstance)
{
	// Create an instance of a Direct 3D application.
	CInitDirect3DApp theApp(hInstance);

	// If we fail while initialising, return a response code of 12 for fatal.
	if (!theApp.Initialise()) 
	{
		return 12;
	}

	return theApp.Run();
}