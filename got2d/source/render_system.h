#pragma once

#include <windows.h>
#include <d3d11.h>
#include <gmlcolor.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class RenderSystem
{
public:
	RenderSystem();

	bool Create(void* nativeWindow);
	bool OnResize(int width, int height);
	void Destroy();

	void Clear();
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
};
