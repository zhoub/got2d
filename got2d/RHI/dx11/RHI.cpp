#include "inner_RHI.h"
#include "../source/inner_utility.h"
#include "../source/scope_utility.h"

rhi::RHICreationResult rhi::CreateRHI()
{
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	auto fb = create_fallback([&]
	{
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
		&featureLevel, 1,	//desire feature level setting
		D3D11_SDK_VERSION,
		&d3dDevice,
		NULL,		//dont care feature level, since we have only one feature.
		&d3dContext);

	RHICreationResult result;
	if (S_OK == hr)
	{
		result.Success = true;
		result.DevicePtr = new ::Device(*d3dDevice);
		result.ContextPtr = new ::Context(*d3dContext);
		fb.cancel();
	}
	else
	{
		result.Success = false;
	}
	return result;
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

ShaderProgram::ShaderProgram(ID3D11VertexShader& vertexShader, ID3D11PixelShader& pixelShader, ID3D11InputLayout& inputLayout)
	: m_vertexShader(vertexShader)
	, m_pixelShader(pixelShader)
	, m_inputLayout(inputLayout)
{

}

ShaderProgram::~ShaderProgram()
{
	m_vertexShader.Release();
	m_pixelShader.Release();
	m_inputLayout.Release();
}

TextureSampler::TextureSampler(ID3D11SamplerState& samplerState)
	: m_samplerState(samplerState)
{

}

TextureSampler::~TextureSampler()
{
	m_samplerState.Release();
}


BlendState::BlendState(ID3D11BlendState& blendState, bool enable, rhi::BlendFactor srcFactor, rhi::BlendFactor dstFactor, rhi::BlendOperator blendOp)
	: m_blendState(blendState)
	, m_srcFactor(srcFactor)
	, m_dstFactor(dstFactor)
	, m_blendOp(blendOp)
{

}

BlendState::~BlendState()
{
	m_blendState.Release();
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
