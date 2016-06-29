#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "InitDirect3DApp.h"
#include <WindowsX.h>
#include "Logger.h"

/*
 This is the game engine itself, only one of these may be defined at any one time.
*/
class CEngine
{
private:
	int InitialiseD3DApp(HINSTANCE hInstance);
	int mD3DInitCode;
	HINSTANCE mhInstance;
	CLogger* mLogger;
public:
	// Run the Direct X Application
	//int RunGameLoop();

/* Signleton class methods. */
public:
	static CEngine& GetEngine(HINSTANCE hInstance);
private:
	// Constructor.
	CEngine(HINSTANCE hInstance);
public:
	// Delete the object prevent memory leaks.
	CEngine(CEngine const&) = delete;
	void operator = (CEngine const&) = delete;
};

#endif