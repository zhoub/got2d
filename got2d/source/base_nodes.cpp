#include "scene.h"
#include <algorithm>

BaseNode::BaseNode()
	: m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotation(0)
	, m_matrixLocal(gml::mat32::identity())
{ }

BaseNode::~BaseNode()
{
	DelayRemoveComponents();
	for (auto& c : m_components)
	{
		if (c.AutoRelease)
		{
			c.ComponentPtr->Release();
		}
	}
	m_components.clear();
	m_releasedComponents.clear();
	EmptyChildren();
}

void BaseNode::EmptyChildren()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
	m_releasedChildren.clear();
}

bool BaseNode::_AddComponent(g2d::Component* component, bool autoRelease, g2d::SceneNode* node)
{
	ENSURE(component != nullptr);
	if (m_components.empty())
	{
		m_components.push_back({ this, component , autoRelease });
		component->SetSceneNode(node);
		component->OnInitial();
		m_componentChanged = true;
		return true;
	}
	else
	{
		auto itFoundReleased = std::find_if(std::begin(m_releasedComponents), std::end(m_releasedComponents),
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFoundReleased != std::end(m_releasedComponents))
		{
			m_releasedComponents.erase(itFoundReleased);
			return true;
		}

		auto itFound = std::find_if(std::begin(m_components), std::end(m_components),
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFound != std::end(m_components))
		{
			return false;
		}

		int cOrder = component->GetExecuteOrder();
		int lastOrder = m_components.back().ComponentPtr->GetExecuteOrder();
		if (cOrder < lastOrder)
		{
			m_components.push_back({ this, component, autoRelease });
			component->SetSceneNode(node);
			component->OnInitial();
		}
		else //try insert
		{
			auto itCur = std::begin(m_components);
			auto itEnd = std::end(m_components);
			for (; itCur != itEnd; itCur++)
			{
				if (itCur->ComponentPtr->GetExecuteOrder() > cOrder)
					break;
			}
			m_components.insert(itCur, { this, component, autoRelease });
			component->SetSceneNode(node);
			component->OnInitial();
		}
		m_componentChanged = true;
		return true;
	}
}

bool BaseNode::_RemoveComponent(g2d::Component* component, bool forceNotReleased)
{
	auto itFoundReleased = std::find_if(std::begin(m_releasedComponents), std::end(m_releasedComponents),
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFoundReleased != std::end(m_releasedComponents))
	{
		return false;
	}

	auto itFound = std::find_if(std::begin(m_components), std::end(m_components),
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFound == std::end(m_components))
	{
		return false;
	}

	m_releasedComponents.push_back({ this, component, forceNotReleased });
	return true;
}

bool BaseNode::_IsComponentAutoRelease(g2d::Component* component) const
{
	auto itFound = std::find_if(std::begin(m_components), std::end(m_components),
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFound == std::end(m_components))
	{
		return false;
	}
	return itFound->AutoRelease;
}

g2d::Component* BaseNode::_GetComponentByIndex(uint32_t index) const
{
	ENSURE(index < _GetComponentCount());
	return m_components.at(index).ComponentPtr;
}

std::vector<NodeComponent>& BaseNode::GetComponenetCollection()
{
	if (m_componentChanged)
	{
		m_collectionComponents.clear();
		for (auto& c : m_components)
		{
			m_collectionComponents.push_back(c);
		}
		m_componentChanged = false;
	}
	return m_collectionComponents;
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

void BaseNode::_Update(uint32_t deltaTime)
{
	for (auto& child : m_children)
	{
		child->Update(deltaTime);
	}
	DelayRemoveChildren();
	DelayRemoveComponents();
}

::SceneNode* BaseNode::_GetChildByIndex(uint32_t index) const
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
	m_releasedChildren.push_back(child);
}

void BaseNode::DelayRemoveChildren()
{
	for (auto removeChild : m_releasedChildren)
	{
		std::replace_if(m_children.begin(), m_children.end(), [&](::SceneNode* child)->bool
		{
			if (removeChild == child)
			{
				delete child;
				return true;
			}
			else
			{
				return false;
			}
		}, nullptr);
	}
	m_releasedChildren.clear();

	//remove null elements.
	auto tail = std::remove(m_children.begin(), m_children.end(), nullptr);
	m_children.erase(tail, m_children.end());

	int index = 0;
	for (auto& child : m_children)
	{
		child->SetChildIndex(index++);
	}
}

void BaseNode::DelayRemoveComponents()
{
	for (auto& rc : m_releasedComponents)
	{
		auto itFound = std::find_if(std::begin(m_components), std::end(m_components),
			[rc](NodeComponent& c) {return c.ComponentPtr == rc.ComponentPtr; });
		if (itFound != std::end(m_components))
		{
			if (!rc.AutoRelease && itFound->AutoRelease)
			{
				itFound->ComponentPtr->Release();
			}
			m_components.erase(itFound);
		}
	}
	m_releasedComponents.clear();
}