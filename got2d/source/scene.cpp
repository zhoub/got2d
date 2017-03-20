#include "scene.h"
#include "spatial_graph.h"
#include <algorithm>
#include <gmlconversion.h>

BaseNode::BaseNode()
	: m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotation(0)
	, m_isVisible(true)
	, m_matrixLocal(gml::mat32::identity())
{ }

BaseNode::~BaseNode()
{
	EmptyChildren();
}

void BaseNode::EmptyChildren()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
}

void BaseNode::OnCreateChild(::Scene& scene, ::SceneNode& child)
{
	AdjustRenderingOrder();
	scene.GetSpatialGraph()->Add(*child.GetEntity());
}

g2d::SceneNode* BaseNode::_CreateSceneNodeChild(::Scene& scene, ::SceneNode& parent, g2d::Entity& e, bool autoRelease)
{
	uint32_t childID = static_cast<uint32_t>(m_children.size());
	auto rst = new ::SceneNode(scene, parent, childID, &e, autoRelease);
	m_children.push_back(rst);
	OnCreateChild(scene, *rst);
	return rst;
}

g2d::SceneNode* BaseNode::_CreateSceneNodeChild(::Scene& scene, g2d::Entity& e, bool autoRelease)
{
	uint32_t childID = static_cast<uint32_t>(m_children.size());
	auto rst = new ::SceneNode(scene, childID, &e, autoRelease);
	m_children.push_back(rst);
	OnCreateChild(scene, *rst);
	return rst;
}

const gml::mat32& BaseNode::_GetLocalMatrix()
{
	if (m_matrixLocalDirty)
	{
		m_matrixLocal = gml::mat32::trsp(m_position, m_rotation, m_scale, m_pivot);
		m_matrixLocalDirty = false;
	}
	return m_matrixLocal;
}

void BaseNode::_SetVisibleMask(uint32_t mask, bool recurssive)
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

void BaseNode::_SetPivot(const gml::vec2& pivot)
{
	SetLocalMatrixDirty();
	m_pivot = pivot;
}

void BaseNode::_SetScale(const gml::vec2& scale)
{
	SetLocalMatrixDirty();
	m_scale = scale;
}

void BaseNode::_SetPosition(const gml::vec2& position)
{
	SetLocalMatrixDirty();
	m_position = position;
}

void BaseNode::_SetRotation(gml::radian r)
{
	SetLocalMatrixDirty();
	m_rotation = r;
}

void BaseNode::SetLocalMatrixDirty()
{
	m_matrixLocalDirty = true;
}

void BaseNode::_Update(uint32_t elpasedTime)
{
	for (auto& child : m_children)
	{
		child->Update(elpasedTime);
	}
	RemoveReleasedChildren();
}

::SceneNode* BaseNode::GetChildByIndex(uint32_t index) const
{
	ENSURE(index < m_children.size());
	return m_children[index];
}


void BaseNode::MoveChild(uint32_t from, uint32_t to)
{
	ENSURE(to < m_children.size() && from < m_children.size());
	if (from == to)
		return;

	auto& siblings = m_children;
	auto fromNode = siblings[from];
	if (from > to)
	{
		for (auto itID = to; itID < from; itID++)
		{
			siblings[itID + 1] = siblings[itID];
			siblings[itID + 1]->SetChildIndex(itID + 1);
		}
	}
	else
	{
		for (auto itID = from; itID < to; itID++)
		{
			siblings[itID] = siblings[itID + 1];
			siblings[itID]->SetChildIndex(itID);
		}
	}
	siblings[to] = fromNode;
	fromNode->SetChildIndex(to);

	AdjustRenderingOrder();
}

void BaseNode::Remove(::SceneNode* child)
{
	ENSURE(child != nullptr);
	m_pendingReleased.push_back(child);
}

void BaseNode::RemoveReleasedChildren()
{
	for (auto removeChild : m_pendingReleased)
	{
		std::replace_if(m_children.begin(), m_children.end(),
			[&](::SceneNode* child)->bool {
			if (removeChild == child)
			{
				delete child;
				return true;
			}
			return false;
		}, nullptr);
	}
	m_pendingReleased.clear();

	//remove null elements.
	auto tail = std::remove(m_children.begin(), m_children.end(), nullptr);
	m_children.erase(tail, m_children.end());
}

SceneNode::SceneNode(::Scene& scene, ::SceneNode& parent, uint32_t childID, g2d::Entity* entity, bool autoRelease)
	: m_scene(scene)
	, m_iparent(parent)
	, m_bparent(parent)
	, m_entity(entity)
	, m_childID(childID)
	, m_autoRelease(autoRelease)
{
	ENSURE(entity != nullptr);
	m_entity->SetSceneNode(this);
	m_entity->OnInitial();
}

SceneNode::SceneNode(::Scene& scene, uint32_t childID, g2d::Entity* entity, bool autoRelease)
	: m_scene(scene)
	, m_iparent(scene)
	, m_bparent(scene)
	, m_entity(entity)
	, m_childID(childID)
	, m_autoRelease(autoRelease)
{
	ENSURE(entity != nullptr);
	m_entity->SetSceneNode(this);
	m_entity->OnInitial();
}

SceneNode::~SceneNode()
{
	m_scene.GetSpatialGraph()->Remove(*m_entity);
	if (m_autoRelease)
	{
		m_entity->Release();
	}
}

g2d::Scene* SceneNode::GetScene() const
{
	return &m_scene;
}

const gml::mat32& SceneNode::GetWorldMatrix()
{
	if (m_matrixWorldDirty)
	{
		auto& matParent = m_iparent.GetWorldMatrix();
		m_matrixWorld = matParent * GetLocalMatrix();
		m_matrixWorldDirty = false;
	}
	return m_matrixWorld;
}

g2d::SceneNode* SceneNode::SetPivot(const gml::vec2& pivot)
{
	_SetPivot(pivot);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetScale(const gml::vec2& scale)
{
	m_entity->OnScale(scale);
	_SetScale(scale);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetPosition(const gml::vec2& position)
{
	m_entity->OnMove(position);
	_SetPosition(position);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetRotation(gml::radian r)
{
	m_entity->OnRotate(r);
	_SetRotation(r);
	SetWorldMatrixDirty();
	return this;
}

void SceneNode::SetWorldMatrixDirty()
{
	m_matrixWorldDirty = true;
	TraversalChildren([](::SceneNode* child) {
		child->SetWorldMatrixDirty();
	});
	m_matrixDirtyUpdate = true;
}

void SceneNode::AdjustSpatial()
{
	m_scene.GetSpatialGraph()->Add(*m_entity);
}

void SceneNode::Update(uint32_t elpasedTime)
{
	m_entity->OnUpdate(elpasedTime);
	if (m_matrixDirtyUpdate)
	{
		//现在阶段只需要在test visible之前处理好就行.
		//也就是 Scene::Render之前
		if (IsStatic())
		{
			AdjustSpatial();
		}
		m_entity->OnUpdateMatrixChanged();
		m_matrixDirtyUpdate = false;
	}
	_Update(elpasedTime);
}

void SceneNode::AdjustRenderingOrder()
{
	uint32_t curIndex = m_baseRenderingOrder + 1;

	m_entity->SetRenderingOrder(curIndex);
	curIndex++;

	TraversalChildren([&](::SceneNode* child) {
		child->SetRenderingOrder(curIndex);
	});

	::BaseNode* parent = _GetParent();
	::BaseNode* current = this;
	while (parent != nullptr)
	{
		parent->TraversalChildrenByIndex(m_childID + 1,
			[&](uint32_t index, ::SceneNode* child) {
			child->SetRenderingOrder(curIndex);
		});

		current = parent;
		parent = current->_GetParent();
	}
}

void SceneNode::SetRenderingOrder(uint32_t& index)
{
	//for mulity-entity backup.
	m_baseRenderingOrder = index++;
	m_entity->SetRenderingOrder(index);
	index++;

	TraversalChildren([&](::SceneNode* child) {
		child->SetRenderingOrder(index);
	});
}

::SceneNode* SceneNode::GetPrevSibling() const
{
	if (m_childID == 0)
	{
		return nullptr;
	}
	return m_bparent.GetChildByIndex(m_childID - 1);
}

::SceneNode* SceneNode::GetNextSibling() const
{
	if (m_childID == m_bparent.GetChildCount() - 1)
	{
		return nullptr;
	}
	return m_bparent.GetChildByIndex(m_childID + 1);
}

void SceneNode::MoveToFront()
{
	m_bparent.MoveChild(m_childID, 0);
}

void SceneNode::MoveToBack()
{
	m_bparent.MoveChild(m_childID, m_bparent.GetChildCount() - 1);
}

void SceneNode::MovePrev()
{
	m_bparent.MoveChild(m_childID, m_childID - 1);
}

void SceneNode::MoveNext()
{
	m_bparent.MoveChild(m_childID, m_childID + 1);
}

void SceneNode::SetStatic(bool s)
{
	if (m_isStatic != s)
	{
		m_isStatic = s;
		AdjustSpatial();
	}
}

gml::vec2 SceneNode::GetWorldPosition()
{
	auto localPos = _GetPosition();
	return gml::transform_point(GetWorldMatrix(), localPos);
}

gml::vec2 SceneNode::WorldToLocal(const gml::vec2& pos)
{
	gml::mat33 worldMatrixInv = gml::mat33(m_iparent.GetWorldMatrix()).inversed();
	auto p = gml::transform_point(worldMatrixInv, pos);
	return p;
}

void SceneNode::OnMessage(const g2d::Message& message)
{
	m_entity->OnMessage(message);
	TraversalChildren([&](::SceneNode* child) {
		child->OnMessage(message);
	});
}

Scene::Scene(float boundSize)
	: m_spatial(boundSize)
{
	CreateCameraNode();
}

void Scene::ResortCameraOrder()
{
	if (m_cameraOrderDirty)
	{
		m_cameraOrderDirty = false;
		m_cameraOrder = m_cameras;

		std::sort(m_cameraOrder.begin(), m_cameraOrder.end(),
			[](g2d::Camera* a, g2d::Camera* b)->bool {

			auto aOrder = a->GetRenderingOrder();
			auto bOrder = b->GetRenderingOrder();
			if (aOrder == bOrder)
			{
				return a->GetID() < b->GetID();
			}
			else
			{
				return aOrder < bOrder;
			}
		});
	}
}

void Scene::Release()
{
	EmptyChildren();
	delete this;
}

void Scene::Update(uint32_t elapsedTime)
{
	_Update(elapsedTime);
	AdjustRenderingOrder();
}

void Scene::AdjustRenderingOrder()
{
	uint32_t curIndex = 1;

	TraversalChildren([&](::SceneNode* child)
	{
		child->SetRenderingOrder(curIndex);
	});
}

void Scene::OnMessage(const g2d::Message& message)
{
	TraversalChildren([&](::SceneNode* child) {
		child->OnMessage(message);
	});

	if (message.Source == g2d::MessageSource::Mouse)
	{
		FindInteractiveObject(message);
	}
}

void Scene::FindInteractiveObject(const g2d::Message& message)
{
	ResortCameraOrder();
	auto cur = std::rbegin(m_cameraOrder);
	auto end = std::rend(m_cameraOrder);

	for (; cur != end; cur++)
	{
		g2d::Camera* camera = *cur;

		int coordX = message.MousePositionX;
		int coordY = message.MousePositionY;
		gml::vec2 worldCoord = camera->ScreenToWorld({ coordX, coordY });
		//if (camera->FindIntersectionObject(worldCoord))
		{
			return;
		}
	}
}

#include "render_system.h"
void Scene::Render()
{
	GetRenderSystem()->FlushRequests();
	ResortCameraOrder();
	for (auto camera : m_cameraOrder)
	{
		if (camera->IsActivity())
		{
			GetRenderSystem()->SetViewMatrix(camera->GetViewMatrix());

			std::vector<g2d::Entity*> visibleEntities;
			m_spatial.FindVisible(*camera, visibleEntities);

			//sort visibleEntities by render order
			std::sort(visibleEntities.begin(), visibleEntities.end(),
				[](g2d::Entity* a, g2d::Entity* b) {
				return a->GetRenderingOrder() < b->GetRenderingOrder();
			});

			for (auto& entity : visibleEntities)
			{
				entity->OnRender();
			}
			GetRenderSystem()->FlushRequests();
		}
	}
}

g2d::Camera* Scene::CreateCameraNode()
{
	Camera* camera = new ::Camera();
	CreateSceneNodeChild(camera, true);
	m_cameraOrderDirty = true;
	camera->SetID(static_cast<uint32_t>(m_cameras.size()));
	m_cameras.push_back(camera);
	return camera;
}

g2d::Camera* Scene::GetCameraByIndex(uint32_t index) const
{
	ENSURE(index < m_cameras.size());
	return m_cameras[index];
}