#pragma once
#include <vector>
#include <map>
#include <gmlaabb.h>
#include "inner_utility.h"
#include "scope_utility.h"

class Camera;

class QuadTreeNode
{
public:
	constexpr static float MIN_SIZE = 100.0f;

	QuadTreeNode(QuadTreeNode* parent, const gml::vec2& center, float gridSize);

	~QuadTreeNode();

	QuadTreeNode* AddRecursive(const gml::aabb2d& entityBound, g2d::Entity&  entity);

	QuadTreeNode* AddToDynamicList(g2d::Entity& entity);

	void Remove(g2d::Entity& entity);

	gml::aabb2d GetBounding() { return m_bounding; }

	void FindVisible(Camera& camera);

	bool IsEmpty() { return m_isEmpty; }

private:
	void TryMarkEmpty();

	class Direction
	{
	public:
		constexpr static uint32_t LeftTop = 0;
		constexpr static uint32_t LeftDown = 1;
		constexpr static uint32_t RightTop = 2;
		constexpr static uint32_t RightDown = 3;
		constexpr static uint32_t Count = 4;
	};

	const bool m_kCanBranch = true;
	bool m_isEmpty = true;
	QuadTreeNode* m_parent;
	QuadTreeNode* m_directionNodes[Direction::Count];
	std::vector<g2d::Entity*> m_dynamicEntities;
	gml::aabb2d m_bounding;
};

class SpatialGraph
{
public:
	SpatialGraph(float boundSize);
	
	void Add(g2d::Entity& entity);

	void Remove(g2d::Entity& entity);

	void FindVisible(Camera& camera);

private:
	autod<QuadTreeNode> m_root;
	std::map<g2d::Entity*, QuadTreeNode*> m_linkRef;
};