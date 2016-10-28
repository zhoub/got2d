#pragma once
#include "../include/g2dscene.h"
#include <gmlmatrix.h>
#include <vector>

class QuadEntity;
class SceneNode : public g2d::SceneNode
{
public:
	SceneNode(SceneNode* parent, g2d::Entity* entity, bool autoRelease);
	virtual ~SceneNode();
	void Update(unsigned int elpasedTime);
	void Render();

public:
	virtual g2d::SceneNode* CreateSceneNode(g2d::Entity* entity, bool autoRelease)override;
	virtual const gml::mat32& GetLocalMatrix() override;
	virtual const gml::mat32& GetWorldMatrix() override;
	virtual g2d::SceneNode* SetPosition(const gml::vec2& position)override;
	virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot)override;
	virtual g2d::SceneNode* SetScale(const gml::vec2& scale)override;
	virtual g2d::SceneNode* SetRotation(float radian)override;
	inline virtual const gml::vec2& GetPosition()  const override { return m_position; }
	inline virtual const gml::vec2& GetPivot() const override { return m_pivot; }
	inline virtual const gml::vec2& GetScale() const override { return m_scale; }
	inline virtual float GetRotation() const override { return m_rotationRadian; }
	inline virtual g2d::Entity* GetEntity() const override { return m_entity; }

private:
	void SetLocalMatrixDirty();
	::SceneNode* m_parent = nullptr;
	g2d::Entity* m_entity = nullptr;
	std::vector<::SceneNode*> m_children;
	bool m_autoRelease = false;

	gml::vec2 m_position;
	gml::vec2 m_pivot;
	gml::vec2 m_scale;
	float m_rotationRadian;

	bool m_matrixLocalDirty = false;
	bool m_matrixWorldDirty = true;
	gml::mat32 m_matrixLocal;
	gml::mat32 m_matrixWorld;
};

#include<g2drender.h>
class QuadEntity : public g2d::QuadEntity
{
	IMPL_CLASSID;
public:
	QuadEntity();
	~QuadEntity();
	virtual void OnInitial() override;
	virtual void OnRender() override;

public:
	inline virtual g2d::Entity* SetSize(const gml::vec2& size) override;
	inline virtual const gml::vec2& GetSize() const override { return m_size; }
	inline virtual void Release() { delete this; }

	g2d::Mesh* m_mesh;
	g2d::Material* m_material;
	gml::vec2 m_size;
};

class Scene : public g2d::Scene
{
public:
	Scene();
	~Scene();

	inline ::SceneNode* GetRoot() { return m_root; }
	inline void Update(unsigned int elpasedTime) { return m_root->Update(elpasedTime); }
	inline void Render() { return m_root->Render(); }

public:
	inline virtual g2d::SceneNode* CreateSceneNode(g2d::Entity* e, bool autoRelease) override { return m_root->CreateSceneNode(e,autoRelease); }
	inline virtual const gml::mat32& GetLocalMatrix() override { return m_root->GetLocalMatrix(); }
	inline virtual const gml::mat32& GetWorldMatrix() override { return m_root->GetWorldMatrix(); }
	inline virtual g2d::SceneNode* SetPosition(const gml::vec2& position) override { m_root->SetPosition(position); return this; }
	inline virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot) override { m_root->SetPivot(pivot); return this; }
	inline virtual g2d::SceneNode* SetScale(const gml::vec2& scale) override { m_root->SetScale(scale); return this; }
	inline virtual g2d::SceneNode* SetRotation(float radian) override { m_root->SetRotation(radian); return this; }
	inline virtual const gml::vec2& GetPosition()  const override { return m_root->GetPosition(); }
	inline virtual const gml::vec2& GetPivot() const override { return m_root->GetPivot(); }
	inline virtual const gml::vec2& GetScale() const override { return m_root->GetScale(); }
	inline virtual float GetRotation() const override { return m_root->GetRotation(); }
	inline virtual g2d::Entity* GetEntity() const override { return m_root->GetEntity(); }

public:
	inline virtual g2d::QuadEntity* CreateQuadEntity() override { return new ::QuadEntity; }

private:
	::SceneNode* m_root;
};
