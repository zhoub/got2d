#include "spatial_graph.h"
#include <assert.h>
QuadTreeNode::QuadTreeNode(gml::aabb2d bounding)
	: m_bounding(bounding)
{
	for (auto& dirNode : m_directionNodes)
	{
		dirNode = nullptr;
	}
}

void QuadTreeNode::Add(SceneNode* sceneNode)
{
	bool needPush = false;
	if (sceneNode->IsStatic())
	{
		gml::aabb2d nodeAABB = sceneNode->GetEntity()->GetWorldAABB();
		needPush = !TryAddRecursive(nodeAABB, sceneNode);
	}
	else
	{
		needPush = true;
	}

	if (needPush)
	{
		sceneNode->SetSpatialNode(this);
		m_dynamicNodes.push_back(sceneNode);
	}
}

void QuadTreeNode::Remove(SceneNode* sceneNode)
{
	for (auto it = m_dynamicNodes.begin(), end = m_dynamicNodes.end(); it != end; it++)
	{
		if (*it == sceneNode)
		{
			m_dynamicNodes.erase(it);
			break;
		}
	}
}

bool QuadTreeNode::TryAddRecursive(const gml::aabb2d& nodeAABB, SceneNode* sceneNode)
{
	CreateChildren();
	if (m_hasChildren)
	{
		auto intersection = m_bounding.is_intersect(nodeAABB);
		if (intersection == gml::it_contain)
		{
			bool pushed = false;
			//try push to child node.
			for (int i = 0; i < NUM_DIR; i++)
			{
				QuadTreeNode* dirNode = GetDirNode(i);
				if (dirNode != nullptr)
				{
					if (dirNode->TryAddRecursive(nodeAABB, sceneNode))
					{
						pushed = true;
						break;
					}
				}
			}

			if (!pushed)
			{
				sceneNode->SetSpatialNode(this);
				m_dynamicNodes.push_back(sceneNode);
			}
			return true;
		}
		else
		{
			return false;
		}
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

void QuadTreeNode::CreateChildren()
{
	if (!m_canCreateChildren)
		return;
	if (m_hasChildren)
		return;

	auto rect = m_bounding.max_bound() - m_bounding.min_bound();
	rect *= 0.5f;
	bool widthEnough = rect.x > MIN_SIZE;
	bool heightEnough = rect.y > MIN_SIZE;
	if (!widthEnough && !heightEnough)
	{
		m_canCreateChildren = false;
		m_hasChildren = false;
	}
	else
	{
		m_hasChildren = true;
		gml::aabb2d childBound;
		gml::vec2 min_bound, max_bound;
		gml::vec2 halfExtend = m_bounding.extend();
		//4 quad
		if (widthEnough && heightEnough)
		{
			min_bound = m_bounding.min_bound();
			min_bound.y += halfExtend.y;
			max_bound = min_bound + halfExtend;
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_LT] = new QuadTreeNode(childBound);

			min_bound = m_bounding.min_bound();
			max_bound = min_bound + halfExtend;
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_LD] = new QuadTreeNode(childBound);

			min_bound = m_bounding.min_bound() + halfExtend;
			max_bound = min_bound + halfExtend;
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_RT] = new QuadTreeNode(childBound);

			min_bound = m_bounding.min_bound();
			min_bound.x += halfExtend.x;
			max_bound = min_bound + halfExtend;
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_RD] = new QuadTreeNode(childBound);
		}
		//left-right
		else if (widthEnough)
		{
			min_bound = m_bounding.min_bound();
			max_bound.set(min_bound.x + halfExtend.x, m_bounding.max_bound().y);
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_LT] = new QuadTreeNode(childBound);

			min_bound.set(m_bounding.min_bound().x + halfExtend.x, m_bounding.min_bound().y);
			max_bound = m_bounding.max_bound();
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_RT] = new QuadTreeNode(childBound);
		}
		//top-bottom
		else
		{
			min_bound = m_bounding.min_bound();
			max_bound = min_bound + halfExtend;
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_LT] = new QuadTreeNode(childBound);

			min_bound.set(min_bound.x, m_bounding.min_bound().y + halfExtend.y);
			max_bound = m_bounding.max_bound();
			childBound.set(min_bound, max_bound);
			m_directionNodes[DIR_LD] = new QuadTreeNode(childBound);
		}
	}

}