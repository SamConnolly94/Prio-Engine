#pragma once
#ifndef D3DAPP_H
#define D3DAPP_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "D3DUtil.h"
#include "GameTimer.h"
#include "Logger.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
//D3DSetup
protected:
	
	D3DApp(HINSTANCE hInstance);
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp&rhs) = delete;
	virtual ~D3DApp();

// Game properties.
private:
	CGameTimer* mTimer;
	bool mAppPaused;	

public:
	int Run();

	void Set4xMsaaState(bool value);

// Initialisation methods.
protected:
	bool InitMainWindow();
	bool InitDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
public:
	static D3DApp* GetApp();
	virtual bool Initialise();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Direct3D properties
protected:
	static D3DApp* mApp;

	// Set true to use 4X MSAA (§4.1.8).  The default is false.
	bool      m4xMsaaState = false;    // 4X MSAA enabled
	UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

	Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	// Handle for the instance of the application.
	HINSTANCE mhAppInst = nullptr;
	HWND      mhMainWnd = nullptr; // main window handle

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption = L"PrioEngine";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

// Window properties.
private:
	int mClientWidth = 800;
	int mClientHeight = 600;
	bool mMinimized = false;
	bool mMaximized = false;
	bool mResizing = false;   // are the resize bars being dragged?
protected:
	virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
private:
	CLogger* mLogger;
};

#endif