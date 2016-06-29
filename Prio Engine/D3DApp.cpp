#include "D3DApp.h"
#include <WindowsX.h>

using namespace std;
using namespace DirectX;

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

D3DApp::D3DApp(HINSTANCE hInstance)
{
	mTimer = new CGameTimer();
	mAppPaused = false;
	mLogger->GetLogger();
	mhAppInst = hInstance;

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
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer->Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer->Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer->Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer->Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::Initialise()
{
	if (!InitMainWindow())
	{
		return false;
	}

	mLogger->GetLogger().WriteLine("Successfully initialised the main window. ");

	// Initialise Direct3D, or tell the caller that we failed to init.
	if (!InitDirect3D())
	{
		return false;
	}

	mLogger->GetLogger().WriteLine("Successfully initialised Direct3D.");

	return true;
}

/* Set up the main window which we will use for our application. */
bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"PrioEngine";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"PrioEngine", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		// Output a detailed error to the debug logs.
		mLogger->GetLogger().WriteLine("Failed to create window, last error was:");
		std::ostringstream errorStream;
		errorStream << GetLastError();
		std::string errorString = errorStream.str();
		mLogger->GetLogger().WriteLine(errorString);

		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

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
	sd.BufferDesc.Width = mClientWidth;
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

ID3D12Resource* D3DApp::CurrentBackBuffer()const
{
	return mSwapChainBuffer[mCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3DApp::DepthStencilView()const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
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
		// Get the buffer at position swapCounter in the swap chain.
		ThrowIfFailed(mSwapChain->GetBuffer(swapCounter, IID_PPV_ARGS(&mSwapChainBuffer[swapCounter])));

		// Create a render target view to the buffer we just retrieved.
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[swapCounter].Get(), nullptr, rtvHeapHandle);

		// Point to the next entry on the heap.
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	// Create depth and stencil buffer.
	// Define a descriptor for the depth & stencil buffer.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	// Set the depth stencil buff to the size of the window
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
	));

	// Create a descriptor of mip level 0 of entire resource using the format of the resource.
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, DepthStencilView());

	// Transition the resource from the initial state to be used as a depth buff.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(), 
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands on the window.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);

	// Waits for the resize to complete.
	FlushCommandQueue();

	// Update the viewport transform to cover client area.
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	mCommandList->RSSetViewports(1, &mScreenViewport);

	mScissorRect = { 0, 0, mClientWidth, mClientHeight };
	mCommandList->RSSetScissorRects(1, &mScissorRect);
}

void D3DApp::Set4xMsaaState(bool value)
{
	if (m4xMsaaState != value)
	{
		m4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		CreateSwapChain();
		OnResize();
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
