#include <string>
#include "render_system.h"

RenderSystem* RenderSystem::Instance = nullptr;

bool RenderSystem::OnResize(uint32_t width, uint32_t height)
{
	//though we create an individual render target
	//we do not use it for rendering, for now.
	//it will be used when building Compositor System.
	m_colorTexture.release();
	m_rtView.release();
	m_bbView.release();

	autor<rhi::Texture2D> colorTexture = nullptr;
	autor<ID3D11RenderTargetView> rtView = nullptr;
	autor<ID3D11RenderTargetView> bbView = nullptr;

	if (!m_swapChain->ResizeBackBuffer(width, height))
	{
		return false;
	}

	//CreateRenderTarget and Views.
	colorTexture = m_device->CreateTexture2D(
		rhi::TextureFormat::BGRA,
		rhi::ResourceUsage::Default,
		rhi::TextureBinding::RenderTarget | rhi::TextureBinding::ShaderResource,
		width, height);
	if (colorTexture.is_null())
	{
		return false;
	}

	if (S_OK != m_device->GetRaw()->CreateRenderTargetView(colorTexture->GetRaw(), NULL, &(rtView.pointer)))
	{
		return false;
	}
	ENSURE(rtView.is_not_null());

	autor<rhi::Texture2D> backBuffer = m_swapChain->GetBackBuffer();
	if (backBuffer.is_null())
	{
		return false;
	}

	if (S_OK != m_device->GetRaw()->CreateRenderTargetView(backBuffer->GetRaw(), NULL, &(bbView.pointer)))
	{
		return false;
	}
	ENSURE(bbView.is_not_null());

	m_colorTexture = std::move(colorTexture);
	m_rtView = std::move(rtView);
	m_bbView = std::move(bbView);

	m_matrixProjDirty = true;
	m_matrixConstBufferDirty = true;
	m_windowWidth = width;
	m_windowHeight = height;
	m_viewport =
	{
		0.0f,					//FLOAT TopLeftX;
		0.0f,					//FLOAT TopLeftY;
		(FLOAT)m_windowWidth,	//FLOAT Width;
		(FLOAT)m_windowHeight,	//FLOAT Height;
		0.0f,					//FLOAT MinDepth;
		1.0f,					//FLOAT MaxDepth;
	};

	m_context->GetRaw()->OMSetRenderTargets(1, &(m_bbView.pointer), NULL);
	m_context->GetRaw()->RSSetViewports(1, &m_viewport);

	return true;
}

bool RenderSystem::CreateBlendModes()
{

	HRESULT hr = S_OK;
	ID3D11BlendState* blendState = nullptr;
	D3D11_BLEND_DESC blendDesc;
	m_blendModes[g2d::BlendMode::None] = nullptr;

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

	hr = m_device->GetRaw()->CreateBlendState(&blendDesc, &blendState);
	if (hr != S_OK)
		return false;
	m_blendModes[g2d::BlendMode::Normal] = blendState;

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

	hr = m_device->GetRaw()->CreateBlendState(&blendDesc, &blendState);
	if (hr != S_OK)
		return false;
	m_blendModes[g2d::BlendMode::Additve] = blendState;

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		blendDesc.RenderTarget[i].BlendEnable = TRUE;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	return true;
}

bool RenderSystem::Create(void* nativeWindow)
{
	if (Instance)
	{
		return Instance == this;
	}

	auto fb = create_fallback([&] { Destroy(); });

	HRESULT hr = S_OK;
	auto rhiResult = rhi::CreateRHI();

	if (!rhiResult.success)
	{
		return false;
	}

	m_device = rhiResult.device;
	m_context = rhiResult.context;

	auto swapChain = m_device->CreateSwapChain(nativeWindow, m_windowWidth, m_windowHeight);
	if (swapChain == nullptr)
	{
		return false;
	}
	m_swapChain = swapChain;
	m_windowWidth = m_swapChain->GetWidth();
	m_windowHeight = m_swapChain->GetHeight();

	m_sceneConstBuffer = m_device->CreateBuffer(rhi::BufferBinding::Constant, rhi::ResourceUsage::Dynamic, sizeof(gml::vec4) * 6);
	if (m_sceneConstBuffer.is_null())
	{
		return false;
	}

	if (!OnResize(m_windowWidth, m_windowHeight))
	{
		return false;
	}

	if (!CreateBlendModes())
	{
		return false;
	}


	SetBlendMode(g2d::BlendMode::None);
	Instance = this;

	//all creation using RenderSystem should be start here.
	if (!m_texPool.CreateDefaultTexture())
		return false;

	m_shaderlib = new ShaderLib();
	fb.cancel();
	return true;
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
		if (blendMode.second)
		{
			blendMode.second->Release();
		}
	}
	m_blendModes.clear();

	m_geometry.Destroy();
	m_texPool.Destroy();

	m_sceneConstBuffer.release();
	m_swapChain.release();
	m_device.release();
	m_context.release();
	m_colorTexture.release();
	m_rtView.release();
	m_bbView.release();

	if (Instance == this)
	{
		Instance = nullptr;
	}
}

void RenderSystem::SetViewMatrix(const gml::mat32& viewMatrix)
{
	if (viewMatrix != m_matView)
	{
		m_matView = viewMatrix;
		m_matrixConstBufferDirty = true;
	}
}

const gml::mat44& RenderSystem::GetProjectionMatrix()
{
	if (m_matrixProjDirty)
	{
		m_matrixProjDirty = false;
		float znear = -1.0f;
		m_matProj = gml::mat44::ortho2d_lh(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight), znear, 1000.0f);
	}
	return m_matProj;
}

void RenderSystem::Clear()
{
	m_context->GetRaw()->ClearRenderTargetView(m_bbView, static_cast<float*>(m_bkColor));
}

void RenderSystem::Present()
{
	m_swapChain->Present();
}

void RenderSystem::SetBlendMode(g2d::BlendMode blendMode)
{
	if (m_blendModes.count(blendMode) == 0)
		return;

	ID3D11BlendState* blendState = m_blendModes[blendMode];
	m_context->GetRaw()->OMSetBlendState(blendState, nullptr, 0xffffffff);
}

Texture* RenderSystem::CreateTextureFromFile(const char* resPath)
{
	return new Texture(resPath);
}

void RenderSystem::UpdateConstBuffer(rhi::Buffer* cbuffer, const void* data, uint32_t length)
{
	auto mappedData = m_context->Map(cbuffer);
	if (mappedData.success)
	{
		uint8_t*  dstBuffre = reinterpret_cast<uint8_t*>(mappedData.data);
		memcpy(dstBuffre, data, length);
		m_context->Unmap(cbuffer);
	}
}

void RenderSystem::UpdateSceneConstBuffer()
{
	if (!m_matrixConstBufferDirty)
		return;

	m_matrixConstBufferDirty = false;
	auto mappedData = m_context->Map(m_sceneConstBuffer);
	if (mappedData.success)
	{
		uint8_t*  dstBuffer = reinterpret_cast<uint8_t*>(mappedData.data);
		memcpy(dstBuffer, &(m_matView.row[0]), sizeof(gml::vec3));
		memcpy(dstBuffer + sizeof(gml::vec4), &(m_matView.row[1]), sizeof(gml::vec3));
		memcpy(dstBuffer + sizeof(gml::vec4) * 2, GetProjectionMatrix().m, sizeof(gml::mat44));
		m_context->Unmap(m_sceneConstBuffer);
	}
}

void RenderSystem::FlushBatch(Mesh& mesh, g2d::Material& material)
{
	if (mesh.GetIndexCount() == 0)
		return;

	m_geometry.MakeEnoughVertexArray(mesh.GetVertexCount());
	m_geometry.MakeEnoughIndexArray(mesh.GetIndexCount());
	m_geometry.UploadVertices(0, mesh.GetRawVertices(), mesh.GetVertexCount());
	m_geometry.UploadIndices(0, mesh.GetRawIndices(), mesh.GetIndexCount());

	for (uint32_t i = 0; i < material.GetPassCount(); i++)
	{
		auto pass = material.GetPassByIndex(i);
		auto shader = m_shaderlib->GetShaderByName(pass->GetVertexShaderName(), pass->GetPixelShaderName());
		if (shader)
		{
			rhi::VertexBufferInfo info;
			info.stride = sizeof(g2d::GeometryVertex);
			info.offset = 0;
			info.buffer = m_geometry.m_vertexBuffer;
			m_context->SetVertexBuffers(0, &info, 1);
			m_context->SetIndexBuffer(m_geometry.m_indexBuffer, 0, rhi::IndexFormat::Int32);
			m_context->GetRaw()->IASetInputLayout(shader->GetInputLayout());
			m_context->GetRaw()->VSSetShader(shader->GetVertexShader(), NULL, 0);
			m_context->GetRaw()->PSSetShader(shader->GetPixelShader(), NULL, 0);
			UpdateSceneConstBuffer();
			m_context->SetVertexShaderConstantBuffers(0, &(m_sceneConstBuffer.pointer), 1);
			SetBlendMode(pass->GetBlendMode());

			auto vcb = shader->GetVertexConstBuffer();
			if (vcb)
			{
				auto length = (shader->GetVertexConstBuffer()->GetLength() > pass->GetVSConstantLength())
					? pass->GetVSConstantLength()
					: shader->GetVertexConstBuffer()->GetLength();
				if (length > 0)
				{
					UpdateConstBuffer(vcb, pass->GetVSConstant(), length);
					m_context->SetVertexShaderConstantBuffers(1, &vcb, 1);
				}
			}

			auto pcb = shader->GetPixelConstBuffer();
			if (pcb)
			{
				auto length = (shader->GetPixelConstBuffer()->GetLength() > pass->GetPSConstantLength())
					? pass->GetPSConstantLength()
					: shader->GetPixelConstBuffer()->GetLength();
				if (length > 0)
				{
					UpdateConstBuffer(pcb, pass->GetPSConstant(), length);
					m_context->SetPixelShaderConstantBuffers(0, &pcb, 1);
				}
			}

			if (pass->GetTextureCount() > 0)
			{
				std::vector<ID3D11ShaderResourceView*> views(pass->GetTextureCount());
				std::vector<ID3D11SamplerState*> samplerstates(pass->GetTextureCount());
				for (uint32_t t = 0; t < pass->GetTextureCount(); t++)
				{
					::Texture* timpl = reinterpret_cast<::Texture*>(pass->GetTextureByIndex(t));
					std::string textureName = (timpl == nullptr) ? "" : timpl->GetResourceName();
					auto texture = m_texPool.GetTexture(textureName);
					if (texture)
					{
						views[t] = texture->m_shaderView;
					}
					else
					{
						views[t] = nullptr;
					}
					samplerstates[t] = nullptr;
				}
				UINT numView = static_cast<UINT>(views.size());
				m_context->GetRaw()->PSSetShaderResources(0, numView, &(views[0]));
				m_context->GetRaw()->PSSetSamplers(0, numView, &(samplerstates[0]));
			}

			m_context->DrawIndexed(rhi::Primitive::TriangleList, mesh.GetIndexCount(), 0, 0);
		}
	}

	mesh.Clear();
}

void RenderSystem::FlushRequests()
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
			if (material == nullptr)
			{
				material = &(request.material);
			}
			else if (!request.material.IsSame(material))//material may be nullptr.
			{
				FlushBatch(batchMesh, *material);
				material = &(request.material);
			}

			if (!batchMesh.Merge(request.mesh, request.worldMatrix))
			{
				FlushBatch(batchMesh, *material);
				//de factor, no need to Merge when there is only ONE MESH each drawcall.
				batchMesh.Merge(request.mesh, request.worldMatrix);
			}
		}
		list.clear();
	}
	if (material != nullptr)
	{
		FlushBatch(batchMesh, *material);
	}
}

void RenderSystem::RenderMesh(uint32_t layer, g2d::Mesh* mesh, g2d::Material* material, const gml::mat32& worldMatrix)
{
	if (m_renderRequests.count(layer) == 0)
	{
		m_renderRequests[layer] = new ReqList();
	}
	ReqList* list = m_renderRequests[layer];
	list->push_back({ *mesh, *material, worldMatrix });
}

gml::vec2 RenderSystem::ScreenToView(const gml::coord& screen) const
{
	int wWidth = static_cast<int>(GetWindowWidth());
	int wHeight = static_cast<int>(GetWindowHeight());
	float x = screen.x - wWidth * 0.5f;
	float y = wHeight* 0.5f - screen.y;
	return { x, y };
}
gml::coord RenderSystem::ViewToScreen(const gml::vec2 & view) const
{
	int wWidth = static_cast<int>(GetWindowWidth());
	int wHeight = static_cast<int>(GetWindowHeight());
	int x = static_cast<int>(round(view.x + wWidth * 0.5f));
	int y = static_cast<int>(round(wHeight* 0.5f - view.y));
	return { x, y };
}

void RenderSystem::BeginRender()
{
	Clear();
}

void RenderSystem::EndRender()
{
	FlushRequests();
	Present();
}
