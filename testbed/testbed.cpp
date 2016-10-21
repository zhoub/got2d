#include "stdafx.h"
#include "testbed.h"
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

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
	m_elapsedTime = 0;
	m_tickInterval = 1000 / 60;

	m_engine = g2d::CreateEngine();
	if (m_engine == nullptr)
		return false;
	return true;
}

void Testbed::DestroyApp()
{
	End();
	if (m_engine)
	{
		m_engine->Release();
		m_engine = nullptr;
	}
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
	return m_engine->Update(elapsedTime);
}