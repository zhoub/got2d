#pragma once
#include "../include/g2dscene.h"
#include <gmlmatrix.h>
#include <vector>

class QuadNode;
class SceneNode
{
public:
	SceneNode(SceneNode* parent);
	virtual ~SceneNode();
	const gml::mat32& GetLocalMatrix();
	const gml::mat32& GetWorldMatrix();
	void Update(unsigned int elpasedTime);
	void Render();

public:
	virtual void OnUpdate() {}
	virtual void OnRender() {}

public:
	QuadNode* _CreateQuadNode();
	SceneNode* _SetPosition(const gml::vec2& position);
	SceneNode* _SetPivot(const gml::vec2& pivot);
	SceneNode* _SetScale(const gml::vec2& scale);
	SceneNode* _SetRotation(float radian);
	inline const gml::vec2& _GetPosition()  const { return m_position; }
	inline const gml::vec2& _GetPivot() const { return m_pivot; }
	inline const gml::vec2& _GetScale() const { return m_scale; }
	inline float _GetRotation() const { return m_rotationRadian; }

private:
	void SetLocalMatrixDirty();
	::SceneNode* m_parent = nullptr;
	std::vector<::SceneNode*> m_children;

	gml::vec2 m_position;
	gml::vec2 m_pivot;
	gml::vec2 m_scale;
	float m_rotationRadian;

	bool m_matrixLocalDirty = false;
	bool m_matrixWorldDirty = true;
	gml::mat32 m_matrixLocal;
	gml::mat32 m_matrixWorld;
};

#define IMPL_SCENE_NODE(member) \
	inline virtual g2d::QuadNode* CreateQuadNode() override { return member->_CreateQuadNode(); } \
	inline virtual g2d::SceneNode* SetPosition(const gml::vec2& position) override { member->_SetPosition(position); return this;} \
	inline virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot) override { member->_SetPivot(pivot); return this; } \
	inline virtual g2d::SceneNode* SetScale(const gml::vec2& scale) override { member->_SetScale(scale); return this; } \
	inline virtual g2d::SceneNode* SetRotation(float radian) override { member->_SetRotation(radian); return this; } \
	inline virtual const gml::vec2& GetPosition()  const override { return member->_GetPosition(); } \
	inline virtual const gml::vec2& GetPivot() const override { return member->_GetPivot(); } \
	inline virtual const gml::vec2& GetScale() const override { return member->_GetScale(); } \
	inline virtual float GetRotation() const override { return member->_GetRotation(); } \

#include<g2drender.h>
class QuadNode : public g2d::QuadNode, public ::SceneNode
{
public:
	QuadNode(::SceneNode* parent);
	~QuadNode();
	virtual void OnRender() override;

public:
	inline virtual g2d::QuadNode* SetSize(const gml::vec2& size) override;
	inline virtual const gml::vec2& GetSize() const override { return m_size; }

public:
	IMPL_SCENE_NODE(this);

	g2d::Mesh* m_mesh;
	g2d::Texture* m_tex;
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
	IMPL_SCENE_NODE(m_root);

private:
	::SceneNode* m_root;
};
