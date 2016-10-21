#include "render_system.h"


bool RenderSystem::OnResize(int width, int height)
{
	//though we create an individual render target
	//we do not use it for rendering, for now.
	//it will be used after Compositor System finished.
	if (m_colorTexture)
	{
		m_colorTexture->Release();
		m_colorTexture = nullptr;
	}
	if (m_rtView)
	{
		m_rtView->Release();
		m_rtView = nullptr;
	}
	if (m_bbView)
	{
		m_bbView->Release();
		m_bbView = nullptr;
	}

	//CreateRenderTarget and Views.
	D3D11_TEXTURE2D_DESC colorTexDesc;
	colorTexDesc.Width = width;
	colorTexDesc.Height = height;
	colorTexDesc.MipLevels = 1;
	colorTexDesc.ArraySize = 1;
	colorTexDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
	colorTexDesc.SampleDesc.Count = 1;
	colorTexDesc.SampleDesc.Quality = 0;
	colorTexDesc.Usage = D3D11_USAGE_DEFAULT;
	colorTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	colorTexDesc.CPUAccessFlags = 0;
	colorTexDesc.MiscFlags = 0;

	if (S_OK != m_d3dDevice->CreateTexture2D(&colorTexDesc, nullptr, &m_colorTexture))
	{
		return false;
	}
	if (S_OK != m_d3dDevice->CreateRenderTargetView(m_colorTexture, NULL, &m_rtView))
	{
		return false;
	}

	ID3D11Texture2D* backBuffer = nullptr;
	if (S_OK != m_swapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))
		|| backBuffer == nullptr)
	{
		return false;
	}
	if (S_OK != m_d3dDevice->CreateRenderTargetView(backBuffer, NULL, &m_bbView))
	{
		return false;
	}
	return true;
}

bool RenderSystem::Create(void* nativeWindow)
{
	HWND hWnd = reinterpret_cast<HWND>(nativeWindow);
	int windowWidth = 0;
	int windowHeight = 0;

	IDXGIFactory1* factory = nullptr;
	do
	{
		//Create Device
		IDXGIAdapter1* adapter = NULL; //default adapter
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
		D3D11_CREATE_DEVICE_FLAG deviceFlag = D3D11_CREATE_DEVICE_SINGLETHREADED;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		if (S_OK != ::D3D11CreateDevice(adapter, driverType, NULL, deviceFlag, &featureLevel, 1, D3D11_SDK_VERSION, &m_d3dDevice, NULL, &m_d3dContext))
		{
			break;
		}

		//CreateSwapChain
		if (S_OK != ::CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory)))
		{
			break;
		}

		DXGI_SWAP_CHAIN_DESC scDesc;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferDesc.Width = windowWidth;
		scDesc.BufferDesc.Height = windowHeight;
		scDesc.BufferDesc.RefreshRate.Numerator = 60;
		scDesc.BufferDesc.RefreshRate.Denominator = 1;
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferCount = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		scDesc.OutputWindow = reinterpret_cast<HWND>(nativeWindow);
		scDesc.Windowed = true;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.Flags = 0;

		if (S_OK != factory->CreateSwapChain(m_d3dDevice, &scDesc, &m_swapChain))
		{
			break;
		}
		factory->Release();

		m_swapChain->GetDesc(&scDesc);
		windowWidth = scDesc.BufferDesc.Width;
		windowHeight = scDesc.BufferDesc.Height;

		if (!OnResize(windowWidth, windowHeight))
		{
			break;
		}

		m_viewport =
		{
			0.0f,//FLOAT TopLeftX;
			0.0f,//FLOAT TopLeftY;
			(FLOAT)windowWidth,//FLOAT Width;
			(FLOAT)windowHeight,//FLOAT Height;
			0.0f,//FLOAT MinDepth;
			1.0f,//FLOAT MaxDepth;
		};

		//m_d3dContext->OMSetRenderTargets(1, &m_rtView, nullptr);
		m_d3dContext->OMSetRenderTargets(1, &m_bbView, nullptr);
		m_d3dContext->RSSetViewports(1, &m_viewport);
		Clear();

		return true;
	} while (0);

	if (factory)
	{
		factory->Release();
	}
	Destroy();

	return false;
}

void RenderSystem::Destroy()
{

	if (m_colorTexture)
	{
		m_colorTexture->Release();
		m_colorTexture = nullptr;
	}
	if (m_rtView)
	{
		m_rtView->Release();
		m_rtView = nullptr;
	}
	if (m_bbView)
	{
		m_bbView->Release();
		m_bbView = nullptr;
	}

	if (m_d3dDevice)
	{
		m_d3dDevice->Release();
		m_d3dDevice = nullptr;
	}

	if (m_d3dContext)
	{
		m_d3dContext->Release();
		m_d3dContext = nullptr;
	}
	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}
}

void RenderSystem::Clear()
{
	float color[] = { 0.5f, 0.0f, 0.0f, 1.0f };
	m_d3dContext->ClearRenderTargetView(m_bbView, color);
}

void RenderSystem::Present()
{
	m_swapChain->Present(0, 0);
}