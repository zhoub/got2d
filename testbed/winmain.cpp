#include "stdafx.h"
#include "framework.h"
#include <cinttypes>

namespace g2d
{
	class Scene;
}

class Testbed
{
public:
	Testbed(Framework& fw) : framework(fw) { }
	void Start();
	void End();
	bool Update(uint32_t elapsedTime);
	void OnMessage(const g2d::Message& message);
private:
	g2d::Scene* mainScene = nullptr;
	Framework& framework;
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
		framework.OnStart = std::bind(&Testbed::Start, &testbed);
		framework.OnFinish = std::bind(&Testbed::End, &testbed);
		using namespace std::placeholders;
		framework.OnUpdate = std::bind(&Testbed::Update, &testbed, _1);
		framework.OnMessage = std::bind(&Testbed::OnMessage, &testbed, _1);
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

g2d::SceneNode* firstNode = nullptr;
g2d::SceneNode* lastNode = nullptr;
void Testbed::Start()
{
	mainScene = g2d::GetEngine()->CreateNewScene(2 << 10);
	
	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto node = mainScene->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 0));
	
	//node.SetVisibleMask(3, true);
	node->SetStatic(true);
	for (int i = 0; i < 5; i++)
	{
		auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
		auto child = node->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 0));

		child->SetVisibleMask((i % 2) ? 1 : 2, true);
		child->SetStatic(true);

		node = child;
	}
	lastNode = node;

	auto mainCamera = mainScene->GetMainCamera();
	mainCamera->SetActivity(true);
	mainCamera->SetScale(gml::vec2(2, 2));

	//测试spatial tree
	if (false)
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

	Hexgon* hexgonEntity = new Hexgon();
	auto hexgonNode = mainScene->CreateSceneNodeChild(hexgonEntity, true);
	hexgonNode->SetPosition({ 0,0 });
	firstNode = hexgonNode;
}

void Testbed::End()
{
	mainScene->Release();
	mainScene = nullptr;
}

bool Testbed::Update(uint32_t elapsedTime)
{
	//测试光标坐标映射
	{
		auto mainCamera = mainScene->GetMainCamera();
		if (0)
		{
			auto mouseP = framework.GetCursorPos();
			auto p = mainCamera->ScreenToWorld(mouseP);
			firstNode->SetPosition(p);
		}
		else if (0)
		{
			auto p = mainCamera->WorldToScreen(lastNode->GetWorldPosition());
			framework.SetCursorPos(p);
		}
		else if(0)
		{
			auto mouseP = framework.GetCursorPos();
			auto p = mainCamera->ScreenToWorld(mouseP);
			p = lastNode->WorldToParent(p);
			lastNode->SetPosition(p);
		}
	}

	mainScene->Update(elapsedTime);

	g2d::GetEngine()->GetRenderSystem()->BeginRender();
	mainScene->Render();
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;
}

void Testbed::OnMessage(const g2d::Message& message)
{
	mainScene->OnMessage(message);
}