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


void Context::SetVertexBuffers(uint32_t startSlot, rhi::VertexBufferInfo * buffers, uint32_t bufferCount)
{
	m_vertexbuffers.clear();
	m_vertexBufferStrides.clear();
	m_vertexBufferOffsets.clear();
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		const rhi::VertexBufferInfo& info = buffers[i];
		m_vertexbuffers.push_back(((::Buffer*)info.buffer)->GetRaw());
		m_vertexBufferStrides.push_back(info.stride);
		m_vertexBufferOffsets.push_back(info.offset);
	}

	m_d3dContext.IASetVertexBuffers(startSlot, bufferCount,
		&(m_vertexbuffers[0]),
		&(m_vertexBufferStrides[0]),
		&(m_vertexBufferOffsets[0]));
}

void Context::SetIndexBuffer(rhi::Buffer* buffer, uint32_t offset, rhi::IndexFormat format)
{
	const DXGI_FORMAT kIndexFormat[] =
	{
		DXGI_FORMAT_R16_UINT,//		Int16 = 0,
		DXGI_FORMAT_R32_UINT,//		Int32 = 1,
	};

	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);
	m_d3dContext.IASetIndexBuffer(bufferImpl->GetRaw(), kIndexFormat[(int)format], offset);
}

void Context::SetVertexShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	m_vsConstantBuffers.clear();
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		m_vsConstantBuffers.push_back(((::Buffer*)buffers[i])->GetRaw());
	}
	m_d3dContext.VSSetConstantBuffers(startSlot, bufferCount, &(m_vsConstantBuffers[0]));
}

void Context::SetPixelShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	m_psConstantBuffers.clear();
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		m_psConstantBuffers.push_back(((::Buffer*)buffers[i])->GetRaw());
	}
	m_d3dContext.PSSetConstantBuffers(startSlot, bufferCount, &(m_psConstantBuffers[0]));
}

void Context::DrawIndexed(rhi::Primitive primitive, uint32_t startIndex, uint32_t indexOffset, uint32_t baseVertex)
{
	const D3D_PRIMITIVE_TOPOLOGY kPrimitive[] =
	{
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	//TriangleList = 0,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	//TriangleFan = 1,
	};
	m_d3dContext.IASetPrimitiveTopology(kPrimitive[(int)primitive]);
	m_d3dContext.DrawIndexed(startIndex, indexOffset, baseVertex);
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

bool SwapChain::ResizeBackBuffer(uint32_t width, uint32_t height)
{
	return (S_OK == m_swapChain.ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
}

void SwapChain::Present()
{
	m_swapChain.Present(0, 0);
}