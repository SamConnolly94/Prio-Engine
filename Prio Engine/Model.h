#ifndef MODEL_H
#define MODEL_H

#include <d3d11.h>
#include <D3DX10math.h>

class CModel
{
private:
	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR4 colour;
	};
	
public:
	CModel();
	~CModel();
public:
	bool Initialise(ID3D11Device* device);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndex();

private:
	bool InitialiseBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);
private:
	ID3D11Buffer* mpVertexBuffer;
	ID3D11Buffer* mpIndexBuffer;
	int mVertexCount;
	int mIndexCount;
};

#endif