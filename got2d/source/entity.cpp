#include "scene.h"
#include <g2dengine.h>
#include <gmlconv.h>

g2d::Entity::~Entity() {}

void g2d::Entity::OnInitial() {}

void g2d::Entity::OnUpdate(unsigned int elpasedTime) {}

void g2d::Entity::OnRender() {}

void g2d::Entity::SetSceneNode(g2d::SceneNode* node)
{
	m_sceneNode = node;
}

g2d::SceneNode* g2d::Entity::GetSceneNode() const { return m_sceneNode; }

QuadEntity::QuadEntity()
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
	m_aabb.expand(gml::vec2(-0.5f, -0.5f));
	m_aabb.expand(gml::vec2(+0.5f, +0.5f));

	vertices[0].texcoord.set(0, 1);
	vertices[3].texcoord.set(0, 0);
	vertices[2].texcoord.set(1, 0);
	vertices[1].texcoord.set(1, 1);

	vertices[0].vtxcolor = gml::color4::random();
	vertices[1].vtxcolor = gml::color4::random();
	vertices[2].vtxcolor = gml::color4::random();
	vertices[3].vtxcolor = gml::color4::random();

	switch ((rand() % 3))
	{
	case 0:
		m_material = g2d::GetEngine()->GetRenderSystem()->CreateSimpleColorMaterial();
		break;
	case 1:
		m_material = g2d::GetEngine()->GetRenderSystem()->CreateSimpleTextureMaterial();
		m_material->GetPass(0)->SetTexture(0, g2d::GetEngine()->LoadTexture((rand() % 2) ? "test_alpha.bmp" : "test_alpha.png"), true);
		break;
	case 2:
		m_material = g2d::GetEngine()->GetRenderSystem()->CreateColorTextureMaterial();
		m_material->GetPass(0)->SetTexture(0, g2d::GetEngine()->LoadTexture((rand() % 2) ? "test_alpha.bmp" : "test_alpha.png"), true);
		break;
	}
}
void QuadEntity::OnInitial()
{
	GetSceneNode()->SetPivot(gml::vec2(-0.5f, -0.5f));
}
QuadEntity::~QuadEntity()
{
	m_mesh->Release();
	m_material->Release();
}
void QuadEntity::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(g2d::RenderOrder::RORDER_DEFAULT, m_mesh, m_material, GetSceneNode()->GetWorldMatrix());
}
g2d::Entity* QuadEntity::SetSize(const gml::vec2& size)
{
	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	vertices[0].position.set(-0.5f, -0.5f) *= size;
	vertices[3].position.set(-0.5f, +0.5f) *= size;
	vertices[2].position.set(+0.5f, +0.5f) *= size;
	vertices[1].position.set(+0.5f, -0.5f) *= size;

	m_aabb.expand(gml::vec2(-0.5f, -0.5f) * size);
	m_aabb.expand(gml::vec2(+0.5f, +0.5f) * size);
	return this;
}

gml::aabb2d QuadEntity::GetWorldAABB() const
{
	if (m_aabb.is_empty())
		return m_aabb;

	auto matrixWorld = GetSceneNode()->GetWorldMatrix();
	return gml::transform(matrixWorld, m_aabb);
}

void Camera::OnUpdate(unsigned int elapsedTime)
{
	static unsigned int t = 0;
	t += elapsedTime;

	float realt = t * 0.002f;
	float cost = cos(realt);
	float sint = sin(realt);

	GetSceneNode()->SetScale(gml::vec2(1.0f + 0.1f*realt, 1.0f + 0.1f*realt));

	auto pos = GetSceneNode()->GetPosition();
	auto scale = GetSceneNode()->GetScale();
	auto rotation = GetSceneNode()->GetRotation();

	m_matrix = gml::mat32::inv_trs(pos, rotation, scale);
	return;
}