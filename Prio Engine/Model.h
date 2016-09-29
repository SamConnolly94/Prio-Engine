#ifndef MODEL_H
#define MODEL_H

// Include DirectX libraries.
#include <d3d11.h>
#include <D3DX10math.h>

#include "Logger.h"
#include "Texture.h"

class CModel
{
private:
	//struct VertexType
	//{
	//	D3DXVECTOR3 position;
	//	D3DXVECTOR4 colour;
	//};
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};
public:
	CModel();
	~CModel();
public:
	bool Initialise(ID3D11Device* device, WCHAR* textureFilename);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndex();

	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitialiseBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
	bool LoadTexture(ID3D11Device* device, WCHAR* filename);
	void ReleaseTexture();
private:
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	int mVertexCount;
	int mIndexCount;
	CTexture* mpTexture;

	CLogger* mpLogger;
};

#endif