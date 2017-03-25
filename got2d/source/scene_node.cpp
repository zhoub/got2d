#include "scene.h"
#include "input.h"

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
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnScale(scale);
	});
	_SetScale(scale);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetPosition(const gml::vec2& position)
{
	m_entity->OnMove(position);
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnMove(position);
	});
	_SetPosition(position);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetRotation(gml::radian r)
{
	m_entity->OnRotate(r);
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnRotate(r);
	});
	_SetRotation(r);
	SetWorldMatrixDirty();
	return this;
}

void SceneNode::SetWorldMatrixDirty()
{
	m_matrixWorldDirty = true;
	TraversalChildren([](::SceneNode* child)
	{
		child->SetWorldMatrixDirty();
	});
	m_matrixDirtyEntityUpdate = true;
}

void SceneNode::AdjustSpatial()
{
	m_scene.GetSpatialGraph()->Add(*m_entity);
}

void SceneNode::OnUpdate(uint32_t deltaTime)
{
	m_entity->OnUpdate(deltaTime);
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnUpdate(deltaTime);
	}
	if (m_matrixDirtyEntityUpdate)
	{
		// 如果是静态对象，需要重新修正其在四叉树的位置
		// 现在阶段只需要在test visible之前处理好就行.
		// 也就是 Scene::Render之前
		if (IsStatic())
		{
			AdjustSpatial();
		}
		m_entity->OnUpdateMatrixChanged();
		for (auto& c : components)
		{
			c.ComponentPtr->OnUpdate(deltaTime);
		}
		m_matrixDirtyEntityUpdate = false;
	}

	OnUpdateChildren(deltaTime);
}

void SceneNode::AdjustRenderingOrder()
{
	uint32_t curIndex = m_baseRenderingOrder + 1;

	m_entity->SetRenderingOrder(curIndex);
	curIndex++;

	TraversalChildren([&](::SceneNode* child)
	{
		child->SetRenderingOrder(curIndex);
	});

	::BaseNode* parent = _GetParent();
	::BaseNode* current = this;
	while (parent != nullptr)
	{
		parent->TraversalChildrenByIndex(m_childID + 1, [&](uint32_t index, ::SceneNode* child)
		{
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

	TraversalChildren([&](::SceneNode* child)
	{
		child->SetRenderingOrder(index);
	});
}

::SceneNode* SceneNode::GetPrevSibling() const
{
	if (m_childID == 0)
	{
		return nullptr;
	}
	return m_bparent._GetChildByIndex(m_childID - 1);
}

::SceneNode* SceneNode::GetNextSibling() const
{
	if (m_childID == m_bparent.GetChildCount() - 1)
	{
		return nullptr;
	}
	return m_bparent._GetChildByIndex(m_childID + 1);
}

void SceneNode::MoveToFront()
{
	m_bparent.MoveChild(m_childID, m_bparent.GetChildCount() - 1);
}

void SceneNode::MoveToBack()
{
	m_bparent.MoveChild(m_childID, 0);
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
	gml::mat33 worldMatrixInv = gml::mat33(GetWorldMatrix()).inversed();
	auto p = gml::transform_point(worldMatrixInv, pos);
	return p;
}

gml::vec2 SceneNode::WorldToParent(const gml::vec2& pos)
{
	gml::mat33 worldMatrixInv = gml::mat33(m_iparent.GetWorldMatrix()).inversed();
	auto p = gml::transform_point(worldMatrixInv, pos);
	return p;
}

void SceneNode::OnMessage(const g2d::Message& message)
{
	m_entity->OnMessage(message);
	auto& components = this->GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnMessage(message);
	}

	auto& children = this->GetChildrenCollection();
	for (auto& child : children)
	{
		child->OnMessage(message);
	};
}

void SceneNode::OnCursorEnterFrom(::SceneNode* adjacency)
{
	m_entity->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnCursorLeaveTo(::SceneNode* adjacency)
{
	m_entity->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnCursorHovering()
{
	m_entity->OnCursorHovering(GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnCursorHovering(GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnClick(g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLClick(GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRClick(GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMClick(GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDoubleClick(g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDoubleClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDoubleClick(GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDoubleClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDoubleClick(GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDoubleClick(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDoubleClick(GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDragBegin(g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragBegin(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDragBegin(GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragBegin(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDragBegin(GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDragBegin(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDragBegin(GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDragging(g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragging(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDragging(GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragging(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDragging(GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDragging(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDragging(GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDragEnd(g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragEnd(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDragEnd(GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragEnd(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDragEnd(GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDragEnd(GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDragEnd(GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDropping(::SceneNode* dropped, g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropping(dropped, GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDropping(dropped, GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDropping(dropped, GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDropping(dropped, GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnDropTo(::SceneNode* dropped, g2d::MouseButton button)
{
	auto& components = GetComponentCollection();
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		}
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		}
	}
	else
	{
		m_entity->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		for (auto& c : components)
		{
			c.ComponentPtr->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		}
	}
}

void SceneNode::OnKeyPress(g2d::KeyCode key)
{
	m_entity->OnKeyPress(key, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnKeyPress(key, GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnKeyPressingBegin(g2d::KeyCode key)
{
	m_entity->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnKeyPressing(g2d::KeyCode key)
{
	m_entity->OnKeyPressing(key, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnKeyPressing(key, GetMouse(), GetKeyboard());
	}
}

void SceneNode::OnKeyPressingEnd(g2d::KeyCode key)
{
	m_entity->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		c.ComponentPtr->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	}
}

void SceneNode::CollectSceneNodes(std::vector<::SceneNode*>& collection)
{
	collection.push_back(this);
	auto& childrenCollection = GetChildrenCollection();
	for (auto& child : childrenCollection)
	{
		child->CollectSceneNodes(collection);
	};
}

void SceneNode::CollectComponents(std::vector<NodeComponent>& collection)
{
	auto& components = GetComponentCollection();
	for (auto& c : components)
	{
		collection.push_back(c);
	}
}


