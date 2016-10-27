#include "stdafx.h"
#include "testbed.h"
#include <timeapi.h>
#include <g2dengine.h>
#include <g2drender.h>
#include <g2dscene.h>
#include <time.h>
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
	g2d::EngineConfig ecfg;
	ecfg.nativeWindow = hWnd;
	ecfg.resourceFolderPath = p.c_str();
	if (!g2d::InitEngine(ecfg))
		return false;

	auto* node = g2d::GetEngine()->GetCurrentScene()->CreateQuadNode()->SetSize(gml::vec2(100, 120))->SetPosition(gml::vec2(100, 100));
	for (int i = 0; i < 4; i++)
	{
		auto child = node->CreateQuadNode()->SetSize(gml::vec2(100, 120))->SetPosition(gml::vec2(50, 50));
		node = child;
	}
	return true;
}

void Testbed::DestroyApp()
{
	End();
	g2d::UninitEngine();
}

void Testbed::FirstTick()
{
	m_lastTimeStamp = timeGetTime();
	Start();
}

void Testbed::OnResize(long width, long height)
{
	//要在初始化之后再做这件事情
	if (g2d::GetEngine())
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

}

void Testbed::End()
{

}

bool Testbed::Update(unsigned long elapsedTime)
{
	if (!g2d::GetEngine()->Update(elapsedTime))
		return false;

	g2d::GetEngine()->GetRenderSystem()->BeginRender();
	g2d::GetEngine()->Render();
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;

}