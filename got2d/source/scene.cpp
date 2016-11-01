#include "scene.h"

g2d::SceneNode::~SceneNode() { }

g2d::Scene::~Scene() { }

SceneNode::SceneNode(SceneNode* parent, g2d::Entity* entity, bool autoRelease)
	: m_parent(parent)
	, m_entity(entity)
	, m_autoRelease(autoRelease)
	, m_matrixLocal(gml::mat32::I())
	, m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotationRadian(0)
	, m_isVisible(true)
{
	if (m_entity)
	{
		m_entity->SetSceneNode(this);
		m_entity->OnInitial();
	}
}

SceneNode::~SceneNode()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
	if (m_autoRelease && m_entity)
	{
		m_entity->Release();
	}
}

const gml::mat32& SceneNode::GetLocalMatrix()
{
	if (m_matrixLocalDirty)
	{
		m_matrixLocal = gml::mat32::trsp(m_position, m_rotationRadian, m_scale, m_pivot);
		m_matrixLocalDirty = false;
	}
	return m_matrixLocal;
}

const gml::mat32& SceneNode::GetWorldMatrix()
{
	if (m_parent == nullptr)//get rid of transform of ROOT.
	{
		return GetLocalMatrix();
	}

	if (m_matrixWorldDirty)
	{
		auto matParent = m_parent->GetWorldMatrix();
		m_matrixWorld = matParent * GetLocalMatrix();
		m_matrixWorldDirty = false;
	}
	return m_matrixWorld;
}

void SceneNode::SetWorldMatrixDirty()
{
	m_matrixWorldDirty = true;
	for (auto& child : m_children)
	{
		child->SetWorldMatrixDirty();
	}
	m_matrixDirtyUpdate = true;
}

void SceneNode::SetLocalMatrixDirty()
{
	m_matrixLocalDirty = true;
	SetWorldMatrixDirty();
}

void SceneNode::Update(unsigned int elpasedTime)
{
	if (m_entity)
	{
		m_entity->OnUpdate(elpasedTime);
		if (m_matrixDirtyUpdate)
		{
			m_entity->OnUpdateMatrixChanged();
			m_matrixDirtyUpdate = false;
		}
	}
	for (auto& child : m_children)
	{
		child->Update(elpasedTime);
	}
}

void SceneNode::Render()
{
	if (!IsVisible())
		return;
	if (m_entity)
	{
		if (!m_entity->GetLocalAABB().is_empty())
		{
			m_entity->OnRender();
		}
	}
	for (auto& child : m_children)
	{
		child->Render();
	}
}

g2d::SceneNode* SceneNode::CreateSceneNode(g2d::Entity* e, bool autoRelease)
{
	if (e == nullptr)
	{
		return nullptr;
	}

	auto rst = new ::SceneNode(this, e, autoRelease);
	m_children.push_back(rst);
	return rst;
}

g2d::SceneNode* SceneNode::SetPivot(const gml::vec2& pivot)
{
	SetLocalMatrixDirty();
	m_pivot = pivot;
	return this;
}
g2d::SceneNode* SceneNode::SetScale(const gml::vec2& scale)
{
	if (m_entity)
	{
		m_entity->OnScale(scale);
	}
	SetLocalMatrixDirty();
	m_scale = scale;
	return this;
}
g2d::SceneNode* SceneNode::SetPosition(const gml::vec2& position)
{
	if (m_entity)
	{
		m_entity->OnMove(position);
	}
	SetLocalMatrixDirty();
	m_position = position;

	return this;
}
g2d::SceneNode* SceneNode::SetRotation(float radian)
{
	if (m_entity)
	{
		m_entity->OnRotate(radian);
	}
	SetLocalMatrixDirty();
	m_rotationRadian = radian;
	return this;
}

Scene::Scene()
{
	m_root = new ::SceneNode(nullptr, nullptr, false);
}
Scene::~Scene()
{
	delete m_root;
}

#include "render_system.h"
void Scene::Render()
{
	::GetRenderSystem()->SetViewMatrix(m_camera->GetViewMatrix());
	return m_root->Render();
}