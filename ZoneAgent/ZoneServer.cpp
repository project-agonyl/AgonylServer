#include "stdafx.h"
#include "ZoneServer.h"

CZoneServer::CZoneServer():m_Client(this)
{
}

BOOL CZoneServer::Start()
{
	return BeginThread();
}

void CZoneServer::LoopProc()
{
	if (!m_bRunServer && m_Client.Start(CA2W(szThisServerIP), usThisServerPort, TRUE) == FALSE)
		OnClose();
	else
	{
		m_bRunServer = TRUE;
		CLoopThread::LoopProc();
	}

}
void CZoneServer::Process()
{
	if (m_bRunServer == FALSE)
		OnClose();
}

void CZoneServer::OnClose()
{
	m_bRunServer = FALSE;
	m_Client.Stop();
	LoopProc();
}

BOOL CZoneServer::Send(void* buf, int len)
{
	return m_Client.Send((BYTE*)buf, len);
}


EnHandleResult CZoneServer::OnConnect(IClient* pClient)
{
	m_bRunServer = TRUE;
	m_bisConnected = TRUE;
	Msg_ZA2S_Connect_Request.byAgentId = g_AgentId;
	Send(&Msg_ZA2S_Connect_Request, sizeof(Msg_ZA2S_Connect_Request));

	return HR_OK;
}

EnHandleResult CZoneServer::OnSend(IClient* pClient, const BYTE* pData, int iLength)
{
	//::PostOnSend(pClient->GetConnectionID(), pData, iLength);
	return HR_OK;
}

EnHandleResult CZoneServer::OnReceive(IClient* pClient, const BYTE* pData, int iLength)
{
	Func_S2ZA_MESSAGE_PROCESS(pData, iLength);
	//::PostOnReceive(pClient->GetConnectionID(), pData, iLength);
	return HR_OK;
}

EnHandleResult CZoneServer::OnClose(IClient* pClient)
{
	m_bRunServer = FALSE;
	m_bisConnected = FALSE;
	
	return HR_OK;
}

EnHandleResult CZoneServer::OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
{
	if ((enOperation == SO_CONNECT || enOperation == SO_UNKNOWN) && (iErrorCode == 10061 || iErrorCode == 10053))
	{
		m_bisConnected = FALSE;
		m_bRunServer = FALSE;
		
	}
	//::PostOnError(pClient->GetConnectionID(), enOperation, iErrorCode);
	return HR_OK;
}

BOOL	CZoneServer::GetMsgHeader(MSG_DFLT_HEADER* pMsgHeader, const BYTE* pData, int nLength)
{
	if (nLength < sizeof(MSG_DFLT_HEADER)) return FALSE;

	memcpy(pMsgHeader, pData, sizeof(MSG_DFLT_HEADER));
	return TRUE;
}