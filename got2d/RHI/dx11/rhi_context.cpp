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

void Context::ClearRenderTarget(rhi::RenderTarget * renderTarget, gml::color4 clearColor)
{
	auto renderTargetImpl = reinterpret_cast<::RenderTarget*>(renderTarget);
	ENSURE(renderTargetImpl != nullptr);

	auto colorBufferCount = renderTargetImpl->GetColorBufferCount();
	for (uint32_t i = 0; i < colorBufferCount; i++)
	{
		m_d3dContext.ClearRenderTargetView(
			renderTargetImpl->GetColorBufferImplByIndex(i)->GetRTView(),
			static_cast<float*>(clearColor));
	}
	
}

void Context::SetViewport(const rhi::Viewport& viewport)
{
	D3D11_VIEWPORT vp11 =
	{
		viewport.LTPosition.x,
		viewport.LTPosition.y,
		viewport.Size.x,
		viewport.Size.y,
		viewport.MinMaxZ.x,
		viewport.MinMaxZ.y
	};
	m_d3dContext.RSSetViewports(1, &vp11);
}
void Context::SetRenderTarget(rhi::RenderTarget* renderTarget)
{
	auto renderTargetImpl = reinterpret_cast<::RenderTarget*>(renderTarget);
	ENSURE(renderTargetImpl != nullptr);

	auto colorBufferCount = renderTargetImpl->GetColorBufferCount();
	if (m_rtViews.size() < colorBufferCount)
	{
		m_rtViews.resize(colorBufferCount);
	}

	for (uint32_t i = 0; i < colorBufferCount; i++)
	{
		m_rtViews[i] = renderTargetImpl->GetColorBufferImplByIndex(i)->GetRTView();
	}

	ID3D11DepthStencilView* dsView = renderTargetImpl->IsDepthStencilUsed() ? 
		renderTargetImpl->GetDepthStencilBufferImpl()->GetDSView() : nullptr;

	m_d3dContext.OMSetRenderTargets(colorBufferCount, &(m_rtViews[0]), dsView);
}

void Context::SetVertexBuffers(uint32_t startSlot, rhi::VertexBufferInfo* buffers, uint32_t bufferCount)
{
	ENSURE(buffers != nullptr);

	if (m_vertexbuffers.size() < bufferCount)
	{
		m_vertexbuffers.resize(bufferCount);
		m_vertexBufferStrides.resize(bufferCount);
		m_vertexBufferOffsets.resize(bufferCount);
	}

	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		const rhi::VertexBufferInfo& info = buffers[i];
		bufferImpl = reinterpret_cast<::Buffer*>(info.buffer);
		m_vertexbuffers[i] = bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw();
		m_vertexBufferStrides[i] = info.stride;
		m_vertexBufferOffsets[i] = info.offset;
	}

	m_d3dContext.IASetVertexBuffers(startSlot, bufferCount,
		&(m_vertexbuffers[0]),
		&(m_vertexBufferStrides[0]),
		&(m_vertexBufferOffsets[0]));
}

void Context::SetIndexBuffer(rhi::Buffer* buffer, uint32_t offset, rhi::IndexFormat format)
{
	auto bufferImpl = reinterpret_cast<::Buffer*>(buffer);

	ID3D11Buffer* indexBuffer = bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw();
	m_d3dContext.IASetIndexBuffer(indexBuffer, kIndexFormat[(int)format], offset);
}

void Context::SetVertexShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	ENSURE(buffers != nullptr);

	if (m_vsConstantBuffers.size() < bufferCount)
	{
		m_vsConstantBuffers.resize(bufferCount);
	}

	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		bufferImpl = reinterpret_cast<::Buffer*>(buffers[i]);
		m_vsConstantBuffers[i] = bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw();
	}
	m_d3dContext.VSSetConstantBuffers(startSlot, bufferCount, &(m_vsConstantBuffers[0]));
}

void Context::SetPixelShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount)
{
	ENSURE(buffers != nullptr);

	if (m_psConstantBuffers.size() < bufferCount)
	{
		m_psConstantBuffers.resize(bufferCount);
	}

	::Buffer* bufferImpl = nullptr;
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		bufferImpl = reinterpret_cast<::Buffer*>(buffers[i]);
		m_psConstantBuffers[i] = bufferImpl == nullptr ? nullptr : bufferImpl->GetRaw();
	}
	m_d3dContext.PSSetConstantBuffers(startSlot, bufferCount, &(m_psConstantBuffers[0]));
}

void Context::SetShaderProgram(rhi::ShaderProgram * program)
{
	auto programImpl = reinterpret_cast<::ShaderProgram*>(program);
	ENSURE(programImpl != nullptr);

	m_d3dContext.IASetInputLayout(programImpl->GetInputLayout());
	m_d3dContext.VSSetShader(programImpl->GetVertexShader(), nullptr, 0);
	m_d3dContext.PSSetShader(programImpl->GetPixelShader(), nullptr, 0);
}

void Context::SetShaderResources(uint32_t startSlot, rhi::ShaderResourceView** srViews, uint32_t resCount)
{
	ENSURE(srViews != nullptr);

	if (m_srViews.size() < resCount)
	{
		m_srViews.resize(resCount);
	}

	::ShaderResourceView* srViewImpl = nullptr;
	for (uint32_t i = 0; i < resCount; i++)
	{
		srViewImpl = reinterpret_cast<::ShaderResourceView*>(srViews[i]);
		m_srViews[i] = srViewImpl == nullptr ? nullptr : srViewImpl->GetRaw();
	}
	m_d3dContext.PSSetShaderResources(startSlot, resCount, &(m_srViews[0]));
}

void Context::SetBlendState(rhi::BlendState* state)
{
	auto stateImpl = reinterpret_cast<::BlendState*>(state);
	ID3D11BlendState* blendState = stateImpl == nullptr ? nullptr : stateImpl->GetRaw();
	m_d3dContext.OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
}

void Context::SetTextureSampler(uint32_t startSlot, rhi::TextureSampler** samplers, uint32_t count)
{
	ENSURE(samplers != nullptr);

	if (m_samplerStates.size() < count)
	{
		m_samplerStates.resize(count);
	}

	::TextureSampler* samplerImpl = nullptr;
	for (uint32_t i = 0; i < count; i++)
	{
		samplerImpl = reinterpret_cast<::TextureSampler*>(samplers[i]);
		m_samplerStates[i] = samplerImpl == nullptr ? nullptr : samplerImpl->GetRaw();
	}
	m_d3dContext.PSSetSamplers(startSlot, count, &(m_samplerStates[0]));
}

void Context::DrawIndexed(rhi::Primitive primitive, uint32_t startIndex, uint32_t indexOffset, uint32_t baseVertex)
{
	m_d3dContext.IASetPrimitiveTopology(kPrimitive[(int)primitive]);
	m_d3dContext.DrawIndexed(startIndex, indexOffset, baseVertex);
}

rhi::MappedResource Context::Map(rhi::Buffer* buffer)
{
	auto bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	return Map(bufferImpl->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0);
}

rhi::MappedResource Context::Map(rhi::Texture2D * buffer)
{
	auto textureImpl = reinterpret_cast<::Texture2D*>(buffer);
	ENSURE(textureImpl != nullptr);

	return Map(textureImpl->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0);
}

void Context::Unmap(rhi::Buffer* buffer)
{
	auto bufferImpl = reinterpret_cast<::Buffer*>(buffer);
	ENSURE(bufferImpl != nullptr);

	Unmap(bufferImpl->GetRaw(), 0);
}

void Context::Unmap(rhi::Texture2D* buffer)
{
	auto textureImpl = reinterpret_cast<::Texture2D*>(buffer);
	ENSURE(textureImpl != nullptr);

	Unmap(textureImpl->GetRaw(), 0);
}

rhi::MappedResource Context::Map(ID3D11Resource* resource, UINT subResource, D3D11_MAP mappingType, UINT flag)
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

void Context::Unmap(ID3D11Resource* resource, UINT subResource)
{
	m_d3dContext.Unmap(resource, subResource);
}

void Context::GenerateMipmaps(rhi::ShaderResourceView* srView)
{
	auto srViewImpl = reinterpret_cast<::ShaderResourceView*>(srView);
	ENSURE(srViewImpl != nullptr);
	m_d3dContext.GenerateMips(srViewImpl->GetRaw());
}
