// LoopThread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////
//#include <windows.h>

#if !defined(_LOOP_THREAD_HREAD_H__NCLUDED_)
#define _LOOP_THREAD_HREAD_H__NCLUDED_


class CLoopThread
{
public:
	CLoopThread();
	virtual		~CLoopThread();

	virtual	BOOL	BeginThread();
	virtual	void	CloseThread();

	virtual	void	LoopProc();
	virtual	void	Process();

protected:

	static	unsigned int __stdcall	ThreadFunction(void* lpParam);

	BOOL	m_bRunThread;
	HANDLE	m_hThread;
	DWORD	m_dwThreadID;
};


#endif // !defined(_LOOP_THREAD_HREAD_H__NCLUDED_)
