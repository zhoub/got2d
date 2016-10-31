#include "render_system.h"
#include <string>

g2d::RenderSystem::~RenderSystem() { }
RenderSystem* RenderSystem::Instance = nullptr;

RenderSystem::RenderSystem() :m_bkColor(gml::color4::blue())
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
	return true;
}

bool RenderSystem::CreateBlendModes()
{

	HRESULT hr = S_OK;
	ID3D11BlendState* blendState = nullptr;
	D3D11_BLEND_DESC blendDesc;
	m_blendModes[BLEND_NONE] = nullptr;

	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	hr = m_d3dDevice->CreateBlendState(&blendDesc, &blendState);
	if (hr != S_OK)
		return false;
	m_blendModes[BLEND_NORMAL] = blendState;

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	hr = m_d3dDevice->CreateBlendState(&blendDesc, &blendState);
	if (hr != S_OK)
		return false;
	m_blendModes[BLEND_ADD] = blendState;

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
		HRESULT hr;

		//Create Device
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
		D3D11_CREATE_DEVICE_FLAG deviceFlag = D3D11_CREATE_DEVICE_SINGLETHREADED;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		hr = ::D3D11CreateDevice(NULL, driverType, NULL, deviceFlag, &featureLevel, 1, D3D11_SDK_VERSION, &m_d3dDevice, NULL, &m_d3dContext);
		if (S_OK != hr)
		{
			break;
		}

		IDXGIDevice * dxgiDevice = nullptr;
		hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
		if (S_OK != hr || dxgiDevice == nullptr)
		{
			break;
		}

		IDXGIAdapter * adapter = nullptr;
		hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&adapter);
		if (S_OK != hr || adapter == nullptr)
		{
			break;
		}

		//CreateSwapChain
		IDXGIFactory* factory = nullptr;
		hr = adapter->GetParent(__uuidof(IDXGIFactory), (void **)&factory);
		if (S_OK != hr || factory == nullptr)
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

		hr = factory->CreateSwapChain(m_d3dDevice, &scDesc, &m_swapChain);
		factory->Release();
		if (S_OK != hr)
		{
			break;
		}

		m_swapChain->GetDesc(&scDesc);
		m_windowWidth = scDesc.BufferDesc.Width;
		m_windowHeight = scDesc.BufferDesc.Height;

		D3D11_BUFFER_DESC bufferDesc =
		{
			sizeof(gml::mat32) + sizeof(gml::mat44),	//UINT ByteWidth;
			D3D11_USAGE_DYNAMIC,						//D3D11_USAGE Usage;
			D3D11_BIND_CONSTANT_BUFFER,					//UINT BindFlags;
			D3D11_CPU_ACCESS_WRITE,						//UINT CPUAccessFlags;
			0,											//UINT MiscFlags;
			0											//UINT StructureByteStride;
		};

		hr = m_d3dDevice->CreateBuffer(&bufferDesc, NULL, &m_sceneConstBuffer);
		if (S_OK != hr)
		{
			break;
		}


		if (!OnResize(m_windowWidth, m_windowHeight))
		{
			break;
		}

		if (!CreateBlendModes())
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
		SetBlendMode(BLEND_NORMAL);
		Instance = this;

		//all creation using RenderSystem should be start here.
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
	for (auto& list : m_renderRequests)
	{
		delete list.second;
	}
	m_renderRequests.clear();

	for (auto& blendMode : m_blendModes)
	{
		blendMode.second->Release();
	}
	m_blendModes.clear();

	m_geometry.Destroy();
	m_texPool.Destroy();
	SD(shaderlib);
	SR(m_sceneConstBuffer);
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
		//m_matProj = gml::mat44::center_ortho_lh(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), znear, 1000.0f);
		m_matProj = gml::mat44::ortho2d_lh(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), znear, 1000.0f);
		m_matProjConstBufferDirty = true;
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

void RenderSystem::SetBlendMode(BlendMode blendMode)
{
	if (m_blendModes.count(blendMode) == 0)
		return;

	ID3D11BlendState* blendState = m_blendModes[blendMode];
	m_d3dContext->OMSetBlendState(blendState, nullptr, 0xffffffff);
}
Texture* RenderSystem::CreateTextureFromFile(const char* resPath)
{
	return new Texture(resPath);
}

void RenderSystem::UpdateConstBuffer(ID3D11Buffer* cbuffer, const void* data, unsigned int length)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;
	if (S_OK == m_d3dContext->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData))
	{
		unsigned char*  dstBuffre = reinterpret_cast<unsigned char*>(mappedData.pData);
		memcpy(dstBuffre, data, length);
		m_d3dContext->Unmap(cbuffer, 0);
	}
}

void RenderSystem::UpdateSceneConstBuffer(gml::mat32* matrixView)
{
	if (matrixView == nullptr && !m_matProjConstBufferDirty && !m_matrixProjDirty)
		return;

	m_matProjConstBufferDirty = false;
	D3D11_MAPPED_SUBRESOURCE mappedData;
	if (S_OK == m_d3dContext->Map(m_sceneConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData))
	{
		unsigned char*  dstBuffer = reinterpret_cast<unsigned char*>(mappedData.pData);
		if (matrixView)
		{
			memcpy(dstBuffer, matrixView->m, sizeof(gml::mat32));
		}
		memcpy(dstBuffer + sizeof(gml::mat32), GetProjectionMatrix().m, sizeof(gml::mat44));
		m_d3dContext->Unmap(m_sceneConstBuffer, 0);
	}
}

void RenderSystem::FlushBatch(Mesh& mesh, g2d::Material* material)
{
	if (mesh.GetIndexCount() == 0 || material == nullptr)
		return;

	m_geometry.MakeEnoughVertexArray(mesh.GetVertexCount());
	m_geometry.MakeEnoughIndexArray(mesh.GetIndexCount());
	m_geometry.UploadVertices(0, mesh.GetRawVertices(), mesh.GetVertexCount());
	m_geometry.UploadIndices(0, mesh.GetRawIndices(), mesh.GetIndexCount());

	for (unsigned int i = 0; i < material->GetPassCount(); i++)
	{
		auto pass = material->GetPass(i);
		auto shader = shaderlib->GetShaderByName(pass->GetVertexShaderName(), pass->GetPixelShaderName());
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
			UpdateSceneConstBuffer(nullptr);
			m_d3dContext->VSSetConstantBuffers(0, 1, &m_sceneConstBuffer);

			auto vcb = shader->GetVertexConstBuffer();
			if (vcb)
			{
				auto length = (shader->GetVertexConstBufferLength() > pass->GetVSConstantLength())
					? pass->GetVSConstantLength()
					: shader->GetVertexConstBufferLength();
				if (length > 0)
				{
					UpdateConstBuffer(vcb, pass->GetVSConstant(), length);
					m_d3dContext->VSSetConstantBuffers(1, 1, &vcb);

				}
			}

			auto pcb = shader->GetPixelConstBuffer();
			if (pcb)
			{
				auto length = (shader->GetPixelConstBufferLength() > pass->GetPSConstantLength())
					? pass->GetPSConstantLength()
					: shader->GetPixelConstBufferLength();
				if (length > 0)
				{
					UpdateConstBuffer(pcb, pass->GetPSConstant(), length);
					m_d3dContext->PSSetConstantBuffers(0, 1, &pcb);
				}
			}

			if (pass->GetTextureCount() > 0)
			{
				std::vector<ID3D11ShaderResourceView*> views(pass->GetTextureCount());
				std::vector<ID3D11SamplerState*> samplerstates(pass->GetTextureCount());
				for (unsigned int i = 0; i < pass->GetTextureCount(); i++)
				{
					::Texture* timpl = dynamic_cast<::Texture*>(pass->GetTexture(i));
					std::string textureName = (timpl == nullptr) ? "" : timpl->GetResourceName();
					Texture2D* texture = m_texPool.GetTexture(textureName);
					if (texture)
					{
						views[i] = texture->m_shaderView;
					}
					else
					{
						views[i] = nullptr;
					}
					samplerstates[i] = nullptr;
				}
				UINT numView = static_cast<UINT>(views.size());
				m_d3dContext->PSSetShaderResources(0, numView, &(views[0]));
				m_d3dContext->PSSetSamplers(0, numView, &(samplerstates[0]));
			}

			m_d3dContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	mesh.Clear();
}

void RenderSystem::Render()
{
	if (m_renderRequests.size() == 0)
		return;

	Mesh batchMesh(0, 0);
	g2d::Material* material = nullptr;
	for (auto& reqList : m_renderRequests)
	{
		ReqList& list = *(reqList.second);
		if (list.size() == 0)
			continue;

		for (auto& request : list)
		{
			if (!request.material->IsSame(material))//material may be nullptr.
			{
				FlushBatch(batchMesh, material);
				material = request.material;
			}

			if (!batchMesh.Merge(request.mesh, request.worldMatrix))
			{
				FlushBatch(batchMesh, material);
				//de factor, no need to Merge when there is only ONE MESH each drawcall.
				batchMesh.Merge(request.mesh, request.worldMatrix);
			}
		}
		list.clear();
	}
	FlushBatch(batchMesh, material);
}

void RenderSystem::RenderMesh(unsigned int layer, g2d::Mesh* mesh, g2d::Material* material, const gml::mat32& worldMatrix)
{
	if (mesh == nullptr || material == nullptr)
		return;
	if (m_renderRequests.count(layer) == 0)
	{
		m_renderRequests[layer] = new ReqList();
	}

	ReqList* list = m_renderRequests[layer];
	list->push_back({ mesh, material, worldMatrix });
}

void RenderSystem::BeginRender()
{
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

g2d::Material* RenderSystem::CreateColorTextureMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("default", "color.texture"));
	mat->GetPass(0)->SetTexture(0, Texture::Default(), false);
	return mat;
}

g2d::Material* RenderSystem::CreateSimpleTextureMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("default", "simple.texture"));
	mat->GetPass(0)->SetTexture(0, Texture::Default(), false);
	return mat;
}

g2d::Material* RenderSystem::CreateSimpleColorMaterial()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new Pass("default", "simple.color"));
	return mat;
}