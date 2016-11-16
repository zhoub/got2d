#pragma once
#include <vector>
#include <map>
#include <gmlaabb.h>

namespace g2d
{
	class Entity;
	class Camera;
}
class QuadTreeNode
{
public:
	constexpr static float MIN_SIZE = 100.0f;

	QuadTreeNode(const gml::vec2& center, float gridSize);
	~QuadTreeNode();

	QuadTreeNode* AddRecursive(const gml::aabb2d& entityBound, g2d::Entity* entity);
	QuadTreeNode* AddToDynamicList(g2d::Entity* entity);
	void Remove(g2d::Entity* entity);
	inline gml::aabb2d GetBounding() { return m_bounding; }
	void FindVisible(g2d::Camera* camera, std::vector<g2d::Entity*>& visibleEntities);

private:
	constexpr static int DIR_LT = 0;
	constexpr static int DIR_LD = 1;
	constexpr static int DIR_RT = 2;
	constexpr static int DIR_RD = 3;
	constexpr static int NUM_DIR = 4;
	const bool m_kCanBranch = true;

	QuadTreeNode* m_directionNodes[NUM_DIR];
	std::vector<g2d::Entity*> m_dynamicEntities;
	gml::aabb2d m_bounding;
};

class SpatialGraph
{
public:
	SpatialGraph(float boundSize);
	~SpatialGraph();
	void Add(g2d::Entity* entity);
	void Remove(g2d::Entity* entity);
	void FindVisible(g2d::Camera* camera, std::vector<g2d::Entity*>& visibleEntities);

private:
	QuadTreeNode* m_root;
	std::map<g2d::Entity*, QuadTreeNode*> m_linkRef;
};