#pragma once
#include <functional>
#include <string>
#include <cinttypes>
#include <windows.h>

extern HINSTANCE hInst;
extern HWND hWnd;

namespace g2d
{
	class Mesh;
	class Scene;
}

class Testbed
{
public:
	std::function<void()> OnStart = nullptr;
	std::function<void()> OnFinish = nullptr;
	std::function<bool(uint32_t)> OnUpdate = nullptr;

public:
	const std::wstring& GetWindowClassName();
	const std::wstring& GetWindowTitle();

	bool InitApp();
	void DestroyApp();
	void FirstTick();
	void OnResize(long width, long height);
	bool MainLoop();//return false表示关闭窗口退出。

protected:
	void QuitApp();
	unsigned long GetFrameCount() const;
	unsigned long GetElapsedTime() const;

private:
	unsigned long m_frameCount = 0;
	unsigned long m_elapsedTime = 0;
	unsigned long m_lastTimeStamp;
	unsigned long m_tickInterval;
	bool m_running = true;
};