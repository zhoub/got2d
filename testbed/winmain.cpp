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
	void Start();
	void End();
	bool Update(uint32_t elapsedTime);
	void OnMessage(const g2d::Message& message);
private:
	g2d::Scene* mainScene = nullptr;
};


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Framework framework(hInstance);
	Testbed testbed;

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

void Testbed::Start()
{
	mainScene = g2d::GetEngine()->CreateNewScene(2 << 10);

	auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
	auto node = mainScene->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(100, 100));

	//node.SetVisibleMask(3, true);
	node->SetStatic(true);
	for (int i = 0; i <= 4; i++)
	{
		auto quad = g2d::Quad::Create()->SetSize(gml::vec2(100, 120));
		auto child = node->CreateSceneNodeChild(quad, true)->SetPosition(gml::vec2(50, 50));
		child->SetVisibleMask((i % 2) ? 1 : 2, true);
		child->SetStatic(true);

		node = child;
	}

	if (false)//测试spatial tree 不需要
	{
		auto mainCamera = mainScene->GetMainCamera();
		mainCamera->SetActivity(true);

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
	hexgonNode->SetPosition({ 10,10 });
}

void Testbed::End()
{
	mainScene->Release();
	mainScene = nullptr;
}

bool Testbed::Update(uint32_t elapsedTime)
{
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