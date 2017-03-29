#include "stdafx.h"
#include "framework.h"
#include <cinttypes>

namespace g2d
{
	class Scene;
	class SceneNode;
}

class Testbed
{
public:
	Testbed(Framework& fw) : framework(fw) { }
	void Start();
	void End();
	bool Update(uint32_t deltaTime);
	void OnMessage(const g2d::Message& message);
private:
	Framework& framework;

	g2d::Scene* mainScene = nullptr;
	g2d::SceneNode* HexagonNode = nullptr;
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Framework framework(hInstance);
	Testbed testbed(framework);

	// 注册事件
	{
		framework.OnStart = [&] { testbed.Start(); };
		framework.OnFinish = [&] { testbed.End(); };
		framework.OnUpdate = [&](uint32_t deltaTime)->bool { return testbed.Update(deltaTime); };
		framework.OnMessage = [&](const g2d::Message& message) { testbed.OnMessage(message); };
	}

	// 运行程序
	if (framework.Initial(nCmdShow, "/../extern/res/win32_test/"))
	{
		return framework.Start();
	}
	return 0;
}

#include "hexagon.h"
#include <g2dengine.h>
#include <g2drender.h>
#include <g2dscene.h>

g2d::SceneNode* CreateQuadNode(g2d::SceneNode* parent);

class KeyboardMoving : public g2d::Component
{
	RTTI_IMPL;
public: //implement
	virtual void Release() override { delete this; }

	virtual void OnKeyPress(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if (key == g2d::KeyCode::Enter && (GetSceneNode()->GetChildCount() < 5 || GetSceneNode()->GetParentNode() == nullptr))
		{
			CreateQuadNode(GetSceneNode());
		}
		else if (key == g2d::KeyCode::Delete && GetSceneNode()->GetChildIndex() == 3)
		{
			GetSceneNode()->Release();
		}
	}

	virtual void OnKeyPressing(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if (key == g2d::KeyCode::ArrowLeft)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2::left());
		}
		else if (key == g2d::KeyCode::ArrowRight)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2::right());
		}
		else if (key == g2d::KeyCode::ArrowUp)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2::up());
		}
		else if (key == g2d::KeyCode::ArrowDown)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2::down());
		}
	}
};


g2d::SceneNode* CreateQuadNode(g2d::SceneNode* parent)
{
	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto child = parent->CreateChild()->SetPosition(gml::vec2(50, 20));
	child->AddComponent(quad, true);
	child->AddComponent(new KeyboardMoving(), true);
	child->SetStatic(false);
	return child;
}


void Testbed::Start()
{
	mainScene = g2d::GetEngine()->CreateNewScene(2 << 10);

	// board
	auto boardNode = mainScene->CreateChild();
	boardNode->SetPosition({ -200.0f, 0.0f });
	boardNode->AddComponent(new HexagonBoard(), true);

	//hexgon node
	HexagonNode = mainScene->CreateChild();
	HexagonNode->SetPosition({ 0,0 });
	HexagonNode->AddComponent(new Hexagon(), true);
	HexagonNode->AddComponent(new EntityDragging(), true);

	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto node = mainScene->CreateChild()->SetPosition(gml::vec2(300, 0));
	node->AddComponent(quad, true);
	node->SetStatic(true);

	for (int i = 0; i < 5; i++)
	{
		auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
		auto child = node->CreateChild()->SetPosition(gml::vec2(50, 60));
		child->AddComponent(quad, true);
		child->AddComponent(new KeyboardMoving(), true);
		child->SetStatic(true);
		node = child;
	}

	HexagonNode->MoveToFront();
}

void Testbed::End()
{
	mainScene->Release();
	mainScene = nullptr;
}

bool Testbed::Update(uint32_t deltaTime)
{
	g2d::GetEngine()->Update(deltaTime);
	g2d::GetEngine()->GetRenderSystem()->BeginRender();
	mainScene->Render();
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;
}

void Testbed::OnMessage(const g2d::Message& message)
{
	g2d::GetEngine()->OnMessage(message);
}