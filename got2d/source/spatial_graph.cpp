#include "spatial_graph.h"
#include "../include/g2dscene.h"
#include "component.h"
#include <algorithm>

QuadTreeNode::QuadTreeNode(QuadTreeNode* parent, const gml::vec2& center, float gridSize)
	: m_bounding(
		gml::vec2(center.x - gridSize, center.y - gridSize),
		gml::vec2(center.x + gridSize, center.y + gridSize))
	, m_kCanBranch(gridSize > MIN_SIZE)
	, m_parent(parent)
{
	for (auto& dirNode : m_directionNodes)
	{
		dirNode = nullptr;
	}
}
QuadTreeNode::~QuadTreeNode()
{
	for (auto& dirNode : m_directionNodes)
	{
		if (dirNode)
		{
			delete dirNode;
			dirNode = nullptr;
		}
	}
}

inline bool Contains(const gml::vec2& center, float gridSize, const gml::aabb2d& nodeAABB)
{
	gml::aabb2d bounding(
		gml::vec2(center.x - gridSize, center.y - gridSize),
		gml::vec2(center.x + gridSize, center.y + gridSize)
	);

	return bounding.is_intersect(nodeAABB) == gml::it_mode::contain;
}

QuadTreeNode* QuadTreeNode::AddRecursive(const gml::aabb2d& bounds, g2d::Component& component)
{
	m_isEmpty = false;
	if (m_kCanBranch)
	{
		//try to push child node
		//push to self if failed.

		//WE HERE NEED aabb::move() !
		float extend = m_bounding.extend().x;
		float halfExtend = extend * 0.5f;
		auto& center = m_bounding.center();
		//x-neg, y-pos
		gml::aabb2d bounding(
			gml::vec2(center.x - extend, center.y + extend),
			m_bounding.center());

		if (bounding.is_intersect(bounds) == gml::it_mode::contain)
		{
			if (m_directionNodes[Direction::LeftTop] == nullptr)
			{
				m_directionNodes[Direction::LeftTop] = new QuadTreeNode(this, center, halfExtend);
			}
			return m_directionNodes[Direction::LeftTop]->AddRecursive(bounds, component);
		}

		//x-neg, y-neg
		bounding.move({ 0, -extend });
		if (bounding.is_intersect(bounds) == gml::it_mode::contain)
		{
			if (m_directionNodes[Direction::LeftDown] == nullptr)
			{
				m_directionNodes[Direction::LeftDown] = new QuadTreeNode(this, center, halfExtend);
			}
			return  m_directionNodes[Direction::LeftDown]->AddRecursive(bounds, component);
		}

		//x-pos,y-pos
		bounding.move({ extend, extend });
		if (bounding.is_intersect(bounds) == gml::it_mode::contain)
		{
			if (m_directionNodes[Direction::RightTop] == nullptr)
			{
				m_directionNodes[Direction::RightTop] = new QuadTreeNode(this, center, halfExtend);
			}
			return m_directionNodes[Direction::RightTop]->AddRecursive(bounds, component);
		}

		//x-pos, y-neg
		bounding.move({ 0, -extend });
		if (bounding.is_intersect(bounds) == gml::it_mode::contain)
		{
			if (m_directionNodes[Direction::RightDown] == nullptr)
			{
				m_directionNodes[Direction::RightDown] = new QuadTreeNode(this, center, halfExtend);
			}
			return m_directionNodes[Direction::RightDown]->AddRecursive(bounds, component);
		}

		return AddToList(component);
	}
	else
	{
		return AddToList(component);
	}
}

QuadTreeNode* QuadTreeNode::AddToList(g2d::Component& component)
{
	m_isEmpty = false;
	m_components.push_back(&component);
	return this;
}

void QuadTreeNode::TryMarkEmpty()
{
	if (m_components.size() > 0)
	{
		return;
	}

	bool hasEntities = false;
	for (auto child : m_directionNodes)
	{
		if (child && !child->IsEmpty())
		{
			hasEntities = true;
			break;
		}
	}

	if (!hasEntities)
	{
		m_isEmpty = true;
		if (m_parent)
		{
			m_parent->TryMarkEmpty();
		}
	}
}
void QuadTreeNode::Remove(g2d::Component& component)
{
	auto oldEnd = std::end(m_components);
	auto newEnd = std::remove(std::begin(m_components), oldEnd, &component);
	m_components.erase(newEnd, oldEnd);

	TryMarkEmpty();
}

void QuadTreeNode::FindVisible(Camera& camera)
{
	if (IsEmpty())
		return;

	for (auto& component : m_components)
	{
		if (component->GetSceneNode()->IsVisible() && camera.TestVisible(*component))
		{
			camera.visibleComponents.push_back(component);
		}
	}

	for (auto& child : m_directionNodes)
	{
		if (child != nullptr && camera.TestVisible(child->GetBounding()))
		{
			child->FindVisible(camera);
		}
	}
}

SpatialGraph::SpatialGraph(float boundSize)
{
	m_root = new QuadTreeNode(nullptr, gml::vec2::zero(), boundSize);
}

void SpatialGraph::Add(g2d::Component& component)
{
	if (component.GetClassID() == ::Camera::GetStaticClassID())
		return;

	Remove(component);

	QuadTreeNode* node = nullptr;
	if (component.GetSceneNode()->IsStatic())
	{
		gml::aabb2d nodeAABB = component.GetWorldAABB();
		node = m_root->AddRecursive(nodeAABB, component);
	}
	else
	{
		// FOR HINT: m_root->m_isEmpty = false;
		node = m_root->AddToList(component);
	}
	m_linkRef[&component] = node;
}

void SpatialGraph::Remove(g2d::Component& component)
{
	if (component.GetClassID() == ::Camera::GetStaticClassID())
		return;

	if (m_linkRef.count(&component))
	{
		auto& node = m_linkRef[&component];
		node->Remove(component);
		m_linkRef.erase(&component);
	}
}

void SpatialGraph::FindVisible(Camera& camera)
{
	m_root->FindVisible(camera);
}