#include "stdafx.h"
#include "ListenServer.h"

CListenServer::CListenServer() :m_Server(this)
{

}

BOOL CListenServer::Start()
{
	if (m_Server.Start(CA2W(g_ZoneAgentIp), g_ZoneAgentPort))
	{
		return TRUE;
	}
	else
	{
		::LogServerStartFail(m_Server.GetLastError(), m_Server.GetLastErrorDesc());
		return FALSE;
	}
}

BOOL CListenServer::Send(CONNID dwConnID, void* pData, int iLength)
{
	return m_Server.Send(dwConnID, (BYTE*)pData, iLength);
}


EnHandleResult CListenServer::OnPrepareListen(SOCKET soListen)
{
	/*TCHAR szAddress[40];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	m_Server.GetListenAddress(szAddress, iAddressLen, usPort);
	::PostOnPrepareListen(szAddress, usPort);
	*/return HR_OK;
}

EnHandleResult CListenServer::OnAccept(CONNID dwConnID, SOCKET soClient)
{
	/*BOOL bPass = TRUE;
	TCHAR szAddress[40];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	m_Server.GetRemoteAddress(dwConnID, szAddress, iAddressLen, usPort);

	::PostOnAccept(dwConnID, szAddress, usPort, bPass);
	*/
	return HR_OK;
}

EnHandleResult CListenServer::OnSend(CONNID dwConnID, const BYTE* pData, int iLength)
{
	//static int t = 0;
	//if(++t % 3 == 0) return HR_ERROR;

	//::PostOnSend(dwConnID, pData, iLength);
	return HR_OK;
}

EnHandleResult CListenServer::OnReceive(CONNID dwConnID, const BYTE* pData, int iLength)
{
	//static int t = 0;
	//if(++t % 3 == 0) return HR_ERROR;
	MSG_S2C_HEADER	tmpMsgHeader;
	if (GetMsgHeader(&tmpMsgHeader, pData, iLength) == FALSE) return HR_OK;
	switch (Func_IDENTIFY_PACKET(tmpMsgHeader))
	{
	case MSG_CL2ZA_USER_LOGIN_REQUEST:
		Sleep(300);
		Func_MSG_CL2ZA_USER_LOGIN((MSG_CL2ZA_USER_LOGIN*)pData, dwConnID);
		break;
		//case MSG_CL2ZA_USER_PING_RESPONSE:
		//	break;
		//case MSG_CL2ZA_KH_CREST_REQUEST:
		//	break;
		//case MSG_CL2ZA_CHECK_PAYMENT_INFO:
		//	break;
	case MSG_ZA2S_REQUEST:
	{
		CPlayer* player = Func_GET_PLAYER(dwConnID);
		char* buffer = (char*)pData;
		char byte_array[4] = { 0 };
		*(DWORD*)byte_array = player->dwPCID;
		strncpy(buffer + 4, byte_array, 4);
		g_ZoneServers[player->byPlayerZone]->Send(buffer, tmpMsgHeader.dwSize);
	}
	break;
	default:
		printf("Invalid message type when parsing CL to ZA message");
		break;
	}

	return HR_OK;
	//if (m_Server.Send(dwConnID, pData, iLength))
	//	return HR_OK;
	//else
	//	return HR_ERROR;
}

EnHandleResult CListenServer::OnClose(CONNID dwConnID)
{
	g_lblZAStatus->SetWindowTextW(CA2W("Disconnected"));
	return HR_OK;
}

EnHandleResult CListenServer::OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	if ((enOperation == SO_CONNECT || enOperation == SO_UNKNOWN) && (iErrorCode == 10061 || iErrorCode == 10053))
	{
		g_lblZAStatus->SetWindowTextW(CA2W("Disconnected"));
	}
	return HR_OK;
}

EnHandleResult CListenServer::OnShutdown()
{
	return HR_OK;
}

BOOL	CListenServer::GetMsgHeader(MSG_S2C_HEADER* pMsgHeader, const BYTE* byData, int nLength)
{
	if (nLength < sizeof(MSG_S2C_HEADER)) return FALSE;

	memcpy(pMsgHeader, byData, sizeof(MSG_S2C_HEADER));
	return TRUE;
}