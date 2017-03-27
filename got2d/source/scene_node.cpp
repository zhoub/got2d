#include "scene.h"
#include "input.h"

SceneNode::SceneNode(::Scene& scene, ::SceneNode* parent, uint32_t childID)
	: m_scene(scene)
	, m_parent(parent)
	, m_parentContainer(parent->m_children)
	, m_childIndex(childID)
{
}

SceneNode::SceneNode(::Scene& scene, SceneNodeContainer& parentContainer, uint32_t childID)
	: m_scene(scene)
	, m_parent(nullptr)
	, m_parentContainer(parentContainer)
	, m_childIndex(childID)
{
}

SceneNode::~SceneNode()
{
	/*
	m_scene.GetSpatialGraph()->Remove(*m_entity);
	if (m_autoRelease)
	{
		m_entity->Release();
	}
	*/
}

const gml::mat32& SceneNode::GetWorldMatrix()
{
	if (m_matrixWorldDirty)
	{
		if (m_parent == nullptr)
		{
			auto& matParent = m_parent->GetWorldMatrix();
			m_matrixWorld = matParent * GetLocalMatrix();
		}
		else
		{
			m_matrixWorld = GetLocalMatrix();
		}
		m_matrixWorldDirty = false;
	}
	return m_matrixWorld;
}

g2d::SceneNode* SceneNode::SetPivot(const gml::vec2& pivot)
{
	m_localTransform.SetPivot(pivot);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetScale(const gml::vec2& scale)
{
	m_components.Traversal([&](g2d::Component* component)
	{
		component->OnScale(scale);
	});
	m_localTransform.SetScale(scale);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetPosition(const gml::vec2& position)
{
	m_components.Traversal([&](g2d::Component* component)
	{
		component->OnMove(position);
	});
	m_localTransform.SetPosition(position);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetRotation(gml::radian r)
{
	m_components.Traversal([&](g2d::Component* component)
	{
		component->OnRotate(r);
	});
	m_localTransform.SetRotation(r);
	SetWorldMatrixDirty();
	return this;
}

void SceneNode::AdjustRenderingOrder()
{
	auto index = m_renderingOrder + 1;
	m_children.Traversal([&](::SceneNode* child)
	{
		child->SetRenderingOrder(index);
	});

	auto current = this;
	while (current != nullptr)
	{
		auto next = current->GetNextSibling();
		while (next != nullptr)
		{
			next->SetRenderingOrder(index);
			next = next->GetNextSibling();
		}
		current = current->GetParent();
	}
}

void SceneNode::SetWorldMatrixDirty()
{
	m_matrixWorldDirty = true;
	m_children.Traversal([](::SceneNode* child)
	{
		child->SetWorldMatrixDirty();
	});
	m_matrixDirtyEntityUpdate = true;
}

void SceneNode::AdjustSpatial()
{
	//m_scene.GetSpatialGraph()->Add(*m_entity);
}

void SceneNode::OnUpdate(uint32_t deltaTime)
{
	auto onUpdate = [&](g2d::Component* component)
	{
		component->OnUpdate(deltaTime);
	};
	m_components.Dispatch(onUpdate);
	if (m_matrixDirtyEntityUpdate)
	{
		// 如果是静态对象，需要重新修正其在四叉树的位置
		// 现在阶段只需要在test visible之前处理好就行.
		// 也就是 Scene::Render之前
		if (IsStatic())
		{
			AdjustSpatial();
		}
		auto onUpdateMatrixChanged = [&](g2d::Component* component)
		{
			component->OnUpdateMatrixChanged();
		};
		m_components.Dispatch(onUpdateMatrixChanged);
		m_matrixDirtyEntityUpdate = false;
	}

	auto onUpdateChildren = [&](::SceneNode* child)
	{
		child->OnUpdate(deltaTime);
	};
	m_children.Dispatch(onUpdateChildren);
}

g2d::Scene * SceneNode::GetScene() const { return &m_scene; }

::SceneNode* SceneNode::GetPrevSibling() const
{
	if (m_childIndex == 0)
	{
		return nullptr;
	}
	return m_parentContainer.At(m_childIndex - 1);
}

::SceneNode* SceneNode::GetNextSibling() const
{
	if (m_childIndex == m_parentContainer.GetCount() - 1)
	{
		return nullptr;
	}
	return m_parentContainer.At(m_childIndex + 1);
}

g2d::SceneNode * SceneNode::CreateChild()
{
	auto child = m_children.CreateChild(m_scene, *this);
	m_scene.SetRenderingOrderDirty(this);
	return child;
}

void SceneNode::Remove()
{
	if (m_parent != nullptr)
	{
		m_parentContainer.Remove(*this);
	}
}

void SceneNode::MoveToFront()
{
	m_parentContainer.Move(m_childIndex, m_parentContainer.GetCount() - 1);
	//m_scene.SetRenderingOrderDirty(&m_bparent);
}

void SceneNode::MoveToBack()
{
	m_parentContainer.Move(m_childIndex, 0);
	//m_scene.SetRenderingOrderDirty(&m_bparent);
}

void SceneNode::MovePrev()
{
	m_parentContainer.Move(m_childIndex, m_childIndex - 1);
	//m_scene.SetRenderingOrderDirty(&m_bparent);
}

void SceneNode::MoveNext()
{
	m_parentContainer.Move(m_childIndex, m_childIndex + 1);
	//m_scene.SetRenderingOrderDirty(&m_bparent);
}

void SceneNode::SetStatic(bool s)
{
	if (m_isStatic != s)
	{
		m_isStatic = s;
		AdjustSpatial();
	}
}

void SceneNode::SetVisibleMask(uint32_t mask, bool recursive)
{
	m_visibleMask = mask;
	if (recursive)
	{
		m_children.Traversal([&](::SceneNode* child)
		{
			child->SetVisibleMask(mask, true);
		});
	}
}

gml::vec2 SceneNode::GetWorldPosition()
{
	auto localPos = m_localTransform.GetPosition();
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
	if (m_parent != nullptr)
	{
		gml::mat33 worldMatrixInv = gml::mat33(m_parent->GetWorldMatrix()).inversed();
		auto p = gml::transform_point(worldMatrixInv, pos);
		return p;
	}
	else
	{
		return pos;
	}
}

void SceneNode::SetRenderingOrder(uint32_t & order)
{
	m_renderingOrder = order++;
	m_children.Traversal([&](::SceneNode* child)
	{
		child->SetRenderingOrder(order);
	});
}

void SceneNode::OnMessage(const g2d::Message& message)
{
	auto cf = [&](g2d::Component* component)
	{
		component->OnMessage(message);
	};
	m_components.Dispatch(cf);

	auto nf = [&](::SceneNode* child)
	{
		child->OnMessage(message);
	};
	m_children.Dispatch(nf);
}

void SceneNode::OnCursorEnterFrom(::SceneNode* adjacency)
{
	auto onCursorEnterFrom = [&](g2d::Component* component)
	{
		component->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());;
	};
	m_components.Dispatch(onCursorEnterFrom);
}

void SceneNode::OnCursorLeaveTo(::SceneNode* adjacency)
{
	auto onCursorLeaveTo = [&](g2d::Component* component)
	{
		component->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(onCursorLeaveTo);
}

void SceneNode::OnCursorHovering()
{
	auto onCursorHovering = [](g2d::Component* component)
	{
		component->OnCursorHovering(GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(onCursorHovering);
}

void SceneNode::OnClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onClick = [](g2d::Component* component)
		{
			component->OnLClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onClick);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onClick = [](g2d::Component* component)
		{
			component->OnRClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onClick);
	}
	else
	{
		auto onClick = [](g2d::Component* component)
		{
			component->OnMClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onClick);
	}
}

void SceneNode::OnDoubleClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnLDoubleClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDoubleClick);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnRDoubleClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDoubleClick);
	}
	else
	{
		auto onDoubleClick = [](g2d::Component* component)
		{
			component->OnMDoubleClick(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDoubleClick);
	}
}

void SceneNode::OnDragBegin(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnLDragBegin(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragBegin);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnRDragBegin(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragBegin);
	}
	else
	{
		auto onDragBegin = [](g2d::Component* component)
		{
			component->OnMDragBegin(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragBegin);
	}
}

void SceneNode::OnDragging(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onDragging = [](g2d::Component* component)
		{
			component->OnLDragging(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragging);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onDragging = [](g2d::Component* component)
		{
			component->OnRDragging(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragging);
	}
	else
	{
		auto onDragging = [](g2d::Component* component)
		{
			component->OnMDragging(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragging);
	}
}

void SceneNode::OnDragEnd(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnLDragEnd(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragEnd);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnRDragEnd(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragEnd);
	}
	else
	{
		auto onDragEnd = [](g2d::Component* component)
		{
			component->OnMDragEnd(GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDragEnd);
	}
}

void SceneNode::OnDropping(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnLDropping(dropped, GetMouse(), ::GetKeyboard());
		};
		m_components.Dispatch(onDropping);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnRDropping(dropped, GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDropping);
	}
	else
	{
		auto onDropping = [&](g2d::Component* component)
		{
			component->OnMDropping(dropped, GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(onDropping);
	}
}

void SceneNode::OnDropTo(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		auto dropTo = [&](g2d::Component* component)
		{
			component->OnLDropTo(dropped, GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(dropTo);
	}
	else if (button == g2d::MouseButton::Right)
	{
		auto dropTo = [&](g2d::Component* component)
		{
			component->OnRDropTo(dropped, GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(dropTo);
	}
	else
	{
		auto dropTo = [&](g2d::Component* component)
		{
			component->OnMDropTo(dropped, GetMouse(), GetKeyboard());
		};
		m_components.Dispatch(dropTo);
	}
}

void SceneNode::OnKeyPress(g2d::KeyCode key)
{
	auto cf = [&](g2d::Component* component)
	{
		component->OnKeyPress(key, GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(cf);

	auto nf = [&](::SceneNode* child)
	{
		child->OnKeyPress(key);
	};
	m_children.Dispatch(nf);
}

void SceneNode::OnKeyPressingBegin(g2d::KeyCode key)
{
	auto cf = [&](g2d::Component* component)
	{
		component->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(cf);

	auto nf = [&](::SceneNode* child)
	{
		child->OnKeyPressingBegin(key);
	};
	m_children.Dispatch(nf);
}

void SceneNode::OnKeyPressing(g2d::KeyCode key)
{
	auto cf = [&](g2d::Component* component)
	{
		component->OnKeyPressing(key, GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(cf);

	auto nf = [&](::SceneNode* child)
	{
		child->OnKeyPressing(key);
	};
	m_children.Dispatch(nf);
}

void SceneNode::OnKeyPressingEnd(g2d::KeyCode key)
{
	auto cf = [&](g2d::Component* component)
	{
		component->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	};
	m_components.Dispatch(cf);

	auto nf = [&](::SceneNode* child)
	{
		child->OnKeyPressingEnd(key);
	};
	m_children.Dispatch(nf);
}


