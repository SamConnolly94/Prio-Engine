#include "InitDirect3DApp.h"

CInitDirect3DApp::CInitDirect3DApp(HINSTANCE hInstance) : D3DApp(hInstance)
{
}


CInitDirect3DApp::~CInitDirect3DApp()
{
}


void CInitDirect3DApp::OnResize()
{
}

//void CInitDirect3DApp::Update(const CGameTimer & gt)
//{
//}
//
//void CInitDirect3DApp::Draw(const CGameTimer & gt)
//{
//}

bool CInitDirect3DApp::Initialise()
{
	if (!D3DApp::Initialise())
	{
		return false;
	}
	return true;
}