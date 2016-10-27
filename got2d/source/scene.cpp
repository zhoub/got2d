#include "scene.h"

g2d::SceneNode::~SceneNode() { }

g2d::Scene::~Scene() { }

::SceneNode::SceneNode(SceneNode* parent)
	: m_parent(parent)
	, m_matrixLocal(gml::mat32::I())
	, m_position(gml::vec2::zero())
	, m_pivot(gml::vec2::zero())
	, m_scale(gml::vec2::one())
	, m_rotationRadian(0)
{

}

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
	if (m_matrixLocalDirty)
	{
		//m_matrixLocal = gml::mat32::trps(m_position, m_rotationRadian, m_pivot, m_scale);
		m_matrixLocal = gml::mat32::trsp(m_position, m_rotationRadian, m_scale, m_pivot);
		m_matrixLocalDirty = false;
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
	auto rst = new QuatNode(this);
	m_children.push_back(rst);
	return rst;
}

void SceneNode::SetPivot(const gml::vec2& pivot)
{
	m_matrixLocalDirty = true;
	m_pivot = pivot;
}
void SceneNode::SetScale(const gml::vec2& scale)
{
	m_matrixLocalDirty = true;
	m_scale = scale;
}
void SceneNode::SetPosition(const gml::vec2& position)
{
	m_matrixLocalDirty = true;
	m_position = position;
}
void SceneNode::SetRotation(float radian)
{
	m_matrixLocalDirty = true;
	m_rotationRadian = radian;
}

#include <g2dengine.h>
QuatNode::QuatNode(SceneNode* parent)
	:SceneNode(parent)
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
	vertices[3].position.set(-0.5f, +0.5f);
	vertices[2].position.set(+0.5f, +0.5f);
	vertices[1].position.set(+0.5f, -0.5f);

	vertices[0].texcoord.set(0, 1);
	vertices[3].texcoord.set(0, 0);
	vertices[2].texcoord.set(1, 0);
	vertices[1].texcoord.set(1, 1);

	vertices[0].vtxcolor = gml::color4::random();
	vertices[1].vtxcolor = gml::color4::random();
	vertices[2].vtxcolor = gml::color4::random();
	vertices[3].vtxcolor = gml::color4::random();

	m_tex = g2d::GetEngine()->LoadTexture((rand() % 2) ? "test_alpha.bmp" : "test_alpha.png");

	SetPivot(gml::vec2(-0.5f, -0.5f));
	SetScale(gml::vec2(100, 120));
}
QuatNode::~QuatNode()
{
	m_mesh->Release();
}
void QuatNode::Render()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(m_mesh, m_tex, GetLocalMatrix());
}

::Scene::Scene()
{
	m_root = new ::SceneNode(nullptr);
}
::Scene::~Scene()
{
	delete m_root;
}