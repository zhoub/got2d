#include "scene.h"
#include <g2dengine.h>
#include <gmlconv.h>

g2d::Entity::Entity()
	: m_sceneNode(nullptr)
{

}

g2d::Entity::~Entity() {}

void g2d::Entity::OnInitial() {}

void g2d::Entity::OnUpdate(unsigned int elpasedTime) {}

void g2d::Entity::OnRender() {}

void g2d::Entity::OnRotate(float radian) {}

void g2d::Entity::OnScale(const gml::vec2 newScaler) {}

void g2d::Entity::OnMove(const gml::vec2 newPos) {}

void g2d::Entity::OnUpdateMatrixChanged() {}

void g2d::Entity::SetSceneNode(g2d::SceneNode* node)
{
	m_sceneNode = node;
}

unsigned int g2d::Entity::GetVisibleMask() const
{
	if (GetSceneNode())
	{
		return GetSceneNode()->GetVisibleMask();
	}
	return DEFAULT_VISIBLE_MASK;
}

g2d::SceneNode* g2d::Entity::GetSceneNode() const { return m_sceneNode; }

Quad::Quad()
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
void Quad::OnInitial()
{
	GetSceneNode()->SetPivot(gml::vec2(-0.5f, -0.5f));
}
Quad::~Quad()
{
	m_mesh->Release();
	m_material->Release();
}
void Quad::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(g2d::RenderOrder::RORDER_DEFAULT, m_mesh, m_material, GetSceneNode()->GetWorldMatrix());
}
g2d::Entity* Quad::SetSize(const gml::vec2& size)
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

gml::aabb2d Quad::GetWorldAABB() const
{
	if (m_aabb.is_empty())
		return m_aabb;

	auto matrixWorld = GetSceneNode()->GetWorldMatrix();
	return gml::transform(matrixWorld, m_aabb);
}

void Camera::OnUpdate(unsigned int elapsedTime)
{
	float realt = elapsedTime * 0.001f;
	auto pos = GetSceneNode()->GetPosition();
	GetSceneNode()->SetPosition(gml::vec2(pos.x + 10 * realt, pos.y));
	GetSceneNode()->SetScale(gml::vec2(1.0f + 0.1f*realt, 1.0f + 0.1f*realt));
}

void Camera::OnUpdateMatrixChanged()
{
	auto pos = GetSceneNode()->GetPosition();
	auto scale = GetSceneNode()->GetScale();
	auto rotation = GetSceneNode()->GetRotation();
	m_matrix = gml::mat32::inv_trs(pos, rotation, scale);

	auto rs = g2d::GetEngine()->GetRenderSystem();
	gml::vec2 halfSize(rs->GetWindowWidth() * 0.5f, rs->GetWindowHeight()* 0.5f);

	halfSize.x /= scale.x;
	halfSize.y /= scale.y;
	gml::vec2 p[4] =
	{
		-halfSize, halfSize,
		{+halfSize.x, -halfSize.y },
		{-halfSize.x, +halfSize.y }
	};

	m_aabb.clear();
	auto r = gml::mat22::rotate(rotation);
	for (int i = 0; i < 4; i++)
	{
		p[i] = r * p[i];
		m_aabb.expand(pos + p[i]);
	}
}

void Camera::SetRenderingOrder(int renderingOrder)
{
	m_renderingOrder = renderingOrder;
	::Scene* scene = dynamic_cast<::Scene*>(GetSceneNode()->GetScene());
	scene->SetRenderingOrderDirty();
}
bool Camera::TestVisible(g2d::Entity* entity)
{
	if (entity->GetLocalAABB().is_empty() || (GetVisibleMask() &entity->GetVisibleMask()) == 0)
	{
		return false;
	}

	auto& entityAABB = entity->GetWorldAABB();
	return (m_aabb.is_intersect(entityAABB) != gml::it_none);

}