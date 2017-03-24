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
	m_matrixDirtyUpdate = true;
}

void SceneNode::AdjustSpatial()
{
	m_scene.GetSpatialGraph()->Add(*m_entity);
}

void SceneNode::Update(uint32_t deltaTime)
{
	m_entity->OnUpdate(deltaTime);
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnUpdate(deltaTime);
	});
	if (m_matrixDirtyUpdate)
	{
		//现在阶段只需要在test visible之前处理好就行.
		//也就是 Scene::Render之前
		if (IsStatic())
		{
			AdjustSpatial();
		}
		m_entity->OnUpdateMatrixChanged();
		TraversalComponent([](g2d::Component* component)
		{
			component->OnUpdateMatrixChanged();
		});
		m_matrixDirtyUpdate = false;
	}
	_Update(deltaTime);
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
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnMessage(message);
	});
	TraversalChildren([&](::SceneNode* child)
	{
		child->OnMessage(message);
	});
}

void SceneNode::OnCursorEnterFrom(::SceneNode* adjacency)
{
	m_entity->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnCursorLeaveTo(::SceneNode* adjacency)
{
	m_entity->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnCursorHovering()
{
	m_entity->OnCursorHovering(GetMouse(), GetKeyboard());
	TraversalComponent([](g2d::Component* component)
	{
		component->OnCursorHovering(GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnLClick(GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnRClick(GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnMClick(GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDoubleClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDoubleClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnLDoubleClick(GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDoubleClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnRDoubleClick(GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDoubleClick(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnMDoubleClick(GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDragBegin(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragBegin(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnLDragBegin(GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragBegin(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnRDragBegin(GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDragBegin(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnMDragBegin(GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDragging(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragging(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnLDragging(GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragging(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnRDragging(GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDragging(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnMDragging(GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDragEnd(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragEnd(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnLDragEnd(GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragEnd(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnRDragEnd(GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDragEnd(GetMouse(), GetKeyboard());
		TraversalComponent([](g2d::Component* component)
		{
			component->OnMDragEnd(GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDropping(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnLDropping(dropped, GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropping(dropped, GetMouse(), GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnRDropping(dropped, GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDropping(dropped, GetMouse(), GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnMDropping(dropped, GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnDropTo(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		});
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		});
	}
	else
	{
		m_entity->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		TraversalComponent([&](g2d::Component* component)
		{
			component->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		});
	}
}

void SceneNode::OnKeyPress(g2d::KeyCode key)
{
	m_entity->OnKeyPress(key, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnKeyPress(key, GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnKeyPressingBegin(g2d::KeyCode key)
{
	m_entity->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnKeyPressing(g2d::KeyCode key)
{
	m_entity->OnKeyPressing(key, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnKeyPressing(key, GetMouse(), GetKeyboard());
	});
}

void SceneNode::OnKeyPressingEnd(g2d::KeyCode key)
{
	m_entity->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	TraversalComponent([&](g2d::Component* component)
	{
		component->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	});
}

void SceneNode::CollectSceneNodes(std::vector<::SceneNode*>& collection)
{
	collection.push_back(this);
	TraversalChildren([&](::SceneNode* child)
	{
		child->CollectSceneNodes(collection);
	});
}

void SceneNode::CollectComponents(std::vector<g2d::Component*>& collection)
{
	for (auto& c : m_components)
	{
		collection.push_back(c.ComponentPtr);
	}

	TraversalChildren([&](::SceneNode* child)
	{
		child->CollectComponents(collection);
	});
}


