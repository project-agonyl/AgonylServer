// LoopThread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <process.h>
#include "LoopThread.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoopThread::CLoopThread()
{
	m_bRunThread = FALSE;
	m_hThread = INVALID_HANDLE_VALUE;
}

CLoopThread::~CLoopThread()
{
	CloseThread();
}

unsigned int __stdcall CLoopThread::ThreadFunction(void* lpParam)
{
	CLoopThread* pThis = (CLoopThread*)lpParam;
	pThis->LoopProc();

	return 0;
}

BOOL CLoopThread::BeginThread()
{
	if (m_bRunThread) return FALSE;

	m_hThread = (HANDLE)_beginthreadex(NULL,
		NULL,
		ThreadFunction,
		this,
		NULL,
		(UINT*)&m_dwThreadID);

	if (m_hThread == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}

void CLoopThread::LoopProc()
{
	m_bRunThread = TRUE;

	while (m_bRunThread)
	{
		Process();

		Sleep(1);
	}
}

void CLoopThread::Process()
{
}

void CLoopThread::CloseThread()
{
	if (m_bRunThread == FALSE)	return;

	m_bRunThread = FALSE;

	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
}
