#include "scene.h"

g2d::SceneNode::~SceneNode() { }

g2d::Scene::~Scene() { }

::SceneNode::SceneNode() : m_matrixLocal(gml::mat32::I()) {}

::SceneNode::~SceneNode()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
}

const gml::mat32& SceneNode::GetLocalMatrix()
{
	if (m_matrixDirty)
	{
		m_matrixLocal = gml::mat32::translate(m_localPosition.x, m_localPosition.y);
		m_matrixDirty = false;
	}
	return m_matrixLocal;
}
void SceneNode::Update(unsigned int elpasedTime)
{
	for (auto& child : m_children)
	{
		child->Update(elpasedTime);
	}
}

void SceneNode::Render()
{
	for (auto& child : m_children)
	{
		child->Render();
	}
}

g2d::SceneNode* SceneNode::CreateQuadNode()
{
	auto rst = new QuatNode();
	m_children.push_back(rst);
	return rst;
}

void SceneNode::SetPosition(const gml::vec2 position)
{
	m_matrixDirty = true;
	m_localPosition = position;
}

#include <g2dengine.h>
QuatNode::QuatNode()
{
	unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
	m_mesh = g2d::GetEngine()->GetRenderSystem()->CreateMesh(4, 6);
	auto idx = m_mesh->GetRawIndices();

	for (int i = 0; i < 6; i++)
	{
		idx[i] = indices[i];
	}

	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	vertices[0].position.set(-0.5f, -0.5f);
	vertices[1].position.set(-0.5f, +0.5f);
	vertices[2].position.set(-0.5f + 0.1f, +0.5f);
	vertices[3].position.set(-0.5f + 0.1f, -0.5f);

	vertices[0].texcoord.set(0, 0);
	vertices[1].texcoord.set(0, 1);
	vertices[2].texcoord.set(1, 1);
	vertices[3].texcoord.set(1, 0);

	vertices[0].vtxcolor = gml::color4::random();
	vertices[1].vtxcolor = gml::color4::random();
	vertices[2].vtxcolor = gml::color4::random();
	vertices[3].vtxcolor = gml::color4::random();
}
QuatNode::~QuatNode()
{
	m_mesh->Release();
}
void QuatNode::Render()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(m_mesh, nullptr, GetLocalMatrix());
}

::Scene::Scene()
{
	m_root = new ::SceneNode();
}
::Scene::~Scene()
{

}