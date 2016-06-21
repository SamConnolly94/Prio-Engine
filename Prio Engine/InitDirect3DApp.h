#pragma once
#include "D3DApp.h"

using namespace DirectX;

class CInitDirect3DApp : public D3DApp
{
private:
	virtual void OnResize()override;
	virtual void Update(const CGameTimer& gt)override;
	virtual void Draw(const CGameTimer& gt)override;
public:
	CInitDirect3DApp::CInitDirect3DApp(HINSTANCE hInstance);
	~CInitDirect3DApp();

	virtual bool Initialise()override;
};

