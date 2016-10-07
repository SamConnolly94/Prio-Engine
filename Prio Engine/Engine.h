#pragma once

#ifndef ENGINE_H
#define ENGINE_H

#include "PrioTypes.h"
#include "GameTimer.h"
#include "Graphics.h"
#include "Input.h"
#include <windows.h>

class CEngine
{
private:
	// Logger which writes to a default log file. Can be accessed through CLogger->GetInstance()->Write();
	CLogger* mpLogger;
public:
	// Default constructor.
	CEngine();
	// Default destructor.
	~CEngine();

	// Attempts to instialse Direct X within the window.
	bool Initialise();
	// Handle what happens when the window is closed and engine is told to stop.
	void Shutdown();
	// Runs the engine.
	bool IsRunning();
	void StartTimer();

	// Control what happens when we recieve different messages from the operating system.
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
private:
	bool Frame();
	void InitialiseWindows(int&, int&);
	void ShutdownWindows();
	float mFrameTime;
private:
	// The name of our application as it will appear in windows.
	LPCWSTR mApplicationName;
	// The instance of our application as reffered to by windows.
	HINSTANCE mHinstance;
	// The handle to the window of our application.
	HWND mHwnd;

	CInput* mpInput;
	CGraphics* mpGraphics;

	void CheckWindowsMessages(MSG &msg);
	bool ProcessWindowsMessages();

	CGameTimer* mTimer;
private:
	MSG mMsg;
	bool result;
	bool mIsRunning;
public:
	// Model creation functions.
	CModel* CreateModel(PrioEngine::RGBA colour, PrioEngine::Primitives shape);
	CModel* CreateModel(WCHAR* textureFilename, bool useLighting, PrioEngine::Primitives shape);
	CModel* CreateModel(WCHAR* textureFilename, PrioEngine::Primitives shape);

	float GetFrameTime();
	CCamera* CreateCamera();
};

// Define WndProc and the application handle pointer here so that we can re-direct the windows system messaging into our message handler 
// function inside the system class.
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static CEngine* ApplicationHandle = 0;

#endif