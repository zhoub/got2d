#pragma once
#include "../include/g2dscene.h"
#include <gmlmatrix.h>
#include <vector>
class SceneNode : public g2d::SceneNode
{
public:
	SceneNode(SceneNode* parent);
	~SceneNode();

	const gml::mat32& GetLocalMatrix();
	virtual void Update(unsigned int elpasedTime);
	virtual void Render();

public:
	virtual g2d::SceneNode* CreateQuadNode() override;

	virtual void SetPosition(const gml::vec2& position) override;
	virtual void SetPivot(const gml::vec2& pivot) override;
	virtual void SetScale(const gml::vec2& scale) override;
	virtual void SetRotation(float radian) override;
	inline virtual const gml::vec2& GetPosition()  const override { return m_position; }
	inline virtual const gml::vec2& GetPivot() const override { return m_pivot; }
	inline virtual const gml::vec2& GetScale() const override { return m_scale; }
	inline virtual float GetRotation() const override { return m_rotationRadian; }

private:
	SceneNode* m_parent = nullptr;
	std::vector<::SceneNode*> m_children;

	gml::vec2 m_position;
	gml::vec2 m_pivot;
	gml::vec2 m_scale;
	float m_rotationRadian;

	bool m_matrixLocalDirty = false;
	gml::mat32 m_matrixLocal;
};

#include<g2drender.h>
class QuatNode : public SceneNode
{
public:
	QuatNode(SceneNode* parent);
	~QuatNode();
	virtual void Render() override;
	g2d::Mesh* m_mesh;
	g2d::Texture* m_tex;
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
	inline virtual g2d::SceneNode* CreateQuadNode() override { return m_root->CreateQuadNode(); }

	inline virtual void SetPosition(const gml::vec2& position) override { m_root->SetPosition(position); }
	inline virtual void SetPivot(const gml::vec2& pivot) override { m_root->SetPivot(pivot); };
	inline virtual void SetScale(const gml::vec2& scale) override { m_root->SetScale(scale); };
	inline virtual void SetRotation(float radian) override { m_root->SetRotation(radian); };
	inline virtual const gml::vec2& GetPosition()  const override { return m_root->GetPosition(); };
	inline virtual const gml::vec2& GetPivot() const override { return m_root->GetPivot(); };
	inline virtual const gml::vec2& GetScale() const override { return m_root->GetScale(); };
	inline virtual float GetRotation() const override { return m_root->GetRotation(); };

private:
	::SceneNode* m_root;
};
