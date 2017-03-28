#pragma once
#include <g2dengine.h>
#include <g2dscene.h>
#include <g2drender.h>
#include <vector>


g2d::Mesh* CreateHexagonMesh(float size, gml::color4 color, gml::aabb2d* aabb);

class Hexagon : public g2d::Component
{
	RTTI_IMPL;
public://implement
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; };

	virtual void OnRender() override;

public:
	Hexagon();

	~Hexagon();

	void GetColors(std::vector<gml::color4>& outColors);

	void SetColors(const std::vector<gml::color4>& colors);

	void SetColors(const gml::color4& color);

private:
	gml::aabb2d m_aabb;
	g2d::Mesh* m_mesh;
	g2d::Material* m_material;
};

class HexagonBoard : public g2d::Component
{
	RTTI_IMPL;
public:
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; };

	virtual void OnInitial() override;

	virtual void OnRender() override;

	virtual void OnCursorHovering(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override;

public:
	HexagonBoard();

	~HexagonBoard();

	void SetHexagonColor(gml::color4 color, int q, int r);

private:
	void PositionToHex(gml::vec2, int& outQ, int& outR);

	int HexToIndex(int q, int r);

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

class HexagonColorChanger : public g2d::Component
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
		HexagonEntity = g2d::FindComponent<Hexagon>(GetSceneNode());
		HexagonEntity->GetColors(colors);
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
		HexagonEntity->GetColors(colors);
	}
	virtual void OnCursorHovering(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		HexagonEntity->SetColors(gml::color4::red());
	}

	virtual void OnCursorLeaveTo(g2d::SceneNode* adjacency, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		HexagonEntity->SetColors(colors);
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
		HexagonEntity->SetColors(gml::color4::yellow());
	}

	virtual void OnLDragEnd(const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		HexagonEntity->SetColors(colors);
	}

	virtual void OnKeyPress(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if ((int)key == 'C')
		{
			for (int i = 0; i < 7; i++)
			{
				colors[i] = gml::color4::random();
			}
			HexagonEntity->SetColors(colors);
		}
	}

	Hexagon* HexagonEntity;
	std::vector<gml::color4> colors;
	EntityDragging*  dragComponent = nullptr;
	bool autoReleased = false;
};