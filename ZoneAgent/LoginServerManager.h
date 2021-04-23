#pragma once
#include "../HpSocket/TcpClient.h"
#include "../HpSocket/Global/helper.h"
#include "LoopThread.h"

class CLoginServerManager : public CTcpClientListener, public CLoopThread
{
public:
	CLoginServerManager();
	BOOL Start();
	BOOL	Send(void* buf, int len);

private:
	virtual EnHandleResult OnSend(IClient* pClient, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(IClient* pClient, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(IClient* pClient);
	virtual EnHandleResult OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnConnect(IClient* pClient);

	void			LoopProc();
	virtual	void	Process();
	void			OnClose();
	BOOL			GetMsgHeader(MSG_DFLT_HEADER* pMsgHeader, const BYTE* pData, int nLength);
	BOOL			m_bRunServer = FALSE;

	CTcpClient m_Client;
};

