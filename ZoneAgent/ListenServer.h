#pragma once
#include "../HpSocket/TcpServer.h"
#include "../HpSocket/Global/helper.h"

class CListenServer : public CTcpServerListener
{
public:
	CListenServer();
	BOOL					Start();
	BOOL					Send(CONNID dwConnID, void* pData, int iLength);
private:
	virtual EnHandleResult	OnPrepareListen(SOCKET soListen);
	virtual EnHandleResult	OnAccept(CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult	OnSend(CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult	OnReceive(CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult	OnClose(CONNID dwConnID);
	virtual EnHandleResult	OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult	OnShutdown();
	BOOL					GetMsgHeader(MSG_S2C_HEADER* pMsgHeader, const BYTE* byData, int nLength);
	CTcpServer m_Server;

};

