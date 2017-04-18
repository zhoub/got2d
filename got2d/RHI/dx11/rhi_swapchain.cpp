#include "inner_RHI.h"
#include "../source/scope_utility.h"
#include "dx11_enum.h"

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
	if (S_OK == m_swapChain.GetBuffer(0, _uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))
	{
		D3D11_TEXTURE2D_DESC desc;
		backBuffer->GetDesc(&desc);
		return new ::Texture2D(*backBuffer,
			GetTextureFormatDX11(desc.Format),
			desc.Width, desc.Height);
	}
	else
	{
		return nullptr;
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
			UpdateWindowSize();
		}
		else
		{
			FAIL("cannot retrieve fullsreen state");
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