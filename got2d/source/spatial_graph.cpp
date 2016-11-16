#include "spatial_graph.h"
#include <assert.h>
QuadTreeNode::QuadTreeNode(const gml::vec2& center, float gridSize)
	: m_bounding(gml::vec2(center.x - gridSize, center.y - gridSize), gml::vec2(center.x + gridSize, center.y + gridSize))
	, m_canCreateChildren(gridSize > MIN_SIZE)
{
	for (auto& dirNode : m_directionNodes)
	{
		dirNode = nullptr;
	}
	m_canCreateChildren = gridSize > MIN_SIZE;
}
QuadTreeNode::~QuadTreeNode()
{
	for (auto& dirNode : m_directionNodes)
	{
		if (dirNode)
		{
			delete dirNode;
		}
	}
}

void QuadTreeNode::Add(g2d::Entity* entity)
{
	bool needPush = true;
	if (entity->GetSceneNode()->IsStatic())
	{
		gml::aabb2d nodeAABB = entity->GetWorldAABB();
		auto intersection = m_bounding.is_intersect(nodeAABB);
		if (intersection == gml::it_contain)
		{
			needPush = !TryAddRecursive(nodeAABB, entity);
		}
	}

	if (needPush)
	{
		::SceneNode* nodeImpl = dynamic_cast<::SceneNode*>(entity->GetSceneNode());
		nodeImpl->SetSpatialNode(this);
		m_dynamicEntities.push_back(entity);
	}
}

void QuadTreeNode::Remove(g2d::Entity* entity)
{
	for (auto it = m_dynamicEntities.begin(), end = m_dynamicEntities.end(); it != end; it++)
	{
		if (*it == entity)
		{
			m_dynamicEntities.erase(it);
			break;
		}
	}
}

inline bool Contain(const gml::vec2& center, float gridSize, const gml::aabb2d& nodeAABB)
{
	gml::aabb2d bounding(
		gml::vec2(center.x - gridSize, center.y - gridSize),
		gml::vec2(center.x + gridSize, center.y + gridSize)
		);

	return bounding.is_intersect(nodeAABB) == gml::it_contain;
}
bool QuadTreeNode::TryAddRecursive(const gml::aabb2d& nodeAABB, g2d::Entity* entity)
{
	if (m_canCreateChildren)
	{
		bool pushed = false;
		gml::aabb2d childBound;
		gml::vec2 center;
		float halfExtend = m_bounding.extend().x * 0.5f;

		do
		{
			//4 quad
			center = m_bounding.center();
			center.x -= halfExtend;
			center.y += halfExtend;
			if (Contain(center, halfExtend, nodeAABB))
			{
				if (m_directionNodes[DIR_LT] == nullptr)
				{
					m_directionNodes[DIR_LT] = new QuadTreeNode(center, halfExtend);
				}
				if (m_directionNodes[DIR_LT]->TryAddRecursive(nodeAABB, entity))
				{
					pushed = true;
					break;
				}
			}

			center = m_bounding.center();
			center.x -= halfExtend;
			center.y -= halfExtend;
			if (Contain(center, halfExtend, nodeAABB))
			{
				if (m_directionNodes[DIR_LD] == nullptr)
				{
					m_directionNodes[DIR_LD] = new QuadTreeNode(center, halfExtend);
				}
				if (m_directionNodes[DIR_LD]->TryAddRecursive(nodeAABB, entity))
				{
					pushed = true;
					break;
				}
			}

			center = m_bounding.center();
			center.x += halfExtend;
			center.y += halfExtend;
			if (Contain(center, halfExtend, nodeAABB))
			{
				if (m_directionNodes[DIR_RT] == nullptr)
				{
					m_directionNodes[DIR_RT] = new QuadTreeNode(center, halfExtend);
				}
				if (m_directionNodes[DIR_RT]->TryAddRecursive(nodeAABB, entity))
				{
					pushed = true;
					break;
				}
			}

			center = m_bounding.center();
			center.x += halfExtend;
			center.y -= halfExtend;
			if (Contain(center, halfExtend, nodeAABB))
			{
				if (m_directionNodes[DIR_RD] == nullptr)
				{
					m_directionNodes[DIR_RD] = new QuadTreeNode(center, halfExtend);
				}
				if (m_directionNodes[DIR_RD]->TryAddRecursive(nodeAABB, entity))
				{
					pushed = true;
					break;
				}
			}
		} while (false);

		if (!pushed)
		{
			::SceneNode* nodeImpl = dynamic_cast<::SceneNode*>(entity->GetSceneNode());
			nodeImpl->SetSpatialNode(this);
			m_dynamicEntities.push_back(entity);
		}
		return true;
	}
	else
	{
		return false;
	}
}


QuadTreeNode* QuadTreeNode::GetDirNode(int index)
{
	assert(index >= 0 && index < NUM_DIR);
	return m_directionNodes[index];
}

void QuadTreeNode::FindVisible(g2d::Camera* camera, std::vector<g2d::Entity*>& visibleEntities)
{
	if (camera->TestVisible(m_bounding))
	{
		for (auto& entity : m_dynamicEntities)
		{
			if (entity->GetSceneNode()->IsVisible() && camera->TestVisible(entity))
			{
				visibleEntities.push_back(entity);
			}
		}

		for (auto& child : m_directionNodes)
		{
			if (child != nullptr)
			{
				child->FindVisible(camera, visibleEntities);
			}
		}
	}
}