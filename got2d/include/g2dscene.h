#pragma once
#include <g2dconfig.h>
#include <gmlvector.h>
namespace g2d
{
	class G2DAPI SceneNode
	{
	public:
		~SceneNode();

		virtual SceneNode* CreateQuadNode() = 0;
		virtual void SetPosition(const gml::vec2 position) = 0;
	};

	class G2DAPI Scene : public SceneNode
	{
	public:
		~Scene();
	};
}
