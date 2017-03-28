#include "stdafx.h"
#include "hexagon.h"
#include <gmlconversion.h>

constexpr float kHexagonMargin = 2.0f;
constexpr float kHexagonSize = 30.0f;
constexpr float kHexagonRealSize = kHexagonSize - kHexagonMargin;
constexpr float kHeightStride = kHexagonSize * 1.5f;
const float kWidthStride = kHexagonSize * sqrt(3.0f);

g2d::Mesh * CreateHexagonMesh(float size, gml::color4 color, gml::aabb2d* aabb)
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
			vertices[i].position = v * (size);
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

void Hexagon::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(
		g2d::RenderLayer::Default,
		m_mesh, m_material,
		GetSceneNode()->GetWorldMatrix());
}

Hexagon::Hexagon()
{
	m_mesh = CreateHexagonMesh(kHexagonRealSize, gml::color4::random(), &m_aabb);
	m_material = g2d::Material::CreateSimpleColor();
}

Hexagon::~Hexagon()
{
	m_mesh->Release();
	m_material->Release();
}

void Hexagon::GetColors(std::vector<gml::color4>& outColors)
{
	if (outColors.size() < 7) outColors.resize(7);

	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	for (int i = 0; i < 7; i++)
	{
		outColors[i] = vertices[i].vtxcolor;
	}
}

void Hexagon::SetColors(const std::vector<gml::color4>& colors)
{
	int count = __min((int)colors.size(), 7);
	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	for (int i = 0; i < count; i++)
	{
		vertices[i].vtxcolor = colors[i];
	}
}

void Hexagon::SetColors(const gml::color4 & color)
{
	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	for (int i = 0; i < 7; i++)
	{
		vertices[i].vtxcolor = color;
	}
}

void HexagonBoard::OnInitial()
{
	GetSceneNode()->SetStatic(true);
}

inline void HexagonBoard::OnRender()
{
	g2d::GetEngine()->GetRenderSystem()->RenderMesh(
		g2d::RenderLayer::Default,
		m_mesh, m_material,
		GetSceneNode()->GetWorldMatrix());
}

inline void HexagonBoard::OnCursorHovering(const g2d::Mouse & mouse, const g2d::Keyboard & keyboard)
{
	auto cursor = mouse.GetCursorPosition();
	auto pos = GetSceneNode()->GetScene()->GetMainCamera()->ScreenToWorld(cursor);
	pos = GetSceneNode()->WorldToLocal(pos);
	int x, y;
	PositionToHex(pos, x, y);
	SetHexagonColor(gml::color4::random(), x, y);
}

HexagonBoard::HexagonBoard()
{

	m_aabb.empty();
	gml::aabb2d HexagonAABB;
	gml::color4 boardColor = gml::color4::gray();
	gml::mat32 transform = gml::mat32::identity();


	auto HexagonMesh = CreateHexagonMesh(kHexagonSize - kHexagonMargin, boardColor, &HexagonAABB);
	m_mesh = g2d::Mesh::Create(0, 0);

	for (int line = 5; line < 10; line++)
	{
		float y = (line - 9) * kHeightStride;
		float xOffset = (line % 2 == 0) ? kWidthStride * 0.5f : 0.0f;
		for (int h = 0; h < line; h++)
		{
			float x = (h - line / 2) * kWidthStride + xOffset;
			transform = gml::mat32::translate(x, y);
			m_mesh->Merge(HexagonMesh, transform);
			auto hexagonWorldAABB = gml::transform(transform, HexagonAABB);
			m_aabb.expand(hexagonWorldAABB);
		}
	}

	for (int line = 8; line >= 5; line--)
	{
		float y = (9 - line) * kHeightStride;
		float xOffset = (line % 2 == 0) ? kWidthStride * 0.5f : 0.0f;
		for (int h = 0; h < line; h++)
		{
			float x = (h - line / 2) * kWidthStride + xOffset;
			transform = gml::mat32::translate(x, y);
			m_mesh->Merge(HexagonMesh, transform);
			auto hexagonWorldAABB = gml::transform(transform, HexagonAABB);
			m_aabb.expand(hexagonWorldAABB);
		}
	}

	HexagonMesh->Release();

	m_material = g2d::Material::CreateSimpleColor();
}

HexagonBoard::~HexagonBoard()
{
	m_mesh->Release();
	m_material->Release();
}

void HexagonBoard::SetHexagonColor(gml::color4 color, int q, int r)
{
	int index = HexToIndex(q, r);
	g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
	auto beg = index * 7;
	auto end = (index + 1) * 7;
	for (int i = beg; i < end; i++)
	{
		vertices[i].vtxcolor = color;
	}
}

void HexagonBoard::PositionToHex(gml::vec2 pos, int & outQ, int & outR)
{
	float x = (pos.x * sqrt(3.0f) / 3.0f - pos.y / 3.0f) / kHexagonSize;
	float z = pos.y * 2.0f / 3.0f / kHexagonSize;
	float y = -x - z;

	int ix = round(x);
	int iy = round(y);
	int iz = round(z);

	float xdiff = abs(x - ix);
	float ydiff = abs(y - iy);
	float zdiff = abs(z - iz);

	if (xdiff > ydiff && xdiff > zdiff)
	{
		outQ = -iy - iz;
		outR = iz;
	}
	else if (ydiff > zdiff)
	{
		outQ = ix;
		outR = iz;
	}
	else
	{
		outQ = ix;
		outR = -ix - iy;
	}
}

int HexagonBoard::HexToIndex(int q, int r)
{
	int x = gml::clamp(q, -4, 4);
	int z = gml::clamp(r, -4, 4);
	int y = gml::clamp(-x - z, -4, 4);
	x = -z - y;
	int absz = abs(z);

	int index = 0;
	if (z <= 0)
	{
		return  (4 - y) + (8 - absz + 5) * (4 - absz) / 2;
	}
	else
	{
		return  (x + 4) + 35 + (9 - absz + 9) * (absz - 1) / 2;
	}
}
