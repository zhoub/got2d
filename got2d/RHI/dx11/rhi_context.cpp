#include "inner_RHI.h"
#include "../source/inner_utility.h"

Context::Context(ID3D11DeviceContext& d3dContext)
	: m_d3dContext(d3dContext)
{
}

Context::~Context()
{
	m_d3dContext.Release();
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
