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
	g2d::Scene* mainScene = nullptr;
	Framework& framework;
	g2d::SceneNode* following = nullptr;
	g2d::SceneNode* hexgonNode = nullptr;
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

#include "hexgon.h"
#include <g2dengine.h>
#include <g2drender.h>
#include <g2dscene.h>

g2d::SceneNode* CreateQuadNode(g2d::SceneNode* parent);

class Moveing : public g2d::Component
{
	RTTI_IMPL;
public: //implement
	virtual void Release() override { delete this; }

	virtual void OnKeyPress(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if (key == g2d::KeyCode::Enter && (GetSceneNode()->GetChildCount() < 5 || GetSceneNode()->GetParentNode()->GetParentNode() == GetSceneNode()->GetScene()))
		{
			CreateQuadNode(GetSceneNode());
		}
		else if (key == g2d::KeyCode::Delete && GetSceneNode()->GetChildIndex() == 3)
		{
			GetSceneNode()->Remove();
		}
	}

	virtual void OnKeyPressing(g2d::KeyCode key, const g2d::Mouse& mouse, const g2d::Keyboard& keyboard) override
	{
		if (key == g2d::KeyCode::ArrowLeft)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2(-1.0f, 0));
		}
		else if (key == g2d::KeyCode::ArrowRight)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2(1.0f, 0));
		}
		else if (key == g2d::KeyCode::ArrowUp)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2(0, -1.0f));
		}
		else if (key == g2d::KeyCode::ArrowDown)
		{
			GetSceneNode()->SetPosition(GetSceneNode()->GetPosition() + gml::vec2(0, 1.0f));
		}
	}
};


g2d::SceneNode* CreateQuadNode(g2d::SceneNode* parent)
{
	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto child = parent->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 20));
	child->AddComponent(new Moveing(), true);
	child->SetStatic(false);
	return child;
}


void Testbed::Start()
{
	mainScene = g2d::GetEngine()->CreateNewScene(2 << 10);

	Hexgon* hexgonEntity = new Hexgon();
	hexgonNode = mainScene->CreateSceneNodeChild(hexgonEntity, true);
	hexgonNode->SetPosition({ 0,0 });
	hexgonNode->AddComponent(new HexgonColorChanger(), true);
	hexgonNode->AddComponent(new EntityDragging(), true);

	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto node = mainScene->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 0));

	//node.SetVisibleMask(3, true);
	node->SetStatic(true);
	for (int i = 0; i < 5; i++)
	{
		auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
		auto child = node->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 20));
		child->AddComponent(new Moveing(), true);
		child->SetVisibleMask((i % 2) ? 1 : 2, true);
		child->SetStatic(true);

		node = child;
	}
	following = node;

	auto mainCamera = mainScene->GetMainCamera();
	mainCamera->SetActivity(true);
	mainCamera->SetScale(gml::vec2(2, 2));

	//测试spatial tree
	if (0)
	{
		auto camera = mainScene->CreateCameraNode();
		if (camera)
		{
			camera->SetPosition(gml::vec2(220, 100));
			camera->SetVisibleMask(2);
			camera->SetActivity(false);
		}

		camera = mainScene->CreateCameraNode();
		if (camera)
		{
			camera->SetPosition(gml::vec2(220, 100));
			camera->SetRenderingOrder(-1);
			camera->SetVisibleMask(1);
			camera->SetActivity(false);
		}
	}

	hexgonNode->MoveToFront();
}

void Testbed::End()
{
	mainScene->Release();
	mainScene = nullptr;
}

bool Testbed::Update(uint32_t deltaTime)
{

	g2d::GetEngine()->Update(deltaTime);

	//测试光标坐标映射
	{
		auto mainCamera = mainScene->GetMainCamera();
		if (0)
		{
			auto cursorPos = framework.GetCursorPos();
			auto worldPos = mainCamera->ScreenToWorld(cursorPos);
			hexgonNode->SetPosition(worldPos);
		}
		else if (0)
		{
			auto p = mainCamera->WorldToScreen(following->GetWorldPosition());
			framework.SetCursorPos(p);
		}
		else if (0)
		{
			auto cursorPos = framework.GetCursorPos();
			auto parentPos = mainCamera->ScreenToWorld(cursorPos);
			parentPos = following->WorldToParent(parentPos);
			following->SetPosition(parentPos);
		}
	}

	g2d::GetEngine()->GetRenderSystem()->BeginRender();
	mainScene->Render();
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;
}

void Testbed::OnMessage(const g2d::Message& message)
{
	g2d::GetEngine()->OnMessage(message);
}