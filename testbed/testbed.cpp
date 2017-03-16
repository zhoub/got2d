#include "stdafx.h"
#include "testbed.h"
#include <timeapi.h>
#include <g2dengine.h>
#include <g2drender.h>
#include <g2dscene.h>
#include <time.h>

#include "hexgon.h"

const std::wstring& Testbed::GetWindowClassName()
{
	static const std::wstring name = L"got2d_window_class";
	return name;
}

const std::wstring& Testbed::GetWindowTitle()
{
	static const std::wstring title = L"got2d test bed";
	return title;
}

bool Testbed::InitApp()
{
	srand(static_cast<unsigned int>(time(0)));
	m_elapsedTime = 0;
	m_tickInterval = 1000 / 60;

	char path[MAX_PATH];
	::GetCurrentDirectoryA(MAX_PATH, path);
	std::string p = path;
	p += "/../extern/res/win32_test/";
	g2d::Engine::Config ecfg;
	ecfg.nativeWindow = hWnd;
	ecfg.resourceFolderPath = p.c_str();
	if (!g2d::Engine::Initialize(ecfg))
		return false;

	return true;
}

void Testbed::DestroyApp()
{
	End();
	g2d::Engine::Uninitialize();
}

void Testbed::FirstTick()
{
	m_lastTimeStamp = timeGetTime();
	Start();
}

void Testbed::OnResize(long width, long height)
{
	//要在初始化之后再做这件事情
	if (g2d::IsEngineInitialized())
	{
		g2d::GetEngine()->GetRenderSystem()->OnResize(width, height);
	}
}

//return false表示关闭窗口退出。
bool Testbed::MainLoop()
{
	bool rst = true;
	auto elapseTime = timeGetTime() - m_lastTimeStamp;
	if (elapseTime > m_tickInterval)
	{
		m_elapsedTime += elapseTime;
		m_lastTimeStamp = timeGetTime();
		rst = Update(elapseTime);
		m_frameCount++;
	}
	return rst;
}

void Testbed::QuitApp()
{
	m_running = false;
}

unsigned long Testbed::GetFrameCount() const
{
	return m_frameCount;
}

unsigned long Testbed::GetElapsedTime() const
{
	return m_elapsedTime;
}

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

bool Testbed::Update(unsigned long elapsedTime)
{
	mainScene->Update(elapsedTime);

	g2d::GetEngine()->GetRenderSystem()->BeginRender();
	mainScene->Render();
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;

}