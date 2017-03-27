#include "scene.h"

void LocalTransform::SetPivot(const gml::vec2& pivot)
{
	SetMatrixDirty();
	m_pivot = pivot;
}

void LocalTransform::SetScale(const gml::vec2& scale)
{
	SetMatrixDirty();
	m_scale = scale;
}

LocalTransform::LocalTransform()
	: m_position(0, 0)
	, m_pivot(0, 0)
	, m_scale(1, 1)
	, m_rotation(0)
	, m_matrix(gml::mat32::identity())
	, m_matrixDirty(true)
{
}

void LocalTransform::SetPosition(const gml::vec2& position)
{
	SetMatrixDirty();
	m_position = position;
}

void LocalTransform::SetRotation(gml::radian r)
{
	SetMatrixDirty();
	m_rotation = r;
}

const gml::mat32& LocalTransform::GetMatrix()
{
	if (m_matrixDirty)
	{
		m_matrix = gml::mat32::trsp(m_position, m_rotation, m_scale, m_pivot);
		m_matrixDirty = false;
	}
	return m_matrix;
}

void LocalTransform::SetMatrixDirty()
{
	m_matrixDirty = true;
}