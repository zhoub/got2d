#include "scene.h"

SceneNode::SceneNode(::Scene& scene, ::SceneNode* parent, uint32_t childID)
	: m_scene(scene)
	, m_parent(parent)
	, m_parentContainer(parent->m_children)
	, m_worldTransform(*this, m_localTransform)
	, m_childIndex(childID)
{

}

SceneNode::SceneNode(::Scene& scene, SceneNodeContainer& parentContainer, uint32_t childID)
	: m_scene(scene)
	, m_parent(nullptr)
	, m_parentContainer(parentContainer)
	, m_worldTransform(*this, m_localTransform)
	, m_childIndex(childID)
{
}

SceneNode::~SceneNode()
{
	m_components.Traversal([&](g2d::Component* component)
	{
		m_scene.GetSpatialGraph().Remove(*component);
	});
}

const gml::mat32& SceneNode::GetWorldMatrix()
{
	return m_worldTransform.GetMatrix();
}

g2d::SceneNode* SceneNode::SetPivot(const gml::vec2& pivot)
{
	m_localTransform.SetPivot(pivot);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetScale(const gml::vec2& scale)
{
	m_components.OnScale(scale);
	m_localTransform.SetScale(scale);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode* SceneNode::SetPosition(const gml::vec2& position)
{
	m_components.OnMove(position);
	m_localTransform.SetPosition(position);
	SetWorldMatrixDirty();
	return this;
}

g2d::SceneNode * SceneNode::SetWorldPosition(const gml::vec2 & position)
{
	auto localPos = WorldToParent(position);
	SetPosition(localPos);
	return this;
}

g2d::SceneNode * SceneNode::SetRight(const gml::vec2 & right)
{
	auto& oldRight = m_worldTransform.GetRight();
	auto cos = dot(right, oldRight);
	bool ccw = cross(right, oldRight) < 0;
	if (gml::fequal(cos, -1.0f))
	{
		gml::radian r = m_localTransform.GetRotation() + gml::radian(gml::PI);
		SetRotation(gml::limited_rotation(r));
	}
	else if (!gml::fequal(cos, 1.0f))
	{
		float acosr = acos(cos);
		gml::radian rdiff = gml::radian(ccw ? acosr : -acosr);
		gml::radian r = m_localTransform.GetRotation() + rdiff;
		SetRotation(gml::limited_rotation(r));
	}
	return this;
}

g2d::SceneNode * SceneNode::SetUp(const gml::vec2 & up)
{
	auto& oldUp = m_worldTransform.GetUp();
	auto cos = dot(up, oldUp);
	bool ccw = cross(up, oldUp) < 0;
	if (gml::fequal(cos, -1.0f))
	{
		gml::radian r = m_localTransform.GetRotation() + gml::radian(gml::PI);
		SetRotation(gml::limited_rotation(r));
	}
	else if (!gml::fequal(cos, 1.0f))
	{
		float acosr = acos(cos);
		gml::radian rdiff = gml::radian(ccw ? acosr : -acosr);
		gml::radian r = m_localTransform.GetRotation() + rdiff;
		SetRotation(gml::limited_rotation(r));
	}
	return this;
}

g2d::SceneNode* SceneNode::SetRotation(gml::radian r)
{
	m_components.OnRotate(r);
	m_localTransform.SetRotation(r);
	SetWorldMatrixDirty();
	return this;
}

void SceneNode::AdjustRenderingOrder()
{
	if (ParentIsScene() && m_childIndex == 0)
	{
		m_scene.AdjustRenderingOrder();
	}
	else
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
}

void SceneNode::SetWorldMatrixDirty()
{
	m_worldTransform.SetMatrixDirty();
	m_children.Traversal([](::SceneNode* child)
	{
		child->SetWorldMatrixDirty();
	});
	m_matrixDirtyUpdate = true;
}

void SceneNode::AdjustSpatial()
{
	m_components.Traversal([&](g2d::Component* component)
	{
		m_scene.GetSpatialGraph().Add(*component);
	});
}

void SceneNode::OnUpdate(uint32_t deltaTime)
{
	m_components.OnUpdate(deltaTime);
	if (m_matrixDirtyUpdate)
	{
		// static object need to adjust location in
		// quad tree, at this time, we may adjust it
		// before visibility testing process, aka 
		// rendering process
		if (IsStatic())
		{
			AdjustSpatial();
		}
		m_components.OnUpdateMatrixChanged();
		m_matrixDirtyUpdate = false;
	}

	m_children.OnUpdate(deltaTime);
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

void SceneNode::Release()
{
	// deleteing node will affect continuity of
	// rendering order, but wont change the order,
	// so we do nothing when deleting nodes
	m_isRemoved = true;
	m_parentContainer.Remove(*this);
	m_scene.OnRemoveSceneNode(*this);
}

void SceneNode::MoveToFront()
{
	auto oldIndex = m_childIndex;
	if (m_parentContainer.Move(m_childIndex, m_parentContainer.GetCount() - 1))
	{
		m_scene.SetRenderingOrderDirty(m_parentContainer.At(oldIndex));
	}
}

void SceneNode::MoveToBack()
{
	if (m_parentContainer.Move(m_childIndex, 0))
	{
		m_scene.SetRenderingOrderDirty(this);
	}
}

void SceneNode::MovePrev()
{
	if (m_parentContainer.Move(m_childIndex, m_childIndex - 1))
	{
		m_scene.SetRenderingOrderDirty(this);
	}
}

void SceneNode::MoveNext()
{
	auto oldIndex = m_childIndex;
	if (m_parentContainer.Move(m_childIndex, m_childIndex + 1))
	{
		m_scene.SetRenderingOrderDirty(m_parentContainer.At(oldIndex));
	}
}

bool SceneNode::AddComponent(g2d::Component* component, bool autoRelease)
{
	ENSURE(component != nullptr);
	auto successed = m_components.Add(this, component, autoRelease);
	if (successed)
	{
		m_scene.GetSpatialGraph().Add(*component);
		m_scene.SetRenderingOrderDirty(this);
		return true;
	}
	else
	{
		return false;
	}
}

bool SceneNode::RemoveComponent(g2d::Component * component)
{
	ENSURE(component != nullptr);
	if (m_components.Remove(component, false))
	{
		m_scene.GetSpatialGraph().Remove(*component);
		return true;
	}
	else
	{
		return false;
	}
}

bool SceneNode::RemoveComponentWithoutRelease(g2d::Component * component)
{
	ENSURE(component != nullptr);
	return m_components.Remove(component, true);
}

bool SceneNode::IsComponentAutoRelease(g2d::Component * component) const
{
	ENSURE(component != nullptr);
	return m_components.IsAutoRelease(component);
}

inline g2d::Component * SceneNode::GetComponentByIndex(uint32_t index) const
{

	ENSURE(index < m_components.GetCount());
	return m_components.At(index);
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
	return m_worldTransform.GetPosition();
}

const gml::vec2 & SceneNode::GetRight()
{
	return m_worldTransform.GetRight();
}

const gml::vec2 & SceneNode::GetUp()
{
	return m_worldTransform.GetUp();
}

gml::vec2 SceneNode::WorldToLocal(const gml::vec2& pos)
{
	gml::mat33 worldMatrixInv = gml::mat33(GetWorldMatrix()).inversed();
	auto p = gml::transform_point(worldMatrixInv, pos);
	return p;
}

gml::vec2 SceneNode::WorldToParent(const gml::vec2& pos)
{
	if (ParentIsScene())
	{
		return pos;
	}
	else
	{
		gml::mat33 worldMatrixInv = gml::mat33(m_parent->GetWorldMatrix()).inversed();
		auto p = gml::transform_point(worldMatrixInv, pos);
		return p;
	}
}

void SceneNode::SetRenderingOrder(uint32_t & order)
{
	m_renderingOrder = order++;
	m_components.Traversal([&](g2d::Component* component)
	{
		component->SetRenderingOrder(order);
	});
	m_children.Traversal([&](::SceneNode* child)
	{
		child->SetRenderingOrder(order);
	});
}

void SceneNode::SetRenderingOrderOnly(uint32_t order)
{
	m_renderingOrder = order;
}

void SceneNode::OnMessage(const g2d::Message& message)
{
	m_components.OnMessage(message);
	m_children.OnMessage(message);
}

void SceneNode::OnCursorEnterFrom(::SceneNode* adjacency)
{
	m_components.OnCursorEnterFrom(adjacency);
}

void SceneNode::OnCursorLeaveTo(::SceneNode* adjacency)
{
	m_components.OnCursorLeaveTo(adjacency);
}

void SceneNode::OnCursorHovering()
{

	m_components.OnCursorHovering();
}

void SceneNode::OnClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLClick();
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRClick();
	}
	else
	{
		m_components.OnMClick();
	}
}

void SceneNode::OnDoubleClick(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDoubleClick();
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDoubleClick();
	}
	else
	{
		m_components.OnMDoubleClick();
	}
}

void SceneNode::OnDragBegin(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDragBegin();
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDragBegin();
	}
	else
	{
		m_components.OnMDragBegin();
	}
}

void SceneNode::OnDragging(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDragging();
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDragging();
	}
	else
	{
		m_components.OnMDragging();
	}
}

void SceneNode::OnDragEnd(g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDragEnd();
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDragEnd();
	}
	else
	{
		m_components.OnMDragEnd();
	}
}

void SceneNode::OnDropping(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDropping(dropped);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDropping(dropped);
	}
	else
	{
		m_components.OnMDropping(dropped);
	}
}

void SceneNode::OnDropTo(::SceneNode* dropped, g2d::MouseButton button)
{
	if (button == g2d::MouseButton::Left)
	{
		m_components.OnLDropTo(dropped);
	}
	else if (button == g2d::MouseButton::Right)
	{
		m_components.OnRDropTo(dropped);
	}
	else
	{
		m_components.OnMDropTo(dropped);
	}
}

void SceneNode::OnKeyPress(g2d::KeyCode key)
{
	m_components.OnKeyPress(key);
	m_children.OnKeyPress(key);
}

void SceneNode::OnKeyPressingBegin(g2d::KeyCode key)
{
	m_components.OnKeyPressingBegin(key);
	m_children.OnKeyPressingBegin(key);
}

void SceneNode::OnKeyPressing(g2d::KeyCode key)
{
	m_components.OnKeyPressing(key);
	m_children.OnKeyPressing(key);
}

void SceneNode::OnKeyPressingEnd(g2d::KeyCode key)
{
	m_components.OnKeyPressingEnd(key);
	m_children.OnKeyPressingEnd(key);
}
