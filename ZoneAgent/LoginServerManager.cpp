#include "stdafx.h"
#include "LoginServerManager.h"

CLoginServerManager::CLoginServerManager():m_Client(this)
{

}

BOOL CLoginServerManager::Start()
{
	//return m_Client.Start(CA2W(g_LoginServerIp), g_LoginServerPort, TRUE);
	return BeginThread();
}

void CLoginServerManager::LoopProc()
{
	if (!m_bRunServer && m_Client.Start(CA2W(g_LoginServerIp), g_LoginServerPort, TRUE) == FALSE)
		OnClose();
	else
	{
		m_bRunServer = TRUE;
		CLoopThread::LoopProc();
	}

}
void CLoginServerManager::Process()
{
	if (m_bRunServer == FALSE)
		OnClose();
}

void CLoginServerManager::OnClose()
{
	m_bRunServer = FALSE;
	m_Client.Stop();
	LoopProc();
}

BOOL CLoginServerManager::Send(void* buf, int len)
{
	return m_Client.Send((BYTE*)buf, len);
}


EnHandleResult CLoginServerManager::OnConnect(IClient* pClient)
{
	m_bRunServer = TRUE;
	g_lblLSStatus->SetWindowTextW(CA2W("Connected"));

	Msg_ZA2LS_Svr_Login.byGateAgntID = g_AgentId;
	Msg_ZA2LS_Svr_Login.byGateSvrID = g_ServerId;
	Msg_ZA2LS_Svr_Login.dwGateSvrPort = g_ZoneAgentPort;
	strncpy(Msg_ZA2LS_Svr_Login.szGateSvrIP, g_ZoneAgentIp, strlen(g_ZoneAgentIp));

	Send(&Msg_ZA2LS_Svr_Login, sizeof(Msg_ZA2LS_Svr_Login));

	return HR_OK;
}

EnHandleResult CLoginServerManager::OnSend(IClient* pClient, const BYTE* pData, int iLength)
{
	//::PostOnSend(pClient->GetConnectionID(), pData, iLength);
	return HR_OK;
}

EnHandleResult CLoginServerManager::OnReceive(IClient* pClient, const BYTE* pData, int iLength)
{
	MSG_DFLT_HEADER	tmpMsgHeader;
	if (GetMsgHeader(&tmpMsgHeader, pData, iLength) == FALSE) return HR_OK;

	switch (tmpMsgHeader.byCtrl)
	{
	case 0x01:
		switch (tmpMsgHeader.byCmd)
		{
		case 0xE1:// MSG_LS2ZA_USER_INFO
			Func_LS2ZA_USER_INFO((MSG_LS2ZA_USER_INFO*)pData);
			break;
		case 0xE3:// MSG_LS2ZA_USER_DROP
			Func_LS2ZA_USER_DROP((MSG_LS2ZA_USER_DROP*)pData);
			break;
		default:
			printf("Invalid cmd when parsing LS to ZA message");
			break;
		}
		break;
	default:
		printf("Invalid ctrl when parsing LS to ZA message");
		break;
	}
	//::PostOnReceive(pClient->GetConnectionID(), pData, iLength);
	return HR_OK;
}

EnHandleResult CLoginServerManager::OnClose(IClient* pClient)
{
	m_bRunServer = FALSE;
	g_lblLSStatus->SetWindowTextW(CA2W("Disconnected"));
	return HR_OK;
}

EnHandleResult CLoginServerManager::OnError(IClient* pClient, EnSocketOperation enOperation, int iErrorCode)
{
	if ((enOperation == SO_CONNECT || enOperation == SO_UNKNOWN) && (iErrorCode == 10061 || iErrorCode == 10053))
	{
		m_bRunServer = FALSE;
		g_lblLSStatus->SetWindowTextW(CA2W("Disconnected"));
	}
	//::PostOnError(pClient->GetConnectionID(), enOperation, iErrorCode);
	return HR_OK;
}

BOOL	CLoginServerManager::GetMsgHeader(MSG_DFLT_HEADER* pMsgHeader, const BYTE* pData, int nLength)
{
	if (nLength < sizeof(MSG_DFLT_HEADER)) return FALSE;

	memcpy(pMsgHeader, pData, sizeof(MSG_DFLT_HEADER));
	return TRUE;
}