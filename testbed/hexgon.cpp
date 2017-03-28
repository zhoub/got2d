#include "stdafx.h"
#include "hexgon.h"

g2d::Mesh * CreateHexgonMesh(gml::color4 color, gml::aabb2d* aabb)
{
	auto mesh = g2d::Mesh::Create(7, 6 * 3);

	//indices;
	{
		uint32_t indices[] = {
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 5,
			0, 5, 6,
			0, 6, 1 };

		auto indexPtr = mesh->GetRawIndices();
		memcpy(indexPtr, indices, sizeof(uint32_t) * mesh->GetIndexCount());
	}

	//vertices
	{
		g2d::GeometryVertex* vertices = mesh->GetRawVertices();

		vertices[0].position.set(0.0f, 0.0f);
		vertices[0].texcoord.set(0.5f, 0.5f);
		vertices[0].vtxcolor = color;

		for (int i = 1; i < 7; i++)
		{
			gml::vec2 v(0, 1);
			float d = (i - 1) * 360.0f / 6;
			v = gml::mat22::rotate((gml::radian)gml::degree(d)) *  v;
			vertices[i].position = v * 50;
			vertices[i].texcoord = v;
			vertices[i].vtxcolor = color;

			if (aabb != nullptr)
			{
				aabb->expand(vertices[i].position);
			}
		}
	}
	return mesh;
}

inline void HexgonBoard::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(
		g2d::RenderLayer::Default,
		m_mesh, m_material,
		GetSceneNode()->GetWorldMatrix());
}

HexgonBoard::HexgonBoard()
{
	m_aabb.empty();
	gml::aabb2d hexgonAABB;
	gml::color4 boardColor = gml::color4::gray();
	gml::mat32 tranform = gml::mat32::identity();

	auto hexgonMesh = CreateHexgonMesh(boardColor, &hexgonAABB);
	m_mesh = g2d::Mesh::Create(0, 0);

	m_mesh->Merge(hexgonMesh, tranform);
	m_aabb.expand(hexgonAABB);

	hexgonMesh->Release();

	m_material = g2d::Material::CreateSimpleColor();
}

HexgonBoard::~HexgonBoard()
{
	m_mesh->Release();
	m_material->Release();
}
