#pragma once
#include <vector>
#include "scene.h"
#include <gmlaabb.h>

class QuadTreeNode
{
public:
	constexpr static float MIN_SIZE = 100.0f;

	QuadTreeNode(gml::aabb2d bounding);
	~QuadTreeNode();
	void Add(g2d::Entity* sceneNode);
	void Remove(g2d::Entity* sceneNode);
	void FindVisible(g2d::Camera* camera, std::vector<g2d::Entity*>& visibleEntities);

private:
	constexpr static int DIR_LT = 0;
	constexpr static int DIR_LD = 1;
	constexpr static int DIR_RT = 2;
	constexpr static int DIR_RD = 3;
	constexpr static int NUM_DIR = 4;

	bool m_hasChildren = false;
	bool m_canCreateChildren = true;
	void CreateChildren();

	bool TryAddRecursive(const gml::aabb2d& nodeAABB, g2d::Entity* node);
	QuadTreeNode* GetDirNode(int id);


	QuadTreeNode* m_directionNodes[NUM_DIR];
	std::vector<g2d::Entity*> m_dynamicEntities;
	gml::aabb2d m_bounding;
};