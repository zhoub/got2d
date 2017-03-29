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


WorldTransform::WorldTransform(::SceneNode& node, LocalTransform& local)
	: m_sceneNode(node)
	, m_localTransform(local)
	, m_position(0, 0)
	, m_matrix(gml::mat32::identity())
	, m_matrixDirty(true)
	, m_positionDirty(true)
	, m_rightDirty(true)
	, m_upDirty(true)
{
}

const gml::vec2 & WorldTransform::GetPosition()
{
	if (m_positionDirty)
	{
		if (m_sceneNode.ParentIsScene())
		{
			m_position = m_localTransform.GetPosition();
		}
		else
		{
			m_position = gml::transform_point(m_sceneNode.GetParent()->GetWorldMatrix(), m_localTransform.GetPosition());
		}

		m_positionDirty = false;
	}
	return m_position;
}

const gml::vec2 & WorldTransform::GetUp()
{
	if (m_upDirty)
	{
		m_up = gml::transform_vector(GetMatrix(), gml::vec2::up());
		m_upDirty = false;
	}
	return m_up;
}

const gml::vec2 & WorldTransform::GetRight()
{
	if (m_rightDirty)
	{
		m_right = gml::transform_vector(GetMatrix(), gml::vec2::right());
		m_rightDirty = false;
	}
	return m_right;
}

const gml::mat32& WorldTransform::GetMatrix()
{
	if (m_matrixDirty)
	{
		if (m_sceneNode.ParentIsScene())
		{
			m_matrix = m_localTransform.GetMatrix();
			SetPositionDirty();
		}
		else
		{
			auto& matParent = m_sceneNode.GetParent()->GetWorldMatrix();
			m_matrix = matParent * m_localTransform.GetMatrix();
			SetPositionDirty();
		}
		m_matrixDirty = false;
	}
	return m_matrix;
}
