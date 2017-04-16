#include "inner_RHI.h"
#include "../source/inner_utility.h"
#include "../source/scope_utility.h"

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

Buffer::Buffer(ID3D11Buffer & buffer, rhi::BufferBinding binding, rhi::BufferUsage usage, uint32_t length)
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

SwapChain::SwapChain(IDXGISwapChain & swapChain)
	: m_swapChain(swapChain)
{
}

SwapChain::~SwapChain()
{
	m_swapChain.Release();
}

Device::Device(ID3D11Device& d3dDevice)
	: m_d3dDevice(d3dDevice)
{
}

Device::~Device()
{
	m_d3dDevice.Release();
}


rhi::MappedResource Context::Map(rhi::Buffer * buffer)
{
	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	D3D11_MAPPED_SUBRESOURCE d3dMappedRes;
	rhi::MappedResource mappedRes;
	if (S_OK == m_d3dContext.Map(bufferImpl->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedRes))
	{
		mappedRes.success = true;
		mappedRes.data = d3dMappedRes.pData;
		mappedRes.linePitch = d3dMappedRes.RowPitch;
	}
	else
	{
		mappedRes.success = false;
	}
	return mappedRes;
}

void Context::Unmap(rhi::Buffer* buffer)
{
	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	m_d3dContext.Unmap(bufferImpl->GetRaw(), 0);
}

Context::Context(ID3D11DeviceContext& d3dContext)
	: m_d3dContext(d3dContext)
{
}

Context::~Context()
{
	m_d3dContext.Release();
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
	ENSURE(swapChain != nullptr);

	return new ::SwapChain(*swapChain);
}

rhi::Buffer * Device::CreateBuffer(rhi::BufferBinding binding, rhi::BufferUsage usage, uint32_t bufferLength)
{
	const UINT kBindings[] =
	{
		D3D11_BIND_VERTEX_BUFFER,		// Vertex = 0,
		D3D11_BIND_INDEX_BUFFER,		// Index = 1,
		D3D11_BIND_CONSTANT_BUFFER,		// Constant = 2,
	};

	const D3D11_USAGE kUsage[] =
	{
		D3D11_USAGE_DEFAULT,	// Default = 0
		D3D11_USAGE_DYNAMIC,	// Dynamic = 1,
	};

	D3D11_BUFFER_DESC bufferDesc =
	{
		bufferLength,				//UINT ByteWidth;
		kUsage[(int)usage],			//D3D11_USAGE Usage;
		kBindings[(int)binding],	//UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE,		//UINT CPUAccessFlags;
		0,							//UINT MiscFlags;
		0							//UINT StructureByteStride;
	};

	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = m_d3dDevice.CreateBuffer(&bufferDesc, NULL, &buffer);
	if (S_OK != hr)
	{
		return false;
	}

	ENSURE(buffer != nullptr);
	return new ::Buffer(*buffer, binding, usage, bufferLength);
}

gml::rect SwapChain::GetRect()
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	m_swapChain.GetDesc(&scDesc);
	gml::rect result;
	result.set_pos(0, 0);
	result.set_size(scDesc.BufferDesc.Width, scDesc.BufferDesc.Height);
	return result;
}

void SwapChain::Present()
{
	m_swapChain.Present(0, 0);
}