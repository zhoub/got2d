#include "scene.h"
#include "input.h"

SceneNode::SceneNode(::Scene& scene, ::SceneNode& parent, uint32_t childID, g2d::Entity* entity, bool autoRelease)
	: m_scene(scene)
	, m_iparent(parent)
	, m_bparent(parent)
	, m_entity(entity)
	, m_childIndex(childID)
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
	, m_childIndex(childID)
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
	auto onUpdate = [&](g2d::Component* component)
	{
		component->OnUpdate(deltaTime);
	};
	DispatchComponent(onUpdate);
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
		auto onUpdateMatrixChanged = [&](g2d::Component* component)
		{
			component->OnUpdateMatrixChanged();
		};
		DispatchComponent(onUpdateMatrixChanged);
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
		parent->TraversalChildrenByIndex(m_childIndex + 1, [&](uint32_t index, ::SceneNode* child)
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
	if (m_childIndex == 0)
	{
		return nullptr;
	}
	return m_bparent._GetChildByIndex(m_childIndex - 1);
}

::SceneNode* SceneNode::GetNextSibling() const
{
	if (m_childIndex == m_bparent.GetChildCount() - 1)
	{
		return nullptr;
	}
	return m_bparent._GetChildByIndex(m_childIndex + 1);
}

void SceneNode::MoveToFront()
{
	m_bparent.MoveChild(m_childIndex, m_bparent.GetChildCount() - 1);
}

void SceneNode::MoveToBack()
{
	m_bparent.MoveChild(m_childIndex, 0);
}

void SceneNode::MovePrev()
{
	m_bparent.MoveChild(m_childIndex, m_childIndex - 1);
}

void SceneNode::MoveNext()
{
	m_bparent.MoveChild(m_childIndex, m_childIndex + 1);
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
}

void SceneNode::OnCursorEnterFrom(::SceneNode* adjacency)
{
	m_entity->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	auto onCursorEnterFrom = [&](g2d::Component* component)
	{
		component->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());;
	};
	DispatchComponent(onCursorEnterFrom);
}

void SceneNode::OnCursorLeaveTo(::SceneNode* adjacency)
{
	m_entity->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	auto onCursorLeaveTo = [&](g2d::Component* component)
	{
		component->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	};
	DispatchComponent(onCursorLeaveTo);
}

void SceneNode::OnCursorHovering()
{
	m_entity->OnCursorHovering(GetMouse(), GetKeyboard());
	auto onCursorHovering = [](g2d::Component* component)
	{
		component->OnCursorHovering(GetMouse(), GetKeyboard());
	};
	DispatchComponent(onCursorHovering);
}

void SceneNode::OnClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLClick(GetMouse(), GetKeyboard());
		auto onClick = [](g2d::Component* component)
		{
			component->OnLClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onClick);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRClick(GetMouse(), GetKeyboard());
		auto onClick = [](g2d::Component* component)
		{
			component->OnRClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onClick);
	}
	else
	{
		m_entity->OnMClick(GetMouse(), GetKeyboard());
		auto onClick = [](g2d::Component* component)
		{
			component->OnMClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onClick);
	}
}

void SceneNode::OnDoubleClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDoubleClick(GetMouse(), GetKeyboard());
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnLDoubleClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDoubleClick);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDoubleClick(GetMouse(), GetKeyboard());
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnRDoubleClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDoubleClick);
	}
	else
	{
		m_entity->OnMDoubleClick(GetMouse(), GetKeyboard()); 
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnMDoubleClick(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDoubleClick);
	}
}

void SceneNode::OnDragBegin(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragBegin(GetMouse(), GetKeyboard());
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnLDragBegin(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragBegin);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragBegin(GetMouse(), GetKeyboard());
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnRDragBegin(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragBegin);
	}
	else
	{
		m_entity->OnMDragBegin(GetMouse(), GetKeyboard());
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnMDragBegin(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragBegin);		
	}
}

void SceneNode::OnDragging(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragging(GetMouse(), GetKeyboard());
		auto onDragging = [](g2d::Component* component)
		{
			component->OnLDragging(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragging);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragging(GetMouse(), GetKeyboard());
		auto onDragging = [](g2d::Component* component)
		{
			component->OnRDragging(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragging);
	}
	else
	{
		m_entity->OnMDragging(GetMouse(), GetKeyboard());
		auto onDragging = [](g2d::Component* component)
		{
			component->OnMDragging(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragging);
	}
}

void SceneNode::OnDragEnd(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDragEnd(GetMouse(), GetKeyboard());
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnLDragEnd(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragEnd);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDragEnd(GetMouse(), GetKeyboard());
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnRDragEnd(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragEnd);
	}
	else
	{
		m_entity->OnMDragEnd(GetMouse(), GetKeyboard());
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnMDragEnd(GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDragEnd);
	}
}

void SceneNode::OnDropping(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		};
		DispatchComponent(onDropping);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropping(dropped, GetMouse(), GetKeyboard());
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnRDropping(dropped, GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDropping);
	}
	else
	{
		m_entity->OnMDropping(dropped, GetMouse(), GetKeyboard());
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnMDropping(dropped, GetMouse(), GetKeyboard());
		};
		DispatchComponent(onDropping);
	}
}

void SceneNode::OnDropTo(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_entity->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		auto dropTo = [&] (g2d::Component* component)
		{
			component->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		};
		DispatchComponent(dropTo);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_entity->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		auto dropTo = [&](g2d::Component* component)
		{
			component->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		};
		DispatchComponent(dropTo);
	}
	else
	{
		m_entity->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		auto dropTo = [&](g2d::Component* component)
		{
			component->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		};
		DispatchComponent(dropTo);
	}
}

void SceneNode::OnKeyPress(g2d::KeyCode key)
{
	m_entity->OnKeyPress(key, GetMouse(), GetKeyboard());
	OnKeyPressComponentsAndChildren(key);
}

void SceneNode::OnKeyPressingBegin(g2d::KeyCode key)
{
	m_entity->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	OnKeyPressingBeginComponentsAndChildren(key);
}

void SceneNode::OnKeyPressing(g2d::KeyCode key)
{
	m_entity->OnKeyPressing(key, GetMouse(), GetKeyboard());
	OnKeyPressingComponentsAndChildren(key);
}

void SceneNode::OnKeyPressingEnd(g2d::KeyCode key)
{
	m_entity->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	OnKeyPressingEndComponentsAndChildren(key);
}


