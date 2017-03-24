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

void Testbed::Start()
{
	mainScene = g2d::GetEngine()->CreateNewScene(2 << 10);

	Hexgon* hexgonEntity = new Hexgon();
	hexgonNode = mainScene->CreateSceneNodeChild(hexgonEntity, true);
	hexgonNode->SetPosition({ 0,0 });
	hexgonNode->AddComponent(new HexgonColorChanger(), true);
	hexgonNode->AddComponent(new EntityDragging(), true);
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