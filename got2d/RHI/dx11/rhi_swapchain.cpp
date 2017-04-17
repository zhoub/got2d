#include "inner_RHI.h"
#include "../source/inner_utility.h"

rhi::TextureFormat GetTextureFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	default:
	case DXGI_FORMAT_UNKNOWN: return rhi::TextureFormat::Unknown;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return rhi::TextureFormat::RGBA;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return rhi::TextureFormat::BGRA;
	case DXGI_FORMAT_BC1_UNORM: return rhi::TextureFormat::DXT1;
	case DXGI_FORMAT_BC2_UNORM: return rhi::TextureFormat::DXT3;
	case DXGI_FORMAT_BC3_UNORM: return rhi::TextureFormat::DXT5;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return rhi::TextureFormat::D24S8;
	case DXGI_FORMAT_R32_FLOAT: return rhi::TextureFormat::Float32;
	}
}

SwapChain::SwapChain(IDXGISwapChain & swapChain)
	: m_swapChain(swapChain)
{
	UpdateWindowSize();
}

SwapChain::~SwapChain()
{
	if (IsFullscreen())
	{
		SetFullscreen(false);
	}
	m_swapChain.Release();
}

rhi::Texture2D* SwapChain::GetBackBuffer()
{
	ID3D11Texture2D* backBuffer = nullptr;
	if (S_OK != m_swapChain.GetBuffer(0, _uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))
	{
		return nullptr;
	}
	else
	{
		D3D11_TEXTURE2D_DESC desc;
		backBuffer->GetDesc(&desc);
		return new ::Texture2D(*backBuffer,
			GetTextureFormat(desc.Format),
			desc.Width, desc.Height);
	}
}

bool SwapChain::ResizeBackBuffer(uint32_t width, uint32_t height)
{
	if (S_OK == m_swapChain.ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))
	{
		UpdateWindowSize();
		return true;
	}
	else
	{
		return false;
	}
}

void SwapChain::SetFullscreen(bool fullscreen)
{
	if (m_fullscreen != fullscreen)
	{
		m_swapChain.SetFullscreenState(fullscreen ? TRUE : FALSE, NULL);
		BOOL isFullScreen;
		if (S_OK == m_swapChain.GetFullscreenState(&isFullScreen, NULL))
		{
			m_fullscreen = (isFullScreen == TRUE);
		}
		else
		{
			ENSURE(false);
			m_fullscreen = false;
		}
	};
}

void SwapChain::Present()
{
	m_swapChain.Present(0, 0);
}

void SwapChain::UpdateWindowSize()
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	m_swapChain.GetDesc(&scDesc);
	m_windowWidth = scDesc.BufferDesc.Width;
	m_windowHeight = scDesc.BufferDesc.Height;
}