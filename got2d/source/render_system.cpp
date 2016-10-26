#include "render_system.h"
#include "engine.h"
#include <string>
#include "file_data.h"
#include "img_data.h"

g2d::RenderSystem::~RenderSystem() { }
RenderSystem* RenderSystem::Instance = nullptr;

bool Geometry::Create(unsigned int vertexCount, unsigned int indexCount)
{
	if (vertexCount == 0 || indexCount == 0)
		return false;

	m_numVertices = vertexCount;
	m_numIndices = indexCount;

	do
	{
		if (!MakeEnoughVertexArray(vertexCount))
		{
			break;
		}

		if (!MakeEnoughIndexArray(indexCount))
		{
			break;
		}

		return true;
	} while (false);
	Destroy();
	return false;
}

bool Geometry::MakeEnoughVertexArray(unsigned int numVertices)
{
	if (m_numVertices >= numVertices)
	{
		return true;
	}


	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(g2d::GeometryVertex) * numVertices,//UINT ByteWidth;
		D3D11_USAGE_DYNAMIC,						//D3D11_USAGE Usage;
		D3D11_BIND_VERTEX_BUFFER,					//UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE,						//UINT CPUAccessFlags;
		0,											//UINT MiscFlags;
		0											//UINT StructureByteStride;
	};

	ID3D11Buffer* vertexBuffer;
	if (S_OK != GetRenderSystem()->GetDevice()->CreateBuffer(&bufferDesc, NULL, &vertexBuffer))
	{
		return  false;
	}

	m_numVertices = numVertices;
	SR(m_vertexBuffer);
	m_vertexBuffer = vertexBuffer;
	return true;
}

bool Geometry::MakeEnoughIndexArray(unsigned int numIndices)
{
	if (m_numIndices >= numIndices)
	{
		return true;
	}

	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(unsigned int) * numIndices,//UINT ByteWidth;
		D3D11_USAGE_DYNAMIC,						//D3D11_USAGE Usage;
		D3D11_BIND_INDEX_BUFFER,					//UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE,						//UINT CPUAccessFlags;
		0,											//UINT MiscFlags;
		0											//UINT StructureByteStride;
	};

	ID3D11Buffer* indexBuffer;
	if (S_OK != GetRenderSystem()->GetDevice()->CreateBuffer(&bufferDesc, NULL, &indexBuffer))
	{
		return false;
	}
	m_numIndices = numIndices;
	SR(m_indexBuffer);
	m_indexBuffer = indexBuffer;
	return true;
}

void Geometry::UploadVertices(unsigned int offset, g2d::GeometryVertex* vertices, unsigned int count)
{
	if (vertices == nullptr || m_vertexBuffer == nullptr)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == GetRenderSystem()->GetContext()->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		count = __min(m_numVertices - offset, count);
		g2d::GeometryVertex* data = reinterpret_cast<g2d::GeometryVertex*>(mappedResource.pData);
		memcpy(data + offset, vertices, sizeof(g2d::GeometryVertex) * count);
		GetRenderSystem()->GetContext()->Unmap(m_vertexBuffer, 0);
	}
}

void Geometry::UploadIndices(unsigned int offset, unsigned int* indices, unsigned int count)
{
	if (indices == nullptr || m_indexBuffer == nullptr)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == GetRenderSystem()->GetContext()->Map(m_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		count = __min(m_numIndices - offset, count);
		unsigned int* data = reinterpret_cast<unsigned int*>(mappedResource.pData);
		memcpy(data + offset, indices, sizeof(unsigned int) * count);
		GetRenderSystem()->GetContext()->Unmap(m_indexBuffer, 0);
	}
}

void Geometry::Destroy()
{
	SR(m_vertexBuffer);
	SR(m_indexBuffer);
	m_numVertices = 0;
	m_numIndices = 0;
}

bool Texture2D::Create(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
		return false;

	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;
	if (S_OK != GetRenderSystem()->GetDevice()->CreateTexture2D(&texDesc, nullptr, &m_texture))
	{
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	::ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = -1;
	viewDesc.Texture2D.MostDetailedMip = 0;
	if (S_OK != GetRenderSystem()->GetDevice()->CreateShaderResourceView(m_texture, &viewDesc, &m_shaderView))
	{
		Destroy();
		return false;
	}

	m_width = width;
	m_height = height;
	return true;
}

void Texture2D::UploadImage(unsigned char* data, bool hasAlpha)
{
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	if (S_OK == GetRenderSystem()->GetContext()->Map(m_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes))
	{
		unsigned char* colorBuffer = static_cast<unsigned char*>(mappedRes.pData);
		if (hasAlpha)
		{
			int srcPitch = m_width * 4;
			for (unsigned int i = 0; i < m_height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedRes.RowPitch;
				auto srcPtr = data + i * srcPitch;
				memcpy(dstPtr, srcPtr, srcPitch);
			}
		}
		else
		{
			int srcPitch = m_width * 3;
			for (unsigned int i = 0; i < m_height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedRes.RowPitch;
				auto srcPtr = data + i * srcPitch;
				for (unsigned int j = 0; j < m_width; j++)
				{
					memcpy(dstPtr + j * 4, srcPtr + j * 3, 3);
				}
			}
		}

		GetRenderSystem()->GetContext()->Unmap(m_texture, 0);
		GetRenderSystem()->GetContext()->GenerateMips(m_shaderView);
	}

}

void Texture2D::Destroy()
{
	SR(m_texture);
	SR(m_shaderView);
	m_width = 0;
	m_height = 0;
}

RenderSystem::RenderSystem() :m_bkColor(gml::color4::blue()), m_mesh(0, 0)
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
	if (Instance)
	{
		return Instance == this;
	}

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
		Instance = this;

		std::string res = ::GetEngine()->GetResourceRoot() + "test_alpha.bmp";
		file_data f;
		if (!load_file(res.c_str(), f))
			break;

		img_data img;
		if (!read_bmp(f.buffer, img))
		{
			destroy_file_data(f);
			break;
		}

		destroy_file_data(f);

		if (!m_defaultTex.Create(img.width, img.height))
		{
			destroy_img_data(img);
			break;
		}

		m_defaultTex.UploadImage(img.raw_data, img.has_alpha);
		destroy_img_data(img);

		shaderlib = new ShaderLib();
		return true;
	} while (false);


	Destroy();

	return false;
}

void RenderSystem::Destroy()
{
	m_geometry.Destroy();
	m_defaultTex.Destroy();
	if (shaderlib)
	{
		delete shaderlib;
		shaderlib = nullptr;
	}
	SR(m_colorTexture);
	SR(m_rtView);
	SR(m_bbView);
	SR(m_d3dDevice);
	SR(m_d3dContext);
	SR(m_swapChain);

	if (Instance == this)
	{
		Instance = nullptr;
	}
}

void RenderSystem::Clear()
{
	m_d3dContext->ClearRenderTargetView(m_bbView, static_cast<float*>(m_bkColor));
}

void RenderSystem::Present()
{
	m_swapChain->Present(0, 0);
}

Texture* RenderSystem::CreateTextureFromFile(const char* resPath)
{
	return new Texture(resPath);
}

void RenderSystem::FlushBatch()
{
	m_geometry.MakeEnoughVertexArray(m_mesh.GetVertexCount());
	m_geometry.MakeEnoughIndexArray(m_mesh.GetIndexCount());
	m_geometry.UploadVertices(0, m_mesh.GetRawVertices(), m_mesh.GetVertexCount());
	m_geometry.UploadIndices(0, m_mesh.GetRawIndices(), m_mesh.GetIndexCount());

	auto shader = shaderlib->GetShaderByName("simple.texture");
	if (shader)
	{
		unsigned int stride = sizeof(g2d::GeometryVertex);
		unsigned int offset = 0;
		m_d3dContext->IASetVertexBuffers(0, 1, &(m_geometry.m_vertexBuffer), &stride, &offset);
		m_d3dContext->IASetIndexBuffer(m_geometry.m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_d3dContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_d3dContext->IASetInputLayout(shader->GetInputLayout());

		m_d3dContext->VSSetShader(shader->GetVertexShader(), NULL, 0);
		m_d3dContext->PSSetShader(shader->GetPixelShader(), NULL, 0);
		m_d3dContext->PSSetShaderResources(0, 1, &(m_defaultTex.m_shaderView));
		ID3D11SamplerState* a = nullptr;
		m_d3dContext->PSSetSamplers(0, 1, &a);

		m_d3dContext->DrawIndexed(m_mesh.GetIndexCount(), 0, 0);
	}
}

void RenderSystem::Render()
{
	if (m_mesh.GetIndexCount() > 0)
	{
		FlushBatch();
	}
}

g2d::Mesh* RenderSystem::CreateMesh(unsigned int vertexCount, unsigned int indexCount)
{
	return new Mesh(vertexCount, indexCount);
}

void RenderSystem::RenderMesh(g2d::Mesh* m, g2d::Texture* t, const gml::mat32& transform)
{
	::Texture* timpl = dynamic_cast<::Texture*>(t);
	//m_texture = (timpl == nullptr) ? "default" : timpl->GetResourceName();

	if (m_mesh.Merge(m, transform))
	{
		return;
	}

	FlushBatch();

	m_mesh.Clear();
	//de factor, no need to Merge when there is only ONE MESH each drawcall.
	m_mesh.Merge(m, transform);
}

void RenderSystem::BeginRender()
{
	m_mesh.Clear();
	//render
	Clear();
}

void RenderSystem::EndRender()
{
	Render();
	Present();
}