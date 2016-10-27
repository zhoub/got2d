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
		virtual void SetPosition(const gml::vec2& position) = 0;
		virtual void SetPivot(const gml::vec2& pivot) = 0;
		virtual void SetScale(const gml::vec2& scale) = 0;
		virtual void SetRotation(float radian) = 0;
		virtual const gml::vec2& GetPosition()  const = 0;
		virtual const gml::vec2& GetPivot() const = 0;
		virtual const gml::vec2& GetScale() const = 0;
		virtual float GetRotation() const = 0;
	};

	class G2DAPI Scene : public SceneNode
	{
	public:
		~Scene();
	};
}
