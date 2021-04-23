#include "stdafx.h"


//-----------------------------------------------------------------------------
// Message Structure Instances declaration
//-----------------------------------------------------------------------------

// ZoneAgent <-> LoginServer
MSG_ZA2LS_SVR_LOGIN			Msg_ZA2LS_Svr_Login;
MSG_ZA2LS_USER_CONNECT		Msg_ZA2LS_User_Connect;
MSG_ZA2LS_USER_DISCONNECT	Msg_ZA2LS_User_Disconnect;
MSG_ZA2LS_USER_RECOVER		Msg_ZA2LS_User_Recover;
MSG_ZA2LS_USER_COUNT_INFORM	Msg_ZA2LS_User_Count_Inform;

// LoginServer <-> ZoneAgent 
MSG_LS2ZA_USER_INFO		Msg_LS2ZA_User_Info;
MSG_LS2ZA_USER_DROP		Msg_LS2ZA_User_Drop;

// Client to ZoneAgent
MSG_CL2ZA_USER_LOGIN		Msg_CL2ZA_User_Login;

// ZoneAgent to Servers
MSG_ZA2S_CONNECT_REQUEST		Msg_ZA2S_Connect_Request;
MSG_ZA2AS_USER_LOGIN			Msg_ZA2AS_User_Login;

// Servers to ZoneAgent
MSG_S2ZA_SWITCH_ZONE			Msg_S2ZA_Switch_Zone;



struct CPreparedPlayerContainsUsername
{
	CPreparedPlayerContainsUsername(char* const& n) : name(n) { }
	bool operator () (CPreparedPlayer const& p) {
		int cmp = strcmp(p.szUsername, name);
		if (cmp == 0)
			return TRUE;
		else
			return FALSE;
	}
private:
	char* name;
};

struct CPreparedPlayerContainsPlayer
{
	CPreparedPlayerContainsPlayer(char* const& n, DWORD dwPCID) : name(n) { this->dwPCID = dwPCID; }
	bool operator () (CPreparedPlayer const& p) {
		int cmp = strcmp(p.szUsername, name);
		if (cmp == 0 && p.dwPCID == dwPCID)
			return TRUE;
		else
			return FALSE;
	}
private:
	char* name;
	DWORD	dwPCID;
};

struct CPlayerContainsCONNID
{
	CPlayerContainsCONNID(CONNID dwConnID) : dwConnID(dwConnID) { }//this->dwPCID = dwPCID; }
	bool operator () (pair<DWORD, CPlayer*> const& p) {
		if (p.second->dwConnID == dwConnID)
			return TRUE;
		else
			return FALSE;
	}
private:
	CONNID	dwConnID;
};


void ReloadServerList()
{
	m_ZoneServers.ResetContent();
	map<BYTE, CZoneServer*>::iterator it;
	for (it = g_ZoneServers.begin(); it != g_ZoneServers.end(); it++)
	{
		char buffer[100];
		sprintf(buffer, "%s:%d:%d  %s", it->second->szThisServerIP, it->second->usThisServerPort, it->first, (it->second->m_bisConnected ? "Connected" : "Disconnected"));
		m_ZoneServers.AddString(CA2W(buffer));
	}
}


BOOL FindOneOf(LPTSTR psz, LPCTSTR pszBanChars)
{
	int len1 = strlen(CW2A(psz));
	int len2 = strlen(CW2A(pszBanChars));

	for (int i = 0; i < len1; i++)
	{
		for (int j = 0; j < len2; j++)
		{
			if (psz[i] == pszBanChars[j])
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


void StrTrim(char* psz, const char* pszTrimChars)
{
	int len1 = strlen(psz);
	int len2 = strlen(pszTrimChars);

	int n = 0;
	char* buf = new char[len1 + 1];	// Incluede Null Termination 

	int j = 0;
	for (int i = 0; i < len1; i++)
	{
		for (j = 0; j < len2; j++)
		{
			if (psz[i] == pszTrimChars[j]) break;
		}

		if (j == len2)
			buf[n++] = psz[i];
	}

	buf[n] = NULL;
	strcpy(psz, buf);

	delete[] buf;
}

CPlayer* Func_GET_PLAYER(CONNID dwConnID)
{
	map<DWORD, CPlayer*>::iterator it;
	it = std::find_if(g_Players.begin(), g_Players.end(), CPlayerContainsCONNID(dwConnID));
	if (it != g_Players.end())
		return it->second;
	else
		return NULL;
}





//----------------------------------------------------------------------------------------------
// LoginServer to ZoneAgent Packet Processing Functions
//----------------------------------------------------------------------------------------------

void	Func_LS2ZA_USER_INFO(MSG_LS2ZA_USER_INFO* pMsg)
{
	list<CPreparedPlayer>::iterator it;
	it = std::find_if(g_PreparedPlayers.begin(), g_PreparedPlayers.end(), CPreparedPlayerContainsUsername(pMsg->szUsername));
	if (it != g_PreparedPlayers.end())
	{
		char buffer[200];
		sprintf(buffer, "<LC> UID = %d %s Prepared\n", pMsg->MsgHeader.dwPCID, pMsg->szUsername);
		::LogMsg(CString(buffer));
	}
	else
	{
		CPreparedPlayer newPlayer;
		newPlayer.dwPCID = pMsg->MsgHeader.dwPCID;
		strcpy(newPlayer.szUsername, pMsg->szUsername);
		//strcpy(newPlayer.szPlayerIP, pMsg->szUserIP);
		g_PreparedPlayers.push_back(newPlayer);
		g_Players[newPlayer.dwPCID] = NULL;
		char buffer[200];
		sprintf(buffer, "<LC> UID = %d %s (%s) Prepared\n", pMsg->MsgHeader.dwPCID, pMsg->szUsername, "127.0.0.1");// pMsg->szUserIP);
		::LogMsg(CString(buffer));
	}
}

void	Func_LS2ZA_USER_DROP(MSG_LS2ZA_USER_DROP* pMsg)
{

}


//----------------------------------------------------------------------------------------------
// LoginServer to ZoneAgent Packet Processing Functions
//----------------------------------------------------------------------------------------------
void	Func_ZA2LS_USER_CONNECT(list<CPreparedPlayer>::iterator it)
{
	Msg_ZA2LS_User_Connect.MsgHeader.dwPCID = it->dwPCID;
	strcpy(Msg_ZA2LS_User_Connect.szUsername, it->szUsername);
	g_LoginServerManager.Send(&Msg_ZA2LS_User_Connect, sizeof(Msg_ZA2LS_User_Connect));
}


//----------------------------------------------------------------------------------------------
// Client to ZoneAgent Packet Processing Functions
//----------------------------------------------------------------------------------------------

MessageType	Func_IDENTIFY_PACKET(MSG_S2C_HEADER header)
{
	MessageType messageType = MSG_ZA2S_REQUEST;
	if (header.byCtrl == 0x01 && header.byCmd == (byte)MSG_CL2ZA_USER_LOGIN_REQUEST)
		messageType = MSG_CL2ZA_USER_LOGIN_REQUEST;
	else if (header.byCtrl == 0x01 && header.byCmd == (byte)MSG_CL2ZA_USER_PING_RESPONSE)
		messageType = MSG_CL2ZA_USER_PING_RESPONSE;
	else if (header.byCtrl == 0xFE && header.byCmd == 0xFF && (header.wProtocol == CL2ZA_KH_CREST_1 || header.wProtocol == CL2ZA_KH_CREST_1))
		messageType = MSG_CL2ZA_KH_CREST_REQUEST;
	else if (header.byCtrl == 0x03 && header.byCmd == 0xFF && header.wProtocol == 0xC000)
		messageType = MSG_CL2ZA_CHECK_PAYMENT_INFO;

	return messageType;
}

void	Func_MSG_CL2ZA_USER_LOGIN(MSG_CL2ZA_USER_LOGIN* pMsg, CONNID dwConnID)
{
	list<CPreparedPlayer>::iterator it;
	it = std::find_if(g_PreparedPlayers.begin(), g_PreparedPlayers.end(), CPreparedPlayerContainsPlayer(pMsg->szUsername, pMsg->dwPCID));
	CPlayer* player = new CPlayer(dwConnID);
	if (it != g_PreparedPlayers.end())
	{
		strcpy(player->szUsername, pMsg->szUsername);
		strcpy(player->szPassword, pMsg->szPassword);
		//strcpy(player->szPlayerIP, it->szPlayerIP);
		strcpy(player->szPlayerIP, "127.0.0.1");
		player->dwPCID = pMsg->dwPCID;
		g_Players[pMsg->dwPCID] = player;
		Func_ZA2LS_USER_CONNECT(it);
		//g_PreparedPlayers.remove_if(CPreparedPlayerContainsPlayer(pMsg->szUsername, pMsg->dwPCID));


		Msg_ZA2AS_User_Login.MsgHeader.dwPCID = pMsg->dwPCID;
		strcpy(Msg_ZA2AS_User_Login.szUsername, pMsg->szUsername);
		strcpy(Msg_ZA2AS_User_Login.szPassword, pMsg->szPassword);
		//strcpy(Msg_ZA2AS_User_Login.szUserIP, it->szPlayerIP);
		strcpy(Msg_ZA2AS_User_Login.szUserIP, "127.0.0.1");
		g_ZoneServers[player->byPlayerZone]->Send(&Msg_ZA2AS_User_Login, sizeof(Msg_ZA2AS_User_Login));

		char buffer[200];
		sprintf(buffer, "%s (%s) User Joined\n", it->szUsername, "127.0.0.1");//it->szPlayerIP);
		::LogMsg(CString(buffer));
	}
	else
	{
		char buffer[200];
		sprintf(buffer, "Player Not Exists : Func_MSG_CL2ZA_USER_LOGIN pcID: %d\n", it->dwPCID);
		::LogMsg(CString(buffer));
	}
}


//----------------------------------------------------------------------------------------------
// ZoneAgent to Server Packet Processing Functions
//----------------------------------------------------------------------------------------------
void Func_S2ZA_MESSAGE_PROCESS(const BYTE* pMsg, int nLen)
{
	char* buffer = (char*)pMsg;
	int i = 0;
	map<CONNID, CPlayer*> players;
	while (i < nLen)
	{
		MSG_DFLT_HEADER* tmpDfltMsgHeader = new MSG_DFLT_HEADER();
		memcpy(tmpDfltMsgHeader, buffer + i, sizeof(MSG_DFLT_HEADER));
		CPlayer* player = g_Players[tmpDfltMsgHeader->dwPCID];

		if (player != NULL)
		{
			if (tmpDfltMsgHeader->dwSize > 12)
			{
				MSG_S2C_HEADER* tmpMsgHeader = new MSG_S2C_HEADER();
				memcpy(tmpMsgHeader, buffer + i, sizeof(MSG_S2C_HEADER));

				if (tmpMsgHeader->byCtrl == 0x03 && tmpMsgHeader->byCmd == 0xFF && tmpMsgHeader->wProtocol == 0x1105)
				{
					MsgCrypt.DecryptMsgBody(buffer, tmpMsgHeader->dwSize);
					for (int j = 32; j <= 784; j += 188)
					{
						buffer[i + j + 3] = buffer[i + j + 2];
						buffer[i + j + 2] = buffer[i + j + 1];
						buffer[i + j + 1] = 0x01;
						buffer[i + j] = 0x00;
					}
					MsgCrypt.EncryptMsgBody(buffer, tmpMsgHeader->dwSize);
				}
				/*else if (tmpMsgHeader->byCtrl == 0x03 && tmpMsgHeader->byCmd == 0xFF && tmpMsgHeader->wProtocol == 0x1108)
				{
					player->SendMsg(buffer + i, tmpMsgHeader->dwSize);
					player->m_bConnect = FALSE;
					player.ToBeDisposed = true;
				}
				else if (header.byCtrl == 0x03 && header.byCmd == 0xFF && header.wProtocol == 0x1800)
				{
					player.Send(packet);
					byte[] tempPacket = Crypt.Decrypt(packet);
					if (tempPacket[12] == 0x0C)
					{
						MSG_S2ZA_EVENT_NOTICE request = new MSG_S2ZA_EVENT_NOTICE();
						request.Deserialize(ref tempPacket);
						config.PrizeEventDetails = request.message;
					}
				}*/

				//g_ListenServer.Send(player->dwConnID, buffer + i, tmpMsgHeader->dwSize);
			}
			else
			{
				//g_ListenServer.Send(g_Players[tmpDfltMsgHeader->dwPCID]->dwConnID, buffer + i, tmpDfltMsgHeader->dwSize);
				if (tmpDfltMsgHeader->byCtrl == 0x01 && tmpDfltMsgHeader->byCmd == 0xE1)
				{
					MSG_S2ZA_SWITCH_ZONE* Msg = (MSG_S2ZA_SWITCH_ZONE*)buffer;
					g_Players[tmpDfltMsgHeader->dwPCID]->byPlayerZone = Msg->byServerId;
					char buffer[200];
					sprintf(buffer, "Player Id : %d Zone Changed : %d\n", tmpDfltMsgHeader->dwPCID, Msg->byServerId);
					::LogMsg(CString(buffer));
				}
			}
			memcpy(&(player->sendBuffer[player->sendBufferSize]), buffer + i, tmpDfltMsgHeader->dwSize);
			player->sendBufferSize += tmpDfltMsgHeader->dwSize;
			if (players[player->dwConnID] == NULL)
				players[player->dwConnID] = player;
		}
		else
		{
			char buffer[200];
			sprintf(buffer, "Player Not Exists : Func_S2ZA_MESSAGE_PROCESS pcID: %d\n", tmpDfltMsgHeader->dwPCID);
			::LogMsg(CString(buffer));
		}
		i += tmpDfltMsgHeader->dwSize;
	}
	map<CONNID, CPlayer*>::iterator it;
	for (it = players.begin(); it != players.end(); it++)
	{
		g_ListenServer.Send(it->first, it->second->sendBuffer, it->second->sendBufferSize);
		it->second->sendBufferSize = 0;
	}

}