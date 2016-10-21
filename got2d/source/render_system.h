#pragma once

#include "../include/g2drender.h"
#include <windows.h>
#include <d3d11.h>
#include <gmlcolor.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")



class Geometry
{
public:
	bool Create(ID3D11Device* device, unsigned int vertexCount, unsigned int indexCount);
	void UploadVertices(ID3D11DeviceContext*, g2d::GeometryVertex*);
	void UploadIndices(ID3D11DeviceContext*, unsigned int*);
	void Destroy();

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	unsigned int m_vertexCount = 0;
	unsigned int m_indexCount = 0;
};

class RenderSystem
{
public:
	RenderSystem();

	bool Create(void* nativeWindow);
	bool OnResize(int width, int height);
	void Destroy();

	void Clear();
	void Render();
	void Present();

private:
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dContext = nullptr;
	ID3D11Texture2D* m_colorTexture = nullptr;
	ID3D11RenderTargetView* m_rtView = nullptr;
	ID3D11RenderTargetView* m_bbView = nullptr;
	D3D11_VIEWPORT m_viewport;

	gml::color4 m_bkColor;


	Geometry m_geometry;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_shaderLayout;
};
