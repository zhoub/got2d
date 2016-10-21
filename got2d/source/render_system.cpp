#include "render_system.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")
#include <string>
namespace
{
	const std::string vsCode = R"(
		struct GeometryVertex
		{
			float2 position : POSITION;
			float2 texcoord : TEXCOORD0;
			float4 vtxcolor : COLOR;
		};
		struct VertexOutput
		{
			float4 position : SV_POSITION;
			float2 texcoord : TEXCOORD0;
			float4 vtxcolor : COLOR;
		};
		VertexOutput VSMain(GeometryVertex input)
		{
			VertexOutput output;
			output.position = float4(input.position, 0, 1);
			output.texcoord = input.texcoord;
			output.vtxcolor = input.vtxcolor;
			return output;
		}
	)";

	const std::string psCode = R"(
		struct VertexInput
		{
			float4 position : SV_POSITION;
			float2 texcoord : TEXCOORD0;
			float4 vtxcolor : COLOR;
		};
		float4 PSMain(VertexInput input):SV_TARGET
		{
			return input.vtxcolor;
		}
	)";
}

bool Geometry::Create(ID3D11Device* device, unsigned int vertexCount, unsigned int indexCount)
{
	if (vertexCount == 0 || indexCount == 0)
		return false;

	m_vertexCount = vertexCount;
	m_indexCount = indexCount;

	do
	{
		D3D11_BUFFER_DESC bufferDesc =
		{
			sizeof(g2d::GeometryVertex) * m_vertexCount,//UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,						//D3D11_USAGE Usage;
			D3D11_BIND_VERTEX_BUFFER,					//UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,						//UINT CPUAccessFlags;
			0,											//UINT MiscFlags;
			0											//UINT StructureByteStride;
		};

		if (S_OK != device->CreateBuffer(&bufferDesc, NULL, &m_vertexBuffer))
		{
			break;
		}

		bufferDesc.ByteWidth = sizeof(unsigned int) * m_indexCount;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		if (S_OK != device->CreateBuffer(&bufferDesc, NULL, &m_indexBuffer))
		{
			break;
		}

		return true;
	} while (false);
	Destroy();
	return false;
}

void Geometry::UploadVertices(ID3D11DeviceContext* ctx, g2d::GeometryVertex* vertices)
{
	if (vertices == nullptr || m_vertexBuffer == nullptr)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == ctx->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		memcpy(mappedResource.pData, vertices, sizeof(g2d::GeometryVertex) * m_vertexCount);
		ctx->Unmap(m_vertexBuffer, 0);
	}
}
void Geometry::UploadIndices(ID3D11DeviceContext* ctx, unsigned int* indices)
{
	if (indices == nullptr || m_indexBuffer == nullptr)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == ctx->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		memcpy(mappedResource.pData, indices, sizeof(unsigned int) * m_indexCount);
		ctx->Unmap(m_indexBuffer, 0);
	}
}

void Geometry::Destroy()
{
	SR(m_vertexBuffer);
	SR(m_indexBuffer);
	m_vertexCount = 0;
	m_indexCount = 0;
}

RenderSystem::RenderSystem() :m_bkColor(gml::color4::blue())
{
}

bool RenderSystem::OnResize(int width, int height)
{
	//though we create an individual render target
	//we do not use it for rendering, for now.
	//it will be used after Compositor System finished.
	SR(m_colorTexture);
	SR(m_rtView);
	SR(m_bbView);

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
	int windowWidth = 0;
	int windowHeight = 0;

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
		IDXGIFactory1* factory = nullptr;
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

		auto ret = factory->CreateSwapChain(m_d3dDevice, &scDesc, &m_swapChain);
		factory->Release();
		if (S_OK != ret)
		{
			break;
		}

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

		g2d::GeometryVertex vertices[4];
		vertices[0].position.set(-0.5f, -0.5f);
		vertices[1].position.set(-0.5f, +0.5f);
		vertices[2].position.set(+0.5f, +0.5f);
		vertices[3].position.set(+0.5f, -0.5f);

		vertices[0].vtxcolor = gml::color4::red();
		vertices[1].vtxcolor = gml::color4::red();
		vertices[2].vtxcolor = gml::color4::red();
		vertices[3].vtxcolor = gml::color4::red();

		unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
		m_geometry.Create(m_d3dDevice, 4, 6);
		m_geometry.UploadVertices(m_d3dContext, vertices);
		m_geometry.UploadIndices(m_d3dContext, indices);

		//compile shader
		ID3DBlob* vsBlob = nullptr;
		ID3DBlob* vsError = nullptr;
		ret = D3DCompile(
			vsCode.c_str(),
			vsCode.length() + 1,
			NULL,
			NULL,
			NULL,
			"VSMain",
			"vs_5_0",
			0,
			0,
			&vsBlob,
			&vsError);

		if (S_OK != ret)
		{
			if (vsError)
			{
				std::string error = (const char*)vsError->GetBufferPointer();
				vsError->Release();
			}
			break;
		}

		//vetex shader
		ret = m_d3dDevice->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&m_vertexShader);

		if (S_OK != ret)
		{
			break;
		}

		D3D11_INPUT_ELEMENT_DESC layoutDesc[3];
		layoutDesc[0].SemanticName = "POSITION";
		layoutDesc[0].SemanticIndex = 0;
		layoutDesc[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		layoutDesc[0].InputSlot = 0;
		layoutDesc[0].AlignedByteOffset = 0;
		layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layoutDesc[0].InstanceDataStepRate = 0;

		layoutDesc[1].SemanticName = "TEXCOORD";
		layoutDesc[1].SemanticIndex = 0;
		layoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		layoutDesc[1].InputSlot = 0;
		layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layoutDesc[1].InstanceDataStepRate = 0;

		layoutDesc[2].SemanticName = "COLOR";
		layoutDesc[2].SemanticIndex = 0;
		layoutDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		layoutDesc[2].InputSlot = 0;
		layoutDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		layoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		layoutDesc[2].InstanceDataStepRate = 0;

		ret = m_d3dDevice->CreateInputLayout(layoutDesc, sizeof(layoutDesc) / sizeof(layoutDesc[0]), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_shaderLayout);
		SR(vsBlob);
		if (S_OK != ret)
		{
			break;
		}

		ID3DBlob* psBlob = nullptr;
		ID3DBlob* psError = nullptr;
		ret = D3DCompile(
			psCode.c_str(),
			psCode.length() + 1,
			NULL,
			NULL,
			NULL,
			"PSMain",
			"ps_5_0",
			0,
			0,
			&psBlob,
			&psError);

		if (S_OK != ret)
		{
			if (psError)
			{
				std::string errorString = (const char*)psError->GetBufferPointer();
				psError->Release();
			}
			break;
		}

		ret = m_d3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &m_pixelShader);
		SR(psBlob);
		if (S_OK != ret)
		{
			return false;
		}
		return true;
	} while (false);


	Destroy();

	return false;
}

void RenderSystem::Destroy()
{
	SR(m_vertexShader);
	SR(m_pixelShader);
	SR(m_shaderLayout);
	SR(m_colorTexture);
	SR(m_rtView);
	SR(m_bbView);
	SR(m_d3dDevice);
	SR(m_d3dContext);
	SR(m_swapChain);
}

void RenderSystem::Clear()
{
	m_d3dContext->ClearRenderTargetView(m_bbView, static_cast<float*>(m_bkColor));
}

void RenderSystem::Present()
{
	m_swapChain->Present(0, 0);
}

void RenderSystem::Render()
{
	auto stride = sizeof(g2d::GeometryVertex);
	unsigned int offset = 0;
	m_d3dContext->IASetVertexBuffers(0, 1, &(m_geometry.m_vertexBuffer), &stride, &offset);
	m_d3dContext->IASetIndexBuffer(m_geometry.m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_d3dContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->IASetInputLayout(m_shaderLayout);
	m_d3dContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dContext->PSSetShader(m_pixelShader, NULL, 0);

	m_d3dContext->DrawIndexed(6, 0, 0);
}