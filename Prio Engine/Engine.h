#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "Logger.h"
#include <WindowsX.h>

/*
 This is the game engine itself, only one of these may be defined at any one time.
*/
class CEngine
{
private:
	//Constructor definition.
	CEngine();
	CEngine(CEngine const&) = delete;
	void operator=(CEngine const&) = delete;

	CLogger* mLogger;
	bool OpenMainWindow();
	void Initialise();
public:
	static CEngine& getInstance()
	{
		static CEngine instance;
		
		return instance;
	}
	void Cleanup();
};

#endif