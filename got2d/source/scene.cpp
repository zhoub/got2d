#include "scene.h"
#include <algorithm>

g2d::SceneNode::~SceneNode() { }

g2d::Scene::~Scene() { }

SceneNode::SceneNode(g2d::Scene* scene, SceneNode* parent, g2d::Entity* entity, bool autoRelease)
	: m_scene(scene)
	, m_parent(parent)
	, m_entity(entity)
	, m_autoRelease(autoRelease)
	, m_matrixLocal(gml::mat32::identity())
	, m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotation(0)
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
		m_matrixLocal = gml::mat32::trsp(m_position, m_rotation, m_scale, m_pivot);
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

void SceneNode::SetVisibleMask(unsigned int mask, bool recurssive)
{
	m_visibleMask = mask;
	if (recurssive)
	{
		for (auto& child : m_children)
		{
			child->SetVisibleMask(mask, true);
		}
	}
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

void SceneNode::Render(g2d::Camera* m_camera)
{
	if (!IsVisible())
		return;
	if (m_entity)
	{
		if (!m_camera || m_camera->TestVisible(m_entity))
		{
			m_entity->OnRender();
		}
	}
	for (auto& child : m_children)
	{
		child->Render(m_camera);
	}
}

g2d::SceneNode* SceneNode::CreateSceneNode(g2d::Entity* e, bool autoRelease)
{
	if (e == nullptr)
	{
		return nullptr;
	}

	auto rst = new ::SceneNode(GetScene(), this, e, autoRelease);
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

g2d::SceneNode* SceneNode::SetRotation(gml::radian r)
{
	if (m_entity)
	{
		m_entity->OnRotate(r);
	}
	SetLocalMatrixDirty();
	m_rotation = r;
	return this;
}

void SceneNode::SetStatic(bool s)
{
	m_isStatic = s;
	if (m_isStatic)
	{
		//reset spartial
	}
}

Scene::Scene()
{
	m_root = new ::SceneNode(this, nullptr, nullptr, false);
	CreateCameraNode();
}

Scene::~Scene()
{
	delete m_root;
}

void Scene::SetRenderingOrderDirty()
{
	m_renderingOrderDirty = true;
}

void Scene::ResortCameraRenderingOrder()
{
	if (m_renderingOrderDirty)
	{
		m_renderingOrderDirty = false;
		m_renderingOrder = m_cameras;

		std::sort(m_renderingOrder.begin(), m_renderingOrder.end(),
			[](g2d::Camera* a, g2d::Camera* b)->bool {

			auto aOrder = a->GetRenderingOrder();
			auto bOrder = b->GetRenderingOrder();
			if (aOrder == bOrder)
			{
				return a->GetID() < b->GetID();
			}
			return aOrder < bOrder;
		});
	}
}

#include "render_system.h"
void Scene::Render()
{
	GetRenderSystem()->FlushRequests();

	ResortCameraRenderingOrder();
	for (size_t i = 0, n = m_renderingOrder.size(); i < n; i++)
	{
		auto& camera = m_renderingOrder[i];
		if (!camera->IsActivity())
			continue;
		GetRenderSystem()->SetViewMatrix(camera->GetViewMatrix());
		m_root->Render(camera);
		GetRenderSystem()->FlushRequests();
	}

}

g2d::Camera* Scene::CreateCameraNode()
{
	Camera* camera = new ::Camera();
	if (CreateSceneNode(camera, true) != nullptr)
	{
		m_renderingOrderDirty = true;
		camera->SetID(static_cast<unsigned int>(m_cameras.size()));
		m_cameras.push_back(camera);
		return camera;
	}
	else
	{
		camera->Release();
		return nullptr;
	}

}

g2d::Camera* Scene::GetCamera(unsigned int index) const
{
	if (index >= m_cameras.size())
		return nullptr;
	return m_cameras[index];
}