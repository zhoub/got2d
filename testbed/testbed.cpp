#include "stdafx.h"
#include "testbed.h"
#include <timeapi.h>
#include <g2dengine.h>
#include <g2drender.h>
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

	g2d::EngineConfig ecfg;
	ecfg.nativeWindow = hWnd;
	if (!g2d::InitEngine(ecfg))
		return false;



	unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
	for (auto& m : m_meshs)
	{
		m = g2d::GetEngine()->GetRenderSystem()->CreateMesh(4, 6);
		auto idx = m->GetRawIndices();

		for (int i = 0; i < 6; i++)
		{
			idx[i] = indices[i];
		}

		static int mi = 0;
		g2d::GeometryVertex* vertices = m->GetRawVertices();
		float offset = -0.5f + (mi++) * 0.2f;
		vertices[0].position.set(-0.5f + offset, -0.5f);
		vertices[1].position.set(-0.5f + offset, +0.5f);
		vertices[2].position.set(-0.5f + offset + 0.1f, +0.5f);
		vertices[3].position.set(-0.5f + offset + 0.1f, -0.5f);

		vertices[0].vtxcolor = gml::color4::random();
		vertices[1].vtxcolor = gml::color4::random();
		vertices[2].vtxcolor = gml::color4::random();
		vertices[3].vtxcolor = gml::color4::random();
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
	for (auto& m : m_meshs)
	{
		g2d::GetEngine()->GetRenderSystem()->RenderMesh(m);
	}
	g2d::GetEngine()->GetRenderSystem()->EndRender();
	return true;

}