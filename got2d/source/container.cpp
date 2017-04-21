#include <algorithm>
#include "scene.h"

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
		auto itEndReleased = m_released.end();
		auto itFoundReleased = std::find_if(m_released.begin(), itEndReleased,
			[component](const NodeComponent& c) { return component == c.ComponentPtr; });
		if (itFoundReleased != itEndReleased)
		{
			m_released.erase(itFoundReleased);
			return true;
		}

		auto itEnd = m_components.end();
		auto itFound = std::find_if(m_components.begin(), itEnd,
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
			auto itCur = m_components.begin();
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
	auto itEndReleased = m_released.end();
	if (itEndReleased != std::find_if(m_released.begin(), itEndReleased,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; }))
	{
		return false;
	}

	auto itEnd = m_components.end();
	auto itFound = std::find_if(m_components.begin(), itEnd,
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
	auto itEndReleased = m_released.end();
	auto itFoundReleased = std::find_if(m_released.begin(), itEndReleased,
		[component](const NodeComponent& c) { return component == c.ComponentPtr; });
	if (itFoundReleased != itEndReleased)
	{
		forcedNotReleased = itFoundReleased->AutoRelease;
	}

	auto itEnd = m_components.end();
	auto itFound = std::find_if(m_components.begin(), itEnd,
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
	ENSURE(index < GetCount());
	return m_children.at(index);
}

::SceneNode * SceneNodeContainer::First() const
{
	return m_children.empty() ? nullptr : *m_children.begin();
}

::SceneNode * SceneNodeContainer::Last() const
{
	return m_children.empty() ? nullptr : m_children.back();
}

void SceneNodeContainer::Remove(::SceneNode& child)
{
	::SceneNode* releasedChild = &child;

	auto itEndReleased = m_released.end();
	if (itEndReleased == std::find(m_released.begin(), itEndReleased, releasedChild))
	{
		auto itEnd = m_children.end();
		auto itFound = std::find(m_children.begin(), itEnd, releasedChild);
		if (itFound != itEnd)
		{
			uint32_t index = releasedChild->GetChildIndex();
			m_released.push_back(releasedChild);
			itFound = m_children.erase(itFound);
			itEnd = m_children.end();
			for (; itFound != itEnd; itFound++)
			{
				(*itFound)->SetChildIndex(index++);
			}
			m_collectionChanged = true;
		}
	}
}

bool SceneNodeContainer::Move(uint32_t from, uint32_t to)
{
	ENSURE(to < GetCount() && from < GetCount());
	if (from == to)
		return false;

	auto& siblings = m_children;
	auto fromNode = siblings[from];
	auto formOrder = siblings[from]->GetRenderingOrder();
	auto toOrder = siblings[to]->GetRenderingOrder();

	// for actual dirty flag: RenderingOrderDirtyNode
	fromNode->SetRenderingOrderOnly(toOrder);
	siblings[to]->SetRenderingOrderOnly(formOrder);

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
	return true;
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



// Event Dispatcher 
void ComponentContainer::OnRotate(gml::radian r)
{
	for (auto& c : m_components)
	{
		c.ComponentPtr->OnRotate(r);
	}
}

void ComponentContainer::OnScale(const gml::vec2 & s)
{
	for (auto& c : m_components)
	{
		c.ComponentPtr->OnScale(s);
	}
}

void ComponentContainer::OnMove(const gml::vec2 & p)
{
	for (auto& c : m_components)
	{
		c.ComponentPtr->OnMove(p);
	}
}

void ComponentContainer::OnMessage(const g2d::Message & message)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMessage(message);
	}
	DelayRemove();
}

void ComponentContainer::OnUpdate(uint32_t deltaTime)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnUpdate(deltaTime);
	}
	DelayRemove();
}

void ComponentContainer::OnUpdateMatrixChanged()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnUpdateMatrixChanged();
	}
	DelayRemove();
}

void ComponentContainer::OnCursorEnterFrom(::SceneNode * adjacency)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnCursorEnterFrom(adjacency, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnCursorHovering()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnCursorHovering(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnCursorLeaveTo(::SceneNode * adjacency)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnCursorLeaveTo(adjacency, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDoubleClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDoubleClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDoubleClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDoubleClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDoubleClick()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDoubleClick(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDragBegin()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDragBegin(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDragBegin()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDragBegin(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDragBegin()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDragBegin(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDragging()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDragging(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDragging()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDragging(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDragging()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDragging(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDragEnd()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDragEnd(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDragEnd()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDragEnd(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDragEnd()
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDragEnd(GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDropping(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDropping(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDropping(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDropping(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDropping(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDropping(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnLDropTo(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnLDropTo(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnRDropTo(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnRDropTo(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnMDropTo(::SceneNode * dropped)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnMDropTo(dropped, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnKeyPress(g2d::KeyCode key)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnKeyPress(key, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnKeyPressingBegin(g2d::KeyCode key)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnKeyPressingBegin(key, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnKeyPressing(g2d::KeyCode key)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnKeyPressing(key, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}

void ComponentContainer::OnKeyPressingEnd(g2d::KeyCode key)
{
	Collect();
	for (auto& c : m_collection)
	{
		c.ComponentPtr->OnKeyPressingEnd(key, GetMouse(), GetKeyboard());
	}
	DelayRemove();
}


void SceneNodeContainer::OnMessage(const g2d::Message & message)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnMessage(message);
	}
	Collect();
}

void SceneNodeContainer::OnUpdate(uint32_t deltaTime)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnUpdate(deltaTime);
	}
	Collect();
}

void SceneNodeContainer::OnKeyPress(g2d::KeyCode key)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnKeyPress(key);
	}
	Collect();
}

void SceneNodeContainer::OnKeyPressingBegin(g2d::KeyCode key)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnKeyPressingBegin(key);
	}
	Collect();
}

void SceneNodeContainer::OnKeyPressing(g2d::KeyCode key)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnKeyPressing(key);
	}
	Collect();
}

void SceneNodeContainer::OnKeyPressingEnd(g2d::KeyCode key)
{
	// Collecting  must not be process before
	// dispatching, or will cause potential 
	// recursive dispatching
	for (auto& child : m_collection)
	{
		child->OnKeyPressingEnd(key);
	}
	Collect();
}
