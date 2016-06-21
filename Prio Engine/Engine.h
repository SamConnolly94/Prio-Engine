#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "Logger.h"
#include "InitDirect3DApp.h"
#include <WindowsX.h>

/*
 This is the game engine itself, only one of these may be defined at any one time.
*/
class CEngine
{
private:
	CLogger* mLogger;
	void InitialiseLogger();
	int InitialiseD3DApp(HINSTANCE hInstance);
	int mD3DInitCode;
	CInitDirect3DApp mD3DApp;
public:
	//Constructor.
	CEngine(HINSTANCE hInstance);
	// Destructor.
	~CEngine();

	// Run the Direct X Application
	int RunGameLoop();

};

#endif