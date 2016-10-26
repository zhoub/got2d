#include "render_system.h"

g2d::Mesh::~Mesh() { }

Mesh::Mesh(unsigned int vertexCount, unsigned int indexCount)
	: m_vertices(vertexCount), m_indices(indexCount)
{

}

bool Mesh::Merge(g2d::Mesh* other, const gml::mat32& transform)
{
	constexpr const int NUM_VERTEX_LIMITED = 32768;
	auto numVertex = GetVertexCount();
	if (numVertex + other->GetVertexCount() > NUM_VERTEX_LIMITED)
	{
		return false;
	}

	auto vertices = other->GetRawVertices();
	for (int i = 0, n = other->GetVertexCount(); i < n; i++)
	{
		m_vertices.push_back(vertices[i]);
		auto& p = m_vertices.back().position;
		p = gml::transform_point(transform, p);
	}
	auto indices = other->GetRawIndices();
	for (int i = 0, n = other->GetIndexCount(); i < n; i++)
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

g2d::GeometryVertex* Mesh::GetRawVertices()
{
	return &(m_vertices[0]);
}

unsigned int* Mesh::GetRawIndices()
{
	return &(m_indices[0]);
}

unsigned int Mesh::GetVertexCount()
{
	return static_cast<unsigned int>(m_vertices.size());
}

unsigned int Mesh::GetIndexCount()
{
	return static_cast<unsigned int>(m_indices.size());
}

void Mesh::ResizeVertexArray(unsigned int vertexCount)
{
	m_vertices.resize(vertexCount);
}

void Mesh::ResizeIndexArray(unsigned int indexCount)
{
	m_indices.resize(indexCount);
}

void Mesh::Release()
{
	delete this;
}


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
