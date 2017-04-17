#include "inner_RHI.h"
#include "../source/inner_utility.h"
#include "../source/scope_utility.h"

namespace
{
	const D3D11_USAGE kResourceUsage[] =
	{
		D3D11_USAGE_DEFAULT,	// Default = 0
		D3D11_USAGE_DYNAMIC,	// Dynamic = 1,
	};

	const DXGI_FORMAT kTextureFormat[] =
	{
		DXGI_FORMAT_UNKNOWN,			// Unknown
		DXGI_FORMAT_R8G8B8A8_UNORM,		// RGBA
		DXGI_FORMAT_B8G8R8X8_UNORM,		// BGRA
		DXGI_FORMAT_BC1_UNORM,			// DXT1
		DXGI_FORMAT_BC2_UNORM,			// DXT3
		DXGI_FORMAT_BC3_UNORM,			// DXT5
		DXGI_FORMAT_D24_UNORM_S8_UINT,	// D24S8
		DXGI_FORMAT_R32_FLOAT,			// Float32
	};
}

Device::Device(ID3D11Device& d3dDevice)
	: m_d3dDevice(d3dDevice)
{
}

Device::~Device()
{
	m_d3dDevice.Release();
}

rhi::SwapChain * Device::CreateSwapChain(void* nativeWindow, uint32_t windowWidth, uint32_t windowHeight)
{
	autor<IDXGIDevice> dxgiDevice = nullptr;
	autor<IDXGIAdapter> adapter = nullptr;
	autor<IDXGIFactory> factory = nullptr;

	HRESULT hr = m_d3dDevice.QueryInterface(__uuidof(IDXGIDevice), (void **)&(dxgiDevice.pointer));
	if (S_OK != hr)
	{
		return nullptr;
	}
	ENSURE(dxgiDevice.is_not_null());

	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&(adapter.pointer));
	if (S_OK != hr)
	{
		return nullptr;
	}
	ENSURE(adapter.is_not_null());

	hr = adapter->GetParent(__uuidof(IDXGIFactory), (void **)&(factory.pointer));
	if (S_OK != hr)
	{
		return nullptr;
	}
	ENSURE(factory.is_not_null());

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

	IDXGISwapChain* swapChain = nullptr;
	hr = factory->CreateSwapChain(&m_d3dDevice, &scDesc, &swapChain);
	if (S_OK != hr)
	{
		return nullptr;
	}
	else
	{
		ENSURE(swapChain != nullptr);
		return new ::SwapChain(*swapChain);
	}
}

rhi::Buffer * Device::CreateBuffer(rhi::BufferBinding binding, rhi::ResourceUsage usage, uint32_t bufferLength)
{
	const UINT kBindings[] =
	{
		D3D11_BIND_VERTEX_BUFFER,		// Vertex = 0,
		D3D11_BIND_INDEX_BUFFER,		// Index = 1,
		D3D11_BIND_CONSTANT_BUFFER,		// Constant = 2,
	};

	D3D11_BUFFER_DESC bufferDesc =
	{
		bufferLength,				//UINT ByteWidth;
		kResourceUsage[(int)usage],			//D3D11_USAGE Usage;
		kBindings[(int)binding],	//UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE,		//UINT CPUAccessFlags;
		0,							//UINT MiscFlags;
		0							//UINT StructureByteStride;
	};

	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = m_d3dDevice.CreateBuffer(&bufferDesc, NULL, &buffer);
	if (S_OK != hr)
	{
		return nullptr;
	}
	else
	{
		ENSURE(buffer != nullptr);
		return new ::Buffer(*buffer, binding, usage, bufferLength);
	}
}

rhi::Texture2D * Device::CreateTexture2D(rhi::TextureFormat format, rhi::ResourceUsage usage, uint32_t binding, uint32_t width, uint32_t height)
{
	const D3D11_BIND_FLAG kBinding[] =
	{
		D3D11_BIND_SHADER_RESOURCE,
		D3D11_BIND_RENDER_TARGET,
		D3D11_BIND_DEPTH_STENCIL,
		D3D11_BIND_STREAM_OUTPUT,
		D3D11_BIND_UNORDERED_ACCESS
	};

	D3D11_TEXTURE2D_DESC colorTexDesc;
	colorTexDesc.Width = width;
	colorTexDesc.Height = height;
	colorTexDesc.MipLevels = 1;
	colorTexDesc.ArraySize = 1;
	colorTexDesc.Format = kTextureFormat[(int)format];
	colorTexDesc.SampleDesc.Count = 1;
	colorTexDesc.SampleDesc.Quality = 0;
	colorTexDesc.Usage = kResourceUsage[(int)usage];
	colorTexDesc.CPUAccessFlags = usage == rhi::ResourceUsage::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	colorTexDesc.MiscFlags = 0;
	colorTexDesc.BindFlags = 0;

	for (int i = 0, n = (int)rhi::TextureBinding::Count; i < n; i++)
	{
		auto bindingFlag = 1 << i;
		if ((binding & bindingFlag) != 0)
		{
			colorTexDesc.BindFlags |= kBinding[i];
		}
	}

	ID3D11Texture2D* texture = nullptr;
	if (S_OK != m_d3dDevice.CreateTexture2D(&colorTexDesc, nullptr, &texture))
	{
		return nullptr;
	}
	else
	{
		ENSURE(texture != nullptr);
		return new ::Texture2D(*texture, format, width, height);
	}
}

rhi::RenderTargetView * Device::CreateRenderTargetView(rhi::Texture2D * texture2D)
{
	::Texture2D* textureImpl = reinterpret_cast<::Texture2D*>(texture2D);
	ENSURE(textureImpl != nullptr);

	ID3D11RenderTargetView* rtView = nullptr;
	if (S_OK != m_d3dDevice.CreateRenderTargetView(textureImpl->GetRaw(), NULL, &rtView))
	{
		return nullptr;
	}
	else
	{
		ENSURE(rtView != nullptr);
		return new ::RenderTargetView(*rtView);
	}

}

rhi::ShaderResourceView * Device::CreateShaderResourceView(rhi::Texture2D * texture2D)
{
	::Texture2D* textureImpl = reinterpret_cast<::Texture2D*>(texture2D);
	ENSURE(textureImpl != nullptr);

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	::ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = kTextureFormat[(int)textureImpl->GetFormat()];
	viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = -1;
	viewDesc.Texture2D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* srView = nullptr;
	if (S_OK != m_d3dDevice.CreateShaderResourceView(textureImpl->GetRaw(), &viewDesc, &srView))
	{
		return nullptr;
	}
	else
	{
		ENSURE(srView != nullptr);
		return new ::ShaderResourceView(*srView);
	}
}

rhi::DepthStencilView * Device::CreateDepthStencilView(rhi::Texture2D * texture2D)
{
	::Texture2D* textureImpl = reinterpret_cast<::Texture2D*>(texture2D);
	ENSURE(textureImpl != nullptr);

	ID3D11DepthStencilView* dsView = nullptr;
	if (S_OK != m_d3dDevice.CreateDepthStencilView(textureImpl->GetRaw(), NULL, &dsView))
	{
		return nullptr;
	}
	else
	{
		ENSURE(dsView != nullptr);
		return new ::DepthStencilView(*dsView);
	}
}
