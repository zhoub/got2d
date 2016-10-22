#include "render_system.h"

g2d::Mesh::~Mesh() { }

Mesh::Mesh(unsigned int vertexCount, unsigned int indexCount)
	: m_vertices(vertexCount), m_indices(indexCount)
{

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