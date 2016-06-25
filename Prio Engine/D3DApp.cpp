#include "D3DApp.h"
#include <WindowsX.h>

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Grab the handle on the window to get messages.
	return D3DApp::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

/* Returns a pointer to our Direct3D Application. All pointers are static. */
D3DApp* D3DApp::mApp = nullptr;
D3DApp* D3DApp::GetApp()
{
	return mApp;
}

D3DApp::D3DApp(HINSTANCE hInstance) : mhAppInst(hInstance)
{
	mTimer = new CGameTimer();
	mAppPaused = false;

	// Only one D3DApp can be constructed.
	assert(mApp == nullptr);
	mApp = this;
}

D3DApp::~D3DApp()
{
	delete (mTimer);
}

/* Controls messages from the OS about our window. */
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Control what behaviour should occur depending on what message we recieve from the OS.
	switch (msg)
	{
	// Controls what happens when our window is activated or deactivated.
	case WM_ACTIVATE:
		// If window is inactive (loses focus).
		if (LOWORD(wParam) == WA_INACTIVE) 
		{
			// Pause the game as we lost focus.
			mAppPaused = true;
			mTimer->Stop();
		} 
		else
		{
			// Resume the game, regained focus.
			mAppPaused = false;
			mTimer->Start();
		}
		return 0;
	// Control what happens when the resize event of the window is triggered.
	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);

		// If our Direct3D device has been initialised.
		if ( md3dDevice )
		{
			// If the window is minimised.
			if ( wParam == SIZE_MINIMIZED )
			{
				// Pause the app, don't bother resetting timer, our active event will handle that.
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if ( wParam == SIZE_MAXIMIZED )
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnRisize();
			}
		}
	}
}

bool D3DApp::Initialise()
{
	if (!InitMainWindow())
	{
		return false;
	}

	// Initialise Direct3D, or tell the caller that we failed to init.
	if (!InitDirect3D())
	{
		return false;
	}

	return true;
}

/* Set up the main window which we will use for our application. */
bool D3DApp::InitMainWindow()
{
	// Window class structure which stores all window class attributes.
	WNDCLASS wc;
	
	// Set the attributes for our window class.
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;

	return true;
}

/* Sets up DirectX variables ready for use within our window. */
bool D3DApp::InitDirect3D()
{
// Check for debug mode.
#if defined (debug) || defined(_DEBUG)
	{
		// Enable the debug layer for DirectX 12
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	// Create Direct X factory.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create the hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		// Null defaults to default display device (graphics card).
		nullptr,
		// Minimum feature levels set to DirectX 11.0
		D3D_FEATURE_LEVEL_11_0,
		// Pass in the direct3d device.
		IID_PPV_ARGS(&md3dDevice)
	);

	// If we failed to initialise using a hardware device, default to a software version named WARP.
	if (FAILED(hardwareResult))
	{
		// Define a pointer to the warm adapter.
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		
		// Throw an error if we can't init the WARP (Windows Advanced Rasterization Platform) adapter.
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		// Throw an error if we fail to get the WARP adapter with a minimum version of DirectX 11.
		ThrowIfFailed(D3D12CreateDevice(
			// Grab the WARP adapter and use it as our display device.
			pWarpAdapter.Get(),
			// Define a minimum feature level of DirectX 11.0.
			D3D_FEATURE_LEVEL_11_0,
			// Initialise our direct3d device to use our WARP device.
			IID_PPV_ARGS(&md3dDevice)
		));
	}

	// Create the fence for GPU / CPU synchronization.
	ThrowIfFailed(md3dDevice->CreateFence(
		// Defines our initial value (0 if we're starting from scratch).
		0,
		// Pass in any fence flags.
		D3D12_FENCE_FLAG_NONE,
		// Pass in the pointer to the fence.
		IID_PPV_ARGS(&mFence)
	));

	// Query the information of descriptors.
	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	// Check 4X MSAA (Multi Sampling Anti Aliasing) support for back buffer.
	// All Direct3D 11 devices should support 4X MSAA for all render target formats,
	// this means we only need to check one of them for support.
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));

	// Check our m4xMsaa is an expected number. 4x MSAA should always be supported, therefor quality levels should always be more than 0.
	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	// Completed setup successfully, return true.
	return true;
}

/* Set up the command queue so that GPU and CPU can be in sync. */
void D3DApp::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	
	ThrowIfFailed(md3dDevice->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(&mCommandQueue)
	));

	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
	D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())
	));

	ThrowIfFailed(md3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		// The command allocator assigned.
		mDirectCmdListAlloc.Get(),
		// Initial PipelineStateObject
		nullptr,
		IID_PPV_ARGS(mCommandList.GetAddressOf())
		));

	// Start the command list in a closed state, so when we first grab it we must reset it. It is required to be closed in order to be reset.
	mCommandList->Close();
}

/* Swap chain swaps the front buffer with a prebuffered back buffer which has already been rendered, then renders the new back buffer. */
void D3DApp::CreateSwapChain()
{
	// Release the previous swapchain, we're going to recreate it.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	// Buffer desc describes the back buffer we want to create.
	sd.BufferDesc.Width = mClientHeight;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// Sample desc is the number of multi samples and quality levels.
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	// Buffer usage specifies the target output.
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Buffer count describes the number of buffers to use in the swap chain. 2 = double buffering, 3 = tripple buffering etc...
	sd.BufferCount = SwapChainBufferCount;
	// Output window is a handle to the window we will output to.
	sd.OutputWindow = mhMainWnd;
	// Describes if the application is windows or fullscreen.
	sd.Windowed = true;
	// Describes what happens when the swap chain swaps buffers.
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// Optional flags, allow switching to full screen to choose a display mode which matches best dimensions of display.
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()
	));
}

/* Create descriptor heaps to store the descriptors / views our application requires. */
void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	// One descriptor for each buffer we have.
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void D3DApp::FlushCommandQueue()
{
	// Increment our fence point.
	mCurrentFence++;

	// Adds an instruction to our queue of commands which sets a new fence point.
	// We'll wait to catch up with ourselves on the fence point before this instruction occurs.
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits fence.
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		// Wait until GPU hits the fence we're currently on. It'll fire an event.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

/* Control what happens when the window is resized. */
void D3DApp::OnResize()
{
	// Check our bare essentials have been correctly initialised.
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	// Clear anything that our GPU still needs to do.
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Release all resources, we'll recreate them.
	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		mSwapChainBuffer[i].Reset();
	}
	mDepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mClientWidth, mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	));

	mCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT swapCounter = 0; swapCounter < SwapChainBufferCount; swapCounter++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(swapCounter, IID_PPV_ARGS(&mSwapChainBuffer[swapCounter])));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[swapCounter].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}
}

/* Run the Direct3D application. */
int D3DApp::Run()
{
	MSG msg = { 0 };

	mTimer->Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Process a tick on the timer.
			mTimer->Tick();

			// Make sure the game isn't paused.
			if (!mAppPaused) 
			{
				
			}
		}
	}

	return (int)msg.wParam;
}
