#pragma once
#include "../include/g2dscene.h"
#include <gmlmatrix.h>
#include <vector>
class SceneNode : public g2d::SceneNode
{
public:
	SceneNode();
	~SceneNode();

	const gml::mat32& GetLocalMatrix();
	virtual void Update(unsigned int elpasedTime);
	virtual void Render();

public:
	virtual g2d::SceneNode* CreateQuadNode() override;
	virtual void SetPosition(const gml::vec2 position) override;

private:
	std::vector<::SceneNode*> m_children;

	gml::vec2 m_localPosition;

	bool m_matrixDirty = false;
	gml::mat32 m_matrixLocal;
};

#include<g2drender.h>
class QuatNode : public SceneNode
{
public:
	QuatNode();
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
	inline virtual void SetPosition(const gml::vec2 position) override { return m_root->SetPosition(position); }

private:
	::SceneNode* m_root;
};
