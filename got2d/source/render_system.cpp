#include "render_system.h"
#include <string>

g2d::RenderSystem::~RenderSystem() { }
RenderSystem* RenderSystem::Instance = nullptr;

RenderSystem::RenderSystem() :m_bkColor(gml::color4::blue()), m_mesh(0, 0)
{
}

bool RenderSystem::OnResize(long width, long height)
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

	m_matrixProjDirty = true;
	m_windowWidth = width;
	m_windowHeight = height;

	D3D11_MAPPED_SUBRESOURCE mappedData;
	if (S_OK == m_d3dContext->Map(m_bufferMatrix, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData))
	{
		memcpy(mappedData.pData, GetProjectionMatrix().m, sizeof(gml::mat44));
		m_d3dContext->Unmap(m_bufferMatrix, 0);
	}
	return true;
}

bool RenderSystem::Create(void* nativeWindow)
{
	if (Instance)
	{
		return Instance == this;
	}

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
		scDesc.BufferDesc.Width = m_windowWidth;
		scDesc.BufferDesc.Height = m_windowHeight;
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
		m_windowWidth = scDesc.BufferDesc.Width;
		m_windowHeight = scDesc.BufferDesc.Height;

		D3D11_BUFFER_DESC bufferDesc =
		{
			sizeof(gml::mat44) + sizeof(gml::mat32),	//UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,						//D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER,					//UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,						//UINT CPUAccessFlags;
			0,											//UINT MiscFlags;
			0											//UINT StructureByteStride;
		};

		if (S_OK != m_d3dDevice->CreateBuffer(&bufferDesc, NULL, &m_bufferMatrix))
		{
			break;
		}


		if (!OnResize(m_windowWidth, m_windowHeight))
		{
			break;
		}

		m_viewport =
		{
			0.0f,//FLOAT TopLeftX;
			0.0f,//FLOAT TopLeftY;
			(FLOAT)m_windowWidth,//FLOAT Width;
			(FLOAT)m_windowHeight,//FLOAT Height;
			0.0f,//FLOAT MinDepth;
			1.0f,//FLOAT MaxDepth;
		};


		m_d3dContext->OMSetRenderTargets(1, &m_bbView, nullptr);
		m_d3dContext->RSSetViewports(1, &m_viewport);
		Instance = this;

		if (!m_texPool.CreateDefaultTexture())
			break;
		shaderlib = new ShaderLib();
		return true;
	} while (false);


	Destroy();

	return false;
}

void RenderSystem::Destroy()
{
	m_geometry.Destroy();
	m_texPool.Destroy();
	if (shaderlib)
	{
		delete shaderlib;
		shaderlib = nullptr;
	}
	SR(m_bufferMatrix);
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

const gml::mat44& RenderSystem::GetProjectionMatrix()
{
	if (m_matrixProjDirty)
	{
		m_matrixProjDirty = false;
		float znear = -0.5f;
		m_matProj = gml::mat44::center_ortho_lh(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), znear, 1000.0f);
		m_matProj = gml::mat44::ortho2d_lh(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), znear, 1000.0f);
	}
	return m_matProj;
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

	Texture2D* texture = m_texPool.GetTexture(m_texture);
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
		m_d3dContext->VSSetConstantBuffers(0, 1, &m_bufferMatrix);
		m_d3dContext->PSSetShader(shader->GetPixelShader(), NULL, 0);
		m_d3dContext->PSSetShaderResources(0, 1, &(texture->m_shaderView));
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

void RenderSystem::RenderMesh(g2d::Mesh* m, g2d::Material* material, const gml::mat32& transform)
{
	//::Texture* timpl = dynamic_cast<::Texture*>(t);
	::Pass* p = dynamic_cast<::Pass*>(material->GetPass(0));
	::Texture* timpl = dynamic_cast<::Texture*>(p->GetTextures(0));
	std::string textureName = (timpl == nullptr) ? "" : timpl->GetResourceName();
	if (m_mesh.GetVertexCount() != 0 && textureName != m_texture)
	{
		FlushBatch();
		m_mesh.Clear();
	}

	m_texture = textureName;
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

g2d::Mesh* RenderSystem::CreateMesh(unsigned int vertexCount, unsigned int indexCount)
{
	return new Mesh(vertexCount, indexCount);
}

g2d::Material* RenderSystem::CreateDefaultMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("default"));
	mat->GetPass(0)->SetTexture(0, Texture::Default(), false);
	return mat;
}

g2d::Material* RenderSystem::CreateSimpleTextureMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("simple.texture"));
	mat->GetPass(0)->SetTexture(0, Texture::Default(), false);
	return mat;
}

g2d::Material* RenderSystem::CreateSimpleColorMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("simple.color"));
	return mat;
}