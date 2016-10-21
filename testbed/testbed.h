#pragma once
#include <string>
#include <windows.h>

extern HINSTANCE hInst;
extern HWND hWnd;

class Testbed
{
public:
	const std::wstring& GetWindowClassName();
	const std::wstring& GetWindowTitle();

	bool InitApp();
	void DestroyApp();
	void FirstTick();
	bool MainLoop();//return false表示关闭窗口退出。

protected:
	void QuitApp();
	unsigned long GetFrameCount() const;
	unsigned long GetElapsedTime() const;

	virtual void Start();
	virtual void End();
	virtual void Update(unsigned long);
	
private:
	unsigned long m_frameCount = 0;
	unsigned long m_elapsedTime = 0;
	unsigned long m_lastTimeStamp;
	unsigned long m_tickInterval;
	bool m_running = true;
};