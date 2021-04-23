#include "stdafx.h"
#include "UIUpdateThread.h"

CUIUpdateThread::CUIUpdateThread(){}

BOOL CUIUpdateThread::Start()
{
	return BeginThread();
}

void CUIUpdateThread::LoopProc()
{
	CLoopThread::LoopProc();
}
void CUIUpdateThread::Process()
{
	ReloadServerList();
	Sleep(1000);
}

void CUIUpdateThread::OnClose()
{
	LoopProc();
}