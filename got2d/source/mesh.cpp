#include "render_system.h"

g2d::Mesh::~Mesh() { }

Mesh::Mesh(unsigned int vertexCount, unsigned int indexCount)
	: m_vertices(vertexCount), m_indices(indexCount)
{

}

bool Mesh::Merge(g2d::Mesh* other)
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