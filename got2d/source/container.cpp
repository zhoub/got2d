#include "scene.h"
#include <algorithm>

ComponentContainer::~ComponentContainer()
{
	for (auto& c : m_components)
	{
		if (c.AutoRelease)
		{
			c.ComponentPtr->Release();
		}
	}
	m_components.clear();
	DelayRemove();
}

void ComponentContainer::Collect()
{
	if (m_collectionChanged)
	{
		DelayRemove();
		Recollect();
		m_collectionChanged = false;
	}
}

bool ComponentContainer::Add(g2d::SceneNode* parent, g2d::Component* component, bool autoRelease)
{
	ENSURE(component != nullptr);
	if (m_components.empty())
	{
		m_components.push_back({ component , autoRelease });
		component->SetSceneNode(parent);
		component->OnInitial();
		m_collectionChanged = true;
		return true;
	}
	else
	{
		auto itEndReleased = std::end(m_released);
		auto itFoundReleased = std::find_if(std::begin(m_released), itEndReleased,
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFoundReleased != itEndReleased)
		{
			m_released.erase(itFoundReleased);
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
			m_components.push_back({ component, autoRelease });
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
			m_components.insert(itCur, { component, autoRelease });
		}
		component->SetSceneNode(parent);
		component->OnInitial();
		m_collectionChanged = true;
		return true;
	}
}

bool ComponentContainer::Remove(g2d::Component* component, bool forceNotReleased)
{
	auto itEndReleased = std::end(m_released);
	if (itEndReleased != std::find_if(std::begin(m_released), itEndReleased,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; }))
	{
		return false;
	}

	auto itEnd = std::end(m_components);
	auto itFound = std::find_if(std::begin(m_components), itEnd,
		[component](NodeComponent& c) {return c.ComponentPtr == component; });

	if (itFound != itEnd)
	{
		m_released.push_back({ component, !forceNotReleased && itFound->AutoRelease });
		m_components.erase(itFound);
		m_collectionChanged = true;
		return true;
	}
	else
	{
		return false;
	}
}

bool ComponentContainer::IsAutoRelease(g2d::Component* component) const
{
	bool forcedNotReleased = false;
	auto itEndReleased = std::end(m_released);
	auto itFoundReleased = std::find_if(std::begin(m_released), itEndReleased,
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
		return !forcedNotReleased;
	}
	else
	{
		return !forcedNotReleased && itFound->AutoRelease;
	}
}

g2d::Component* ComponentContainer::At(uint32_t index) const
{
	ENSURE(index < GetCount());
	return m_components.at(index).ComponentPtr;
}

void ComponentContainer::DelayRemove()
{
	if (m_released.size() != 0)
	{
		for (auto& rcomponent : m_released)
		{
			if (rcomponent.AutoRelease)
			{
				rcomponent.ComponentPtr->Release();
			}
		}
		m_released.clear();
	}
}

void ComponentContainer::Recollect()
{
	m_collection.clear();
	for (auto& c : m_components)
	{
		m_collection.push_back(c);
	}
}

SceneNodeContainer::~SceneNodeContainer()
{
	ClearChildren();
}


void SceneNodeContainer::ClearChildren()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
	DelayRemove();
}

void SceneNodeContainer::Collect()
{
	if (m_collectionChanged)
	{
		DelayRemove();
		Recollect();
		m_collectionChanged = false;
	}
}

::SceneNode* SceneNodeContainer::CreateChild(::Scene& scene, ::SceneNode& parent)
{
	uint32_t childID = GetCount();
	auto rst = new ::SceneNode(scene, &parent, childID);
	m_children.push_back(rst);
	m_collectionChanged = true;
	return rst;
}

::SceneNode* SceneNodeContainer::CreateChild(::Scene& scene, SceneNodeContainer& parent)
{
	uint32_t childID = GetCount();
	auto rst = new ::SceneNode(scene, parent, childID);
	m_children.push_back(rst);
	m_collectionChanged = true;
	return rst;
}


::SceneNode* SceneNodeContainer::At(uint32_t index) const
{
	ENSURE(index < m_children.size());
	return m_children[index];
}

::SceneNode * SceneNodeContainer::First() const
{
	return m_children.empty() ? nullptr : *m_children.begin();
}

::SceneNode * SceneNodeContainer::Last() const
{
	return m_children.empty() ? nullptr : m_children.back();
}



// 删除节点虽然会影响RenderingOder的连续性
// 但是不会影响RenderingOrder的顺序，所以不做更新
void SceneNodeContainer::Remove(::SceneNode& child)
{
	::SceneNode* releasedChild = &child;

	auto itEndReleased = std::end(m_released);
	if (itEndReleased == std::find(std::begin(m_released), itEndReleased, releasedChild))
	{
		auto itEnd = std::end(m_children);
		auto itFound = std::find(std::begin(m_children), itEnd, releasedChild);
		if (itFound != itEnd)
		{
			uint32_t index = releasedChild->GetChildIndex();
			m_released.push_back(releasedChild);
			itFound = m_children.erase(itFound);
			itEnd = std::end(m_children);
			for (; itFound != itEnd; itFound++)
			{
				(*itFound)->SetChildIndex(index++);
			}
			m_collectionChanged = true;
		}
	}
}

void SceneNodeContainer::Move(uint32_t from, uint32_t to)
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
}

void SceneNodeContainer::DelayRemove()
{
	if (m_released.size() != 0)
	{
		for (auto& rnode : m_released)
		{
			delete rnode;
		}
		m_released.clear();
	}
}


void SceneNodeContainer::Recollect()
{
	m_collection.clear();
	for (auto& c : m_children)
	{
		m_collection.push_back(c);
	}
}