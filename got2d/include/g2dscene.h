#pragma once
#include <g2dconfig.h>
#include <gmlvector.h>
#include <gmlmatrix.h>
#include <gmlaabb.h>
namespace g2d
{
	class SceneNode;
	class G2DAPI Entity
	{
	public:
		DECL_CLASSID;
		virtual void Release() = 0;
		virtual gml::aabb2d GetLocalAABB() const = 0;
		virtual gml::aabb2d GetWorldAABB() const = 0;
	public:
		virtual ~Entity();
		virtual void OnInitial();
		virtual void OnUpdate(unsigned int elpasedTime);
		virtual void OnRender();
		void SetSceneNode(g2d::SceneNode* node);

	protected:
		SceneNode* GetSceneNode() const;
	private:
		SceneNode* m_sceneNode;
	};

	class G2DAPI QuadEntity : public Entity
	{
	public:
		virtual g2d::Entity* SetSize(const gml::vec2& size) = 0;
		virtual const gml::vec2& GetSize()  const = 0;
	};

	class G2DAPI Camera : public Entity
	{
	public:
		virtual Camera* SetPosition(const gml::vec2& position) = 0;
		virtual Camera* SetScale(const gml::vec2& scale) = 0;
		virtual Camera* SetRotation(float radian) = 0;
		virtual const gml::mat32& GetViewMatrix() const = 0;
	};

	class G2DAPI SceneNode
	{
	public:
		virtual ~SceneNode();
		virtual SceneNode* CreateSceneNode(Entity*, bool autoRelease) = 0;
		virtual const gml::mat32& GetLocalMatrix() = 0;
		virtual const gml::mat32& GetWorldMatrix() = 0;
		virtual SceneNode* SetPosition(const gml::vec2& position) = 0;
		virtual SceneNode* SetPivot(const gml::vec2& pivot) = 0;
		virtual SceneNode* SetScale(const gml::vec2& scale) = 0;
		virtual SceneNode* SetRotation(float radian) = 0;
		virtual void SetVisible(bool) = 0;
		virtual const gml::vec2& GetPosition()  const = 0;
		virtual const gml::vec2& GetPivot() const = 0;
		virtual const gml::vec2& GetScale() const = 0;
		virtual float GetRotation() const = 0;
		virtual Entity* GetEntity() const = 0;
		virtual bool IsVisible() const = 0;
	};

	template<class T> T* GetEntity(SceneNode* node)
	{
		Entity* entity = node->GetEntity();
		return (T::GetClassID() == entity->GetClassID()) ? entity : nullptr;
	}

	class G2DAPI Scene : public SceneNode
	{
	public:
		virtual ~Scene();
		virtual Camera* CreateCamera() = 0;
		virtual QuadEntity* CreateQuadEntity() = 0;
		virtual void SetCamera(Camera* camera) = 0;
	};
}
