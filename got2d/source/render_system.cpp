#include <string>
#include "render_system.h"

RenderSystem* RenderSystem::Instance = nullptr;

bool RenderSystem::OnResize(uint32_t width, uint32_t height)
{
	if (!m_swapChain->OnResize(width, height))
	{
		return false;
	}

	//though we create an individual render target
	//we do not use it for rendering, for now.
	//it will be used when building Compositor System.
	auto rtFormat = rhi::TextureFormat::BGRA;
	autor<rhi::RenderTarget> renderTarget = m_device->CreateRenderTarget(width, height, &rtFormat, 1, false);
	if (renderTarget.is_null())
	{
		return false;
	}

	m_renderTarget = std::move(renderTarget);
	m_backBufferRT = m_swapChain->GetBackBuffer();

	m_matrixProjDirty = true;
	m_matrixConstBufferDirty = true;

	m_viewport.Size.set(
		(float)width,
		(float)height);

	m_context->SetRenderTarget(m_backBufferRT);
	m_context->SetViewport(m_viewport);

	return true;
}

bool RenderSystem::CreateBlendModes()
{
	m_blendModes[g2d::BlendMode::None] = nullptr;

	m_blendModes[g2d::BlendMode::Normal] = m_device->CreateBlendState(true,
		rhi::BlendFactor::SourceAlpha,
		rhi::BlendFactor::InverseSourceAlpha,
		rhi::BlendOperator::Add);

	if (m_blendModes[g2d::BlendMode::Normal] == nullptr)
		return false;

	m_blendModes[g2d::BlendMode::Additve] = m_device->CreateBlendState(true,
		rhi::BlendFactor::One,
		rhi::BlendFactor::One,
		rhi::BlendOperator::Add);

	if (m_blendModes[g2d::BlendMode::Additve] == nullptr)
		return false;

	return true;
}

bool RenderSystem::Create(void* nativeWindow)
{
	m_viewport.LTPosition.set(0, 0);
	m_viewport.MinMaxZ.set(0, 1);

	if (Instance)
	{
		return Instance == this;
	}

	auto fb = create_fallback([&] { Destroy(); });

	auto rhiResult = rhi::CreateRHI();
	if (!rhiResult.Success)
	{
		return false;
	}

	m_device = rhiResult.DevicePtr;
	m_context = rhiResult.ContextPtr;

	m_swapChain = m_device->CreateSwapChain(nativeWindow, false, 0, 0);
	if (m_swapChain.is_null())
	{
		return false;
	}

	m_sceneConstBuffer = m_device->CreateBuffer(rhi::BufferBinding::Constant, rhi::ResourceUsage::Dynamic, sizeof(gml::vec4) * 6);
	if (m_sceneConstBuffer.is_null())
	{
		return false;
	}

	if (!OnResize(m_swapChain->GetWidth(), m_swapChain->GetHeight()))
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
	{
		return false;
	}

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

	m_backBufferRT = nullptr;
	m_shaderlib.release();
	m_sceneConstBuffer.release();
	m_renderTarget.release();
	m_swapChain.release();
	m_device.release();
	m_context.release();

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
		m_matProj = gml::mat44::ortho2d_lh(
			static_cast<float>(GetWindowWidth()),
			static_cast<float>(GetWindowHeight()),
			znear, 1000.0f);
	}
	return m_matProj;
}

void RenderSystem::Clear()
{
	m_context->ClearRenderTarget(m_backBufferRT, m_bkColor);
}

void RenderSystem::Present()
{
	m_swapChain->Present();
}

void RenderSystem::SetBlendMode(g2d::BlendMode blendMode)
{
	if (m_blendModes.count(blendMode) == 0)
		return;

	m_context->SetBlendState(m_blendModes[blendMode]);
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
		auto dstBuffre = reinterpret_cast<uint8_t*>(mappedData.data);
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
		auto dstBuffer = reinterpret_cast<uint8_t*>(mappedData.data);
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
			m_context->SetShaderProgram(shader->GetShaderProgram());
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
				if (m_textures.size() < pass->GetTextureCount())
				{
					m_textures.resize(pass->GetTextureCount());
					m_textureSamplers.resize(pass->GetTextureCount());
				}
				for (uint32_t t = 0; t < pass->GetTextureCount(); t++)
				{
					auto timpl = reinterpret_cast<::Texture*>(pass->GetTextureByIndex(t));
					if (timpl != nullptr)
					{
						auto texture = m_texPool.GetTexture(timpl->GetResourceName());
						if (texture && texture->m_texture->IsShaderResource())
						{
							m_textures[t] = texture->m_texture;
						}
						else
						{
							m_textures[t] = m_texPool.GetDefaultTexture().m_texture;
						}
					}
					else
					{
						m_textures[t] = m_texPool.GetDefaultTexture().m_texture;
					}
					m_textureSamplers[t] = nullptr;
				}

				m_context->SetTextures(0, &(m_textures[0]), pass->GetTextureCount());
				m_context->SetTextureSampler(0, &(m_textureSamplers[0]), pass->GetTextureCount());
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
