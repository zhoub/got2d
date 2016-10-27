#pragma once
#include <g2dconfig.h>
#include <gmlvector.h>
namespace g2d
{
	class QuadNode;
	class G2DAPI SceneNode
	{
	public:
		~SceneNode();

		virtual QuadNode* CreateQuadNode() = 0;
		virtual SceneNode* SetPosition(const gml::vec2& position) = 0;
		virtual SceneNode* SetPivot(const gml::vec2& pivot) = 0;
		virtual SceneNode* SetScale(const gml::vec2& scale) = 0;
		virtual SceneNode* SetRotation(float radian) = 0;
		virtual const gml::vec2& GetPosition()  const = 0;
		virtual const gml::vec2& GetPivot() const = 0;
		virtual const gml::vec2& GetScale() const = 0;
		virtual float GetRotation() const = 0;
	};

	class G2DAPI QuadNode : public SceneNode
	{
	public:
		virtual QuadNode* SetSize(const gml::vec2& size) = 0;
		virtual const gml::vec2& GetSize()  const = 0;
	};

	class G2DAPI Scene : public SceneNode
	{
	public:
		~Scene();
	};
}
