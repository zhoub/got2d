#pragma once
#include <g2dengine.h>
#include <g2dscene.h>
#include <g2drender.h>
#include <vector>


g2d::Mesh* CreateHexgonMesh(gml::color4 color, gml::aabb2d* aabb);

class Hexgon : public g2d::Component
{
	RTTI_IMPL;
public://implement
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; };

	virtual void OnRender() override
	{
		g2d::GetEngine()->GetRenderSystem()->RenderMesh(
			g2d::RenderLayer::Default,
			m_mesh, m_material,
			GetSceneNode()->GetWorldMatrix());
	}

public:
	Hexgon()
	{
		m_mesh = CreateHexgonMesh(gml::color4::random(), &m_aabb);
		m_material = g2d::Material::CreateSimpleColor();
	}

	~Hexgon()
	{
		m_mesh->Release();
		m_material->Release();
	}

	void GetColors(std::vector<gml::color4>& outColors)
	{
		if (outColors.size() < 7) outColors.resize(7);

		g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
		for (int i = 0; i < 7; i++)
		{
			outColors[i] = vertices[i].vtxcolor;
		}
	}

	void SetColors(const std::vector<gml::color4>& colors)
	{
		int count = __min((int)colors.size(), 7);
		g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
		for (int i = 0; i < count; i++)
		{
			vertices[i].vtxcolor = colors[i];
		}
	}

	void SetColors(const gml::color4& color)
	{
		g2d::GeometryVertex* vertices = m_mesh->GetRawVertices();
		for (int i = 0; i < 7; i++)
		{
			vertices[i].vtxcolor = color;
		}
	}

private:
	gml::aabb2d m_aabb;
	g2d::Mesh* m_mesh;
	g2d::Material* m_material;
};

class HexgonBoard : public g2d::Component
{
	RTTI_IMPL;
public:
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; };

	virtual void OnRender() override;

	HexgonBoard();

	~HexgonBoard();

private:
	gml::aabb2d m_aabb;
	g2d::Mesh* m_mesh;
	g2d::Material* m_material;
};


class EntityDragging : public g2d::Component
{
	RTTI_IMPL;
public: //implement
	virtual void Release() override { delete this; }

	virtual void OnLDragBegin(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		auto worldP = GetSceneNode()->GetScene()->GetMainCamera()->ScreenToWorld(mouse.GetCursorPosition());
		m_dragOffset = GetSceneNode()->WorldToLocal(worldP);
	}

	virtual void OnLDragging(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		auto worldP = GetSceneNode()->GetScene()->GetMainCamera()->ScreenToWorld(mouse.GetCursorPosition());
		auto parentP = GetSceneNode()->WorldToParent(worldP);
		GetSceneNode()->SetPosition(parentP - m_dragOffset);
	}
	gml::vec2 m_dragOffset;
};

class HexgonColorChanger : public g2d::Component
{
	RTTI_IMPL;
public: //implement
	virtual void Release() override
	{
		if (dragComponent != nullptr && autoReleased)
		{
			dragComponent->Release();
			dragComponent = nullptr;
		}
		delete this;
	}

public: //events
	virtual void OnInitial() override
	{
		colors.resize(7);
		hexgonEntity = g2d::FindComponent<Hexgon>(GetSceneNode());
		hexgonEntity->GetColors(colors);
	}

	virtual void OnLClick(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		for (int i = 0; i < 7; i++)
		{
			colors[i] = gml::color4::random();
		}
	}

	virtual void OnCursorEnterFrom(g2d::SceneNode* adjacency, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		hexgonEntity->GetColors(colors);
	}
	virtual void OnCursorHovering(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		hexgonEntity->SetColors(gml::color4::red());
	}

	virtual void OnCursorLeaveTo(g2d::SceneNode* adjacency, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		hexgonEntity->SetColors(colors);
	}

	virtual void OnLDoubleClick(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		auto component = g2d::FindComponent<EntityDragging>(GetSceneNode());
		if (component != nullptr)
		{
			dragComponent = component;
			autoReleased = GetSceneNode()->IsComponentAutoRelease(component);
			GetSceneNode()->RemoveComponentWithoutRelease(component);

			for (int i = 0; i < 7; i++)
			{
				colors[i] = gml::color4::green();
			}
		}
		else
		{
			GetSceneNode()->AddComponent(dragComponent, autoReleased);
			dragComponent = nullptr;

			for (int i = 0; i < 7; i++)
			{
				colors[i] = gml::color4::random();
			}
		}
	}


	virtual void OnLDragBegin(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		hexgonEntity->SetColors(gml::color4::yellow());
	}

	virtual void OnLDragEnd(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		hexgonEntity->SetColors(colors);
	}

	virtual void OnKeyPress(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if ((int)key == 'C')
		{
			for (int i = 0; i < 7; i++)
			{
				colors[i] = gml::color4::random();
			}
			hexgonEntity->SetColors(colors);
		}
	}

	Hexgon* hexgonEntity;
	std::vector<gml::color4> colors;
	EntityDragging*  dragComponent = nullptr;
	bool autoReleased = false;
};