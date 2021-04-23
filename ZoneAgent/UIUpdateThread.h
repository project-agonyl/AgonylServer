#pragma once
#include "LoopThread.h"

class CUIUpdateThread : public CLoopThread
{
public:
	CUIUpdateThread();
	BOOL			Start();
private:
	void			LoopProc();
	virtual	void	Process();
	void			OnClose();
};

