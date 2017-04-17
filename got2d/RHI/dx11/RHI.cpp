#include "inner_RHI.h"
#include "../source/inner_utility.h"
#include "../source/scope_utility.h"

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

rhi::RHICreationResult rhi::CreateRHI()
{
	RHICreationResult pair;

	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	auto fb = create_fallback([&] {
		SR(d3dDevice);
		SR(d3dContext);
	});

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
	D3D11_CREATE_DEVICE_FLAG deviceFlag = D3D11_CREATE_DEVICE_SINGLETHREADED;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr = ::D3D11CreateDevice(
		NULL,		//adapter
		driverType,	//hardware device
		NULL,		//no software renderer
		deviceFlag,	//single-thread
		&featureLevel, 1,	//feature level setting
		D3D11_SDK_VERSION,
		&d3dDevice,
		NULL,		//dont care feature level, since we have only one feature.
		&d3dContext);

	if (S_OK != hr)
	{
		pair.success = false;
	}
	else
	{
		ENSURE(d3dDevice != nullptr && d3dContext != nullptr);
		pair.success = true;
		pair.device = new ::Device(*d3dDevice);
		pair.context = new ::Context(*d3dContext);
		fb.cancel();
	}
	return pair;
}

Buffer::Buffer(ID3D11Buffer & buffer, rhi::BufferBinding binding, rhi::ResourceUsage usage, uint32_t length)
	: m_buffer(buffer)
	, m_bufferBinding(binding)
	, m_bufferUsage(usage)
	, m_bufferLength(length)
{
}

Buffer::~Buffer()
{
	m_buffer.Release();
}


RenderTargetView::RenderTargetView(ID3D11RenderTargetView & rtView)
	: m_rtView(rtView)
{
}

RenderTargetView::~RenderTargetView()
{
	m_rtView.Release();
}

DepthStencilView::DepthStencilView(ID3D11DepthStencilView & dsView)
	: m_dsView(dsView)
{
}

DepthStencilView::~DepthStencilView()
{
	m_dsView.Release();
}

ShaderResourceView::ShaderResourceView(ID3D11ShaderResourceView & srView)
	: m_srView(srView)
{
}

ShaderResourceView::~ShaderResourceView()
{
	m_srView.Release();
}

SwapChain::SwapChain(IDXGISwapChain & swapChain)
	: m_swapChain(swapChain)
{
	UpdateWindowSize();
}

SwapChain::~SwapChain()
{
	m_swapChain.Release();
}

Texture2D::Texture2D(ID3D11Texture2D& texture, rhi::TextureFormat format, uint32_t width, uint32_t height)
	: m_texture(texture)
	, m_textureWidth(width)
	, m_textureHeight(height)
	, m_textureFormat(format)
{
}

Texture2D::~Texture2D()
{
	m_texture.Release();
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
		ENSURE(backBuffer != nullptr);
		D3D11_TEXTURE2D_DESC desc;
		backBuffer->GetDesc(&desc);
		return new ::Texture2D(*backBuffer, GetTextureFormat(desc.Format), m_windowWidth, m_windowHeight);
	}
}

bool SwapChain::ResizeBackBuffer(uint32_t width, uint32_t height)
{
	return (S_OK == m_swapChain.ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
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