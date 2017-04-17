#include "inner_RHI.h"
#include "../source/scope_utility.h"
#include "dx11_enum.h"

Context::Context(ID3D11DeviceContext& d3dContext)
	: m_d3dContext(d3dContext)
{
}

Context::~Context()
{
	m_d3dContext.Release();
}

void Context::ClearRenderTargetView(rhi::RenderTargetView* rtView, gml::color4 clearColor)
{
	::RenderTargetView* rtViewImpl = reinterpret_cast<::RenderTargetView*>(rtView);
	ENSURE(rtViewImpl != nullptr);
	m_d3dContext.ClearRenderTargetView(rtViewImpl->GetRaw(), static_cast<float*>(clearColor));
}

void Context::SetViewport(const rhi::Viewport* viewport, uint32_t count)
{
	m_viewports.clear();
	for (uint32_t i = 0; i < count; i++)
	{
		m_viewports.push_back(
		{
			viewport[i].LTPosition.x,
			viewport[i].LTPosition.y,
			viewport[i].Size.x,
			viewport[i].Size.y,
			viewport[i].MinMaxZ.x,
			viewport[i].MinMaxZ.y
		});
	}

	m_d3dContext.RSSetViewports(m_viewports.size(), &(m_viewports[0]));
}

void Context::SetRenderTargets(uint32_t rtCount, rhi::RenderTargetView ** renderTargets, rhi::DepthStencilView * dsView)
{
	m_rtViews.clear();
	::RenderTargetView* rtViewImpl = nullptr;
	::DepthStencilView* dsViewImpl = reinterpret_cast<::DepthStencilView*>(dsView);
	for (uint32_t i = 0; i < rtCount; i++)
	{
		rtViewImpl = reinterpret_cast<::RenderTargetView*>(renderTargets[i]);
		m_rtViews.push_back(rtViewImpl == nullptr ? nullptr : rtViewImpl->GetRaw());
	}
	m_d3dContext.OMSetRenderTargets(rtCount, &(m_rtViews[0]), dsViewImpl ? dsViewImpl->GetRaw() : nullptr);
}

void Context::SetVertexBuffers(uint32_t startSlot, rhi::VertexBufferInfo * buffers, uint32_t bufferCount)
{
	m_vertexbuffers.clear();
	m_vertexBufferStrides.clear();
	m_vertexBufferOffsets.clear();
	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		const rhi::VertexBufferInfo& info = buffers[i];
		bufferImpl = reinterpret_cast<::Buffer*>(info.buffer);
		m_vertexbuffers.push_back(bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw());
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
	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);

	ID3D11Buffer* indexBuffer = bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw();
	m_d3dContext.IASetIndexBuffer(indexBuffer, kIndexFormat[(int)format], offset);
}

void Context::SetVertexShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	m_vsConstantBuffers.clear();
	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		bufferImpl = reinterpret_cast<::Buffer*>(buffers[i]);
		if (bufferImpl == nullptr)
		{
			m_vsConstantBuffers.push_back(nullptr);
		}
		else
		{
			m_vsConstantBuffers.push_back(bufferImpl->GetRaw());
		}
	}
	m_d3dContext.VSSetConstantBuffers(startSlot, bufferCount, &(m_vsConstantBuffers[0]));
}

void Context::SetPixelShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	m_psConstantBuffers.clear();
	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		bufferImpl = reinterpret_cast<::Buffer*>(buffers[i]);
		if (bufferImpl == nullptr)
		{
			m_srViews.push_back(nullptr);
		}
		else
		{
			m_psConstantBuffers.push_back(bufferImpl->GetRaw());
		}
	}
	m_d3dContext.PSSetConstantBuffers(startSlot, bufferCount, &(m_psConstantBuffers[0]));
}

void Context::SetShaderProgram(rhi::ShaderProgram * program)
{
	::ShaderProgram* programImpl = reinterpret_cast<::ShaderProgram*>(program);

	m_d3dContext.IASetInputLayout(programImpl->GetInputLayout());
	m_d3dContext.VSSetShader(programImpl->GetVertexShader(), nullptr, 0);
	m_d3dContext.PSSetShader(programImpl->GetPixelShader(), nullptr, 0);
}

void Context::SetShaderResources(uint32_t startSlot, rhi::ShaderResourceView ** srViews, uint32_t viewCount)
{
	m_srViews.clear();
	::ShaderResourceView* srViewImpl = nullptr;
	for (uint32_t i = 0; i < viewCount; i++)
	{
		srViewImpl = reinterpret_cast<::ShaderResourceView*>(srViews[i]);
		if (srViewImpl == nullptr)
		{
			m_srViews.push_back(nullptr);
		}
		else
		{
			m_srViews.push_back(srViewImpl->GetRaw());
		}
	}
	m_d3dContext.PSSetShaderResources(startSlot, viewCount, &(m_srViews[0]));
}

void Context::SetBlendState(rhi::BlendState * state)
{
	::BlendState* stateImpl = reinterpret_cast<::BlendState*>(state);
	ID3D11BlendState* blendState = stateImpl == nullptr ? nullptr : stateImpl->GetRaw();
	m_d3dContext.OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
}

void Context::SetTextureSampler(uint32_t startSlot, rhi::TextureSampler ** samplers, uint32_t count)
{
	m_samplerStates.clear();
	::TextureSampler* samplerImpl = nullptr;
	for (uint32_t i = 0; i < count; i++)
	{
		samplerImpl = reinterpret_cast<::TextureSampler*>(samplers[i]);
		if (samplerImpl == nullptr)
		{
			m_samplerStates.push_back(nullptr);
		}
		else
		{
			m_samplerStates.push_back(samplerImpl->GetRaw());
		}
	}
	m_d3dContext.PSSetSamplers(0, m_samplerStates.size(), &(m_samplerStates[0]));
}

void Context::DrawIndexed(rhi::Primitive primitive, uint32_t startIndex, uint32_t indexOffset, uint32_t baseVertex)
{
	m_d3dContext.IASetPrimitiveTopology(kPrimitive[(int)primitive]);
	m_d3dContext.DrawIndexed(startIndex, indexOffset, baseVertex);
}

rhi::MappedResource Context::Map(rhi::Buffer * buffer)
{
	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	return Map(bufferImpl->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0);
}

rhi::MappedResource Context::Map(rhi::Texture2D * buffer)
{
	::Texture2D* textureImpl = reinterpret_cast<::Texture2D*>(buffer);
	ENSURE(textureImpl != nullptr);

	return Map(textureImpl->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0);
}

void Context::Unmap(rhi::Buffer* buffer)
{
	::Buffer* bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	Unmap(bufferImpl->GetRaw(), 0);
}

void Context::Unmap(rhi::Texture2D * buffer)
{
	::Texture2D* textureImpl = reinterpret_cast<::Texture2D*>(buffer);
	ENSURE(textureImpl != nullptr);

	Unmap(textureImpl->GetRaw(), 0);
}

rhi::MappedResource Context::Map(ID3D11Resource * resource, UINT subResource, D3D11_MAP mappingType, UINT flag)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedRes;
	rhi::MappedResource mappedRes;
	if (S_OK == m_d3dContext.Map(resource, subResource, mappingType, flag, &d3dMappedRes))
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

void Context::Unmap(ID3D11Resource * resource, UINT subResource)
{
	m_d3dContext.Unmap(resource, subResource);
}

void Context::GenerateMipmaps(rhi::ShaderResourceView * srView)
{
	::ShaderResourceView* srViewImpl = reinterpret_cast<::ShaderResourceView*>(srView);
	ENSURE(srViewImpl != nullptr);
	m_d3dContext.GenerateMips(srViewImpl->GetRaw());
}
