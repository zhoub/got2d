#include "render_system.h"

g2d::Mesh* g2d::Mesh::Create(uint32_t vertexCount, uint32_t indexCount)
{
	return new ::Mesh(vertexCount, indexCount);
}

Mesh::Mesh(uint32_t vertexCount, uint32_t indexCount)
	: m_vertices(vertexCount), m_indices(indexCount)
{ }

bool Mesh::Merge(const g2d::Mesh& other, const gml::mat32& transform)
{
	constexpr const int NUM_VERTEX_LIMITED = 32768;
	auto numVertex = GetVertexCount();
	if (numVertex + other.GetVertexCount() > NUM_VERTEX_LIMITED)
	{
		return false;
	}

	auto vertices = other.GetRawVertices();
	for (int i = 0, n = other.GetVertexCount(); i < n; i++)
	{
		m_vertices.push_back(vertices[i]);
		auto& p = m_vertices.back().position;
		p = gml::transform_point(transform, p);
	}

	auto indices = other.GetRawIndices();
	for (int i = 0, n = other.GetIndexCount(); i < n; i++)
	{
		m_indices.push_back(indices[i] + numVertex);
	}
	return true;
}

void Mesh::Clear()
{
	m_vertices.clear();
	m_indices.clear();
}

const g2d::GeometryVertex* Mesh::GetRawVertices() const
{
	return &(m_vertices[0]);
}

g2d::GeometryVertex* Mesh::GetRawVertices()
{
	return &(m_vertices[0]);
}

const uint32_t* Mesh::GetRawIndices() const
{
	return &(m_indices[0]);
}

uint32_t* Mesh::GetRawIndices()
{
	return &(m_indices[0]);
}

uint32_t Mesh::GetVertexCount() const
{
	return static_cast<uint32_t>(m_vertices.size());
}

uint32_t Mesh::GetIndexCount() const
{
	return static_cast<uint32_t>(m_indices.size());
}

void Mesh::ResizeVertexArray(uint32_t vertexCount)
{
	m_vertices.resize(vertexCount);
}

void Mesh::ResizeIndexArray(uint32_t indexCount)
{
	m_indices.resize(indexCount);
}

bool Mesh::Merge(g2d::Mesh* other, const gml::mat32& transform)
{
	ENSURE(other != nullptr);
	return Merge(*other, transform);
}

void Mesh::Release()
{
	delete this;
}

bool Geometry::Create(uint32_t vertexCount, uint32_t indexCount)
{
	if (vertexCount == 0 || indexCount == 0)
		return false;

	m_numVertices = vertexCount;
	m_numIndices = indexCount;

	auto fb = create_fallback([&] { Destroy(); });

	if (!MakeEnoughVertexArray(vertexCount))
	{
		return false;
	}

	if (!MakeEnoughIndexArray(indexCount))
	{
		return false;
	}
	fb.cancel();
	return true;
}

bool Geometry::MakeEnoughVertexArray(uint32_t numVertices)
{
	if (m_numVertices >= numVertices)
	{
		return true;
	}

	auto vertexBuffer = GetRenderSystem()->GetDevice()->CreateBuffer(rhi::BufferBinding::Vertex, rhi::BufferUsage::Dynamic, sizeof(g2d::GeometryVertex) * numVertices);
	if (vertexBuffer == nullptr)
	{
		return  false;
	}
	else
	{
		m_numVertices = numVertices;
		m_vertexBuffer = vertexBuffer;
		return true;
	}
}

bool Geometry::MakeEnoughIndexArray(uint32_t numIndices)
{
	if (m_numIndices >= numIndices)
	{
		return true;
	}

	auto indexBuffer = GetRenderSystem()->GetDevice()->CreateBuffer(rhi::BufferBinding::Index, rhi::BufferUsage::Dynamic, sizeof(uint32_t) * numIndices);
	if (indexBuffer == nullptr)
	{
		return false;
	}
	else
	{
		m_numIndices = numIndices;
		m_indexBuffer = indexBuffer;
		return true;
	}
}

void Geometry::UploadVertices(uint32_t offset, g2d::GeometryVertex* vertices, uint32_t count)
{
	ENSURE(vertices != nullptr && m_vertexBuffer != nullptr);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == GetRenderSystem()->GetContext()->GetRaw()->Map(m_vertexBuffer->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		count = __min(m_numVertices - offset, count);
		g2d::GeometryVertex* data = reinterpret_cast<g2d::GeometryVertex*>(mappedResource.pData);
		memcpy(data + offset, vertices, sizeof(g2d::GeometryVertex) * count);
		GetRenderSystem()->GetContext()->GetRaw()->Unmap(m_vertexBuffer->GetRaw(), 0);
	}
}

void Geometry::UploadIndices(uint32_t offset, uint32_t* indices, uint32_t count)
{
	ENSURE(indices != nullptr && m_indexBuffer != nullptr);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (S_OK == GetRenderSystem()->GetContext()->GetRaw()->Map(m_indexBuffer->GetRaw(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))
	{
		count = __min(m_numIndices - offset, count);
		uint32_t* data = reinterpret_cast<uint32_t*>(mappedResource.pData);
		memcpy(data + offset, indices, sizeof(uint32_t) * count);
		GetRenderSystem()->GetContext()->GetRaw()->Unmap(m_indexBuffer->GetRaw(), 0);
	}
}

void Geometry::Destroy()
{
	m_vertexBuffer.release();
	m_indexBuffer.release();
	m_numVertices = 0;
	m_numIndices = 0;
}
