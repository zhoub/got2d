#include "scene.h"
#include <algorithm>

BaseNode::BaseNode()
	: m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotation(0)
	, m_matrixLocal(gml::mat32::identity())
{
}

BaseNode::~BaseNode()
{
	// 需要先移除那些需要移除的
	// 因为有些对象会被强制修改auto release选项
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
void BaseNode::OnUpdateChildren(uint32_t deltaTime)
{
	auto children = GetChildrenCollection();
	for (auto& child : children)
	{
		child->OnUpdate(deltaTime);
	}
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
		auto itEndReleased = std::end(m_releasedComponents);
		auto itFoundReleased = std::find_if(std::begin(m_releasedComponents), itEndReleased,
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFoundReleased != itEndReleased)
		{
			m_releasedComponents.erase(itFoundReleased);
			return true;
		}

		auto itEnd = std::end(m_components);
		auto itFound = std::find_if(std::begin(m_components), itEnd,
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFound != itEnd)
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
	auto itEndReleased = std::end(m_releasedComponents);
	if (itEndReleased != std::find_if(std::begin(m_releasedComponents), itEndReleased,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; }))
	{
		return false;
	}

	auto itEnd = std::end(m_components);
	if (itEnd == std::find_if(std::begin(m_components), itEnd,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; }))
	{
		return false;
	}

	m_releasedComponents.push_back({ this, component, forceNotReleased });
	m_componentChanged = true;
	return true;
}

bool BaseNode::_IsComponentAutoRelease(g2d::Component* component) const
{
	bool forcedNotReleased = false;
	auto itEndReleased = std::end(m_releasedComponents);
	auto itFoundReleased = std::find_if(std::begin(m_releasedComponents), itEndReleased,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFoundReleased != itEndReleased)
	{
		forcedNotReleased = itFoundReleased->AutoRelease;
	}

	auto itEnd = std::end(m_components);
	auto itFound = std::find_if(std::begin(m_components), itEnd,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFound == itEnd)
	{
		return false;
	}
	return !forcedNotReleased && itFound->AutoRelease;
}

g2d::Component* BaseNode::_GetComponentByIndex(uint32_t index) const
{
	ENSURE(index < _GetComponentCount());
	return m_components.at(index).ComponentPtr;
}

std::vector<NodeComponent>& BaseNode::GetComponentCollection()
{
	if (m_componentChanged)
	{
		DelayRemoveComponents();
		RecollectComponents();
		m_componentChanged = false;
	}
	return m_collectionComponents;
}

std::vector<::SceneNode*>& BaseNode::GetChildrenCollection()
{
	if (m_childrenChanged)
	{
		DelayRemoveChildren();
		RecollectChildren();
		m_childrenChanged = false;
	}
	return m_collectionChildren;
}


void BaseNode::OnCreateChild(::Scene& scene, ::SceneNode& child)
{
	AdjustRenderingOrder();
	scene.GetSpatialGraph()->Add(*child.GetEntity());
	m_childrenChanged = true;
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
		auto releasedFunc = [&](::SceneNode* child)->bool
		{
			if (removeChild == child)
			{
				delete child;
				return true;
			}
			return false;
		};
		std::replace_if(std::begin(m_children), std::end(m_children),
			releasedFunc, nullptr);
	}
	m_releasedChildren.clear();

	//remove null elements.
	auto itCurr = m_children.begin();
	auto itNext = itCurr;
	auto itEnd = m_children.end();
	uint32_t index = 0;
	for (; itNext != itEnd; itNext++)
	{
		::SceneNode* node = *itNext;
		if (node != nullptr)
		{
			node->SetChildIndex(index++);
			*(itCurr++) = node;
		}
	}
	m_children.erase(itCurr, itEnd);
}

void BaseNode::DelayRemoveComponents()
{
	for (auto& rc : m_releasedComponents)
	{
		auto itEnd = std::end(m_components);
		auto itFound = std::find_if(std::begin(m_components), itEnd,
			[rc](NodeComponent& c) {return c.ComponentPtr == rc.ComponentPtr; });
		if (itFound != itEnd)
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

void BaseNode::RecollectChildren()
{
	m_collectionChildren.clear();
	for (auto& c : m_children)
	{
		m_collectionChildren.push_back(c);
	}
}

void BaseNode::RecollectComponents()
{
	m_collectionComponents.clear();
	for (auto& c : m_components)
	{
		m_collectionComponents.push_back(c);
	}
}