#include "scene.h"
#include <g2dengine.h>
#include <gmlconversion.h>

g2d::Quad* g2d::Quad::Create()
{
	return new ::Quad();
}

gml::aabb2d g2d::Entity::GetWorldAABB() const
{
	if (GetLocalAABB().is_point())
		return GetLocalAABB();

	auto matrixWorld = GetSceneNode()->GetWorldMatrix();
	return gml::transform(matrixWorld, GetLocalAABB());
}

void g2d::Entity::SetSceneNode(g2d::SceneNode* node)
{
	m_sceneNode = node;
}

void g2d::Entity::SetRenderingOrder(uint32_t order)
{
	m_renderingOrder = order;
}

uint32_t g2d::Entity::GetVisibleMask() const
{
	return GetSceneNode()->GetVisibleMask();
}

void g2d::Component::SetSceneNode(g2d::SceneNode* node)
{
	m_sceneNode = node;
}

Quad::Quad()
{
	uint32_t indices[] = { 0, 1, 2, 0, 2, 3 };
	m_mesh = g2d::Mesh::Create(4, 6);
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
		m_material = g2d::Material::CreateSimpleColor();
		break;
	case 1:
		m_material = g2d::Material::CreateSimpleTexture();
		m_material->GetPassByIndex(0)->SetTexture(0, g2d::Texture::LoadFromFile((rand() % 2) ? "test_alpha.bmp" : "test_alpha.png"), true);
		break;
	case 2:
		m_material = g2d::Material::CreateColorTexture();
		m_material->GetPassByIndex(0)->SetTexture(0, g2d::Texture::LoadFromFile((rand() % 2) ? "test_alpha.bmp" : "test_alpha.png"), true);
		break;
	}
}

void Quad::OnInitial()
{
}

void Quad::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(
		g2d::RenderLayer::Default,
		m_mesh,
		m_material,
		GetSceneNode()->GetWorldMatrix());
}

g2d::Quad* Quad::SetSize(const gml::vec2& size)
{
	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	vertices[0].position.set(-0.5f, -0.5f);
	vertices[3].position.set(-0.5f, +0.5f);
	vertices[2].position.set(+0.5f, +0.5f);
	vertices[1].position.set(+0.5f, -0.5f);

	vertices[0].position *= size;
	vertices[3].position *= size;
	vertices[2].position *= size;
	vertices[1].position *= size;

	m_aabb.expand(gml::vec2(-0.5f, -0.5f) * size);
	m_aabb.expand(gml::vec2(+0.5f, +0.5f) * size);
	return this;
}

void Camera::OnUpdateMatrixChanged()
{
	auto pos = GetSceneNode()->GetPosition();
	auto scale = GetSceneNode()->GetScale();
	auto rotation = GetSceneNode()->GetRotation();
	m_matView = gml::mat32::inv_trs(pos, rotation, scale);
	m_matViewInverse = gml::mat33(m_matView).inversed();

	auto rs = g2d::GetEngine()->GetRenderSystem();
	gml::vec2 halfSize(rs->GetWindowWidth() * 0.5f, rs->GetWindowHeight()* 0.5f);

	halfSize.x *= scale.x;
	halfSize.y *= scale.y;
	gml::vec2 p[4] =
	{
		-halfSize, halfSize,
		{+halfSize.x, -halfSize.y },
		{-halfSize.x, +halfSize.y }
	};

	m_aabb.empty();
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
	::Scene* scene = reinterpret_cast<::Scene*>(GetSceneNode()->GetScene());
	scene->SetCameraOrderDirty();
}

bool Camera::TestVisible(const gml::aabb2d& bounding) const
{
	return (m_aabb.is_intersect(bounding) != gml::it_mode::none);
}

bool Camera::TestVisible(g2d::Entity& entity) const
{
	if (IsSameType(&entity) ||
		entity.GetLocalAABB().is_point() ||
		(GetVisibleMask() & entity.GetVisibleMask()) == 0)
	{
		return false;
	}

	return TestVisible(entity.GetWorldAABB());
}

g2d::Entity* Camera::FindIntersectionObject(const gml::vec2& worldPosition)
{
	auto itCur = std::rbegin(visibleEntities);
	auto itEnd = std::rend(visibleEntities);
	for (; itCur != itEnd; itCur++)
	{
		Entity* entity = *itCur;
		auto localPos = entity->GetSceneNode()->WorldToLocal(worldPosition);
		if (entity->GetLocalAABB().contains(localPos))
		{
			return entity;
		}
	}
	return nullptr;
}

gml::vec2 Camera::ScreenToWorld(const gml::coord& pos) const
{
	auto renderSystem = g2d::GetEngine()->GetRenderSystem();
	auto viewPos = renderSystem->ScreenToView(pos);
	return gml::transform_point(m_matViewInverse, viewPos);
}

gml::coord Camera::WorldToScreen(const gml::vec2& pos) const
{
	auto renderSystem = g2d::GetEngine()->GetRenderSystem();
	auto viewPos = gml::transform_point(m_matView, pos);
	return renderSystem->ViewToScreen(viewPos);
}