#pragma once
#include <vector>
#include <map>
#include <gml/gmlaabb.h>
#include "scope_utility.h"

class Camera;

class QuadTreeNode
{
public:
	constexpr static float MIN_SIZE = 100.0f;

	QuadTreeNode(QuadTreeNode* parent, const gml::vec2& center, float gridSize);

	~QuadTreeNode();

	QuadTreeNode* AddRecursive(const gml::aabb2d& bounds, g2d::Component& component);

	QuadTreeNode* AddToList(g2d::Component& component);

	void Remove(g2d::Component& component);

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
	std::vector<g2d::Component*> m_components;
	gml::aabb2d m_bounding;
};

class SpatialGraph
{
public:
	SpatialGraph(float boundSize);

	void Add(g2d::Component& component);

	void Remove(g2d::Component& component);

	void FindVisible(Camera& camera);

private:
	autod<QuadTreeNode> m_root;
	std::map<g2d::Component*, QuadTreeNode*> m_linkRef;
};
