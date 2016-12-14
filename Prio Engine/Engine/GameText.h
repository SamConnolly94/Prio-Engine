#ifndef GAMETEXT_H
#define GAMETEXT_H

#include "GameFont.h"
#include "FontShader.h"

class CGameText
{
private:
	struct SentenceType
	{
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		int vertexCount;
		int indexCount;
		int maxLength;
		float red;
		float green;
		float blue;
	};

	struct VertexType
	{
		D3DXVECTOR3 position;
		D3DXVECTOR2 texture;
	};
public:
	CGameText();
	~CGameText();

public:
	bool Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hWnd, int screenWidth, int screenHeight, D3DXMATRIX baseViewMatrix);
	void Shutdown();
	bool Render(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);
private:
	bool InitialiseSentence(SentenceType** sentence, int maxLength, ID3D11Device * device);
	bool UpdateSentence(SentenceType* sentence, char* text, int posX, int posY, float red, float green, float blue, ID3D11DeviceContext* deviceContext);
	void ReleaseSentence(SentenceType* sentence);
	bool RenderSentence(ID3D11DeviceContext* deviceContext, SentenceType* sentence, D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix);
private:
	CGameFont* mpFont;
	CFontShader* mpFontShader;
	int mScreenWidth;
	int mScreenHeight;
	D3DXMATRIX mBaseViewMatrix;

	//std::list<SentenceType*> mpSentences;
	SentenceType* mpSentence1;
};

#endif