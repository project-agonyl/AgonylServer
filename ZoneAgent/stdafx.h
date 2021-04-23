
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

//#define _WIN32_WINNT _WIN32_WINNT_WINXP
//#define _WIN32_WINNT _WIN32_WINNT_WIN7

#define _USE_MFC

#define _UDP_DISABLED
#define _SSL_DISABLED
#define _HTTP_DISABLED
#define _ZLIB_DISABLED
#define _BROTLI_DISABLED

#include "../HpSocket/Common/GeneralHelper.h"
#include "../HpSocket/Global/helper.h"
#include "MsgStructure.h"
#include "ListenServer.h"
#include "LoginServerManager.h"
#include "MsgType.h"
#include "PreparedPlayer.h"
#include "Player.h"
#include "Crypt.h"
#include "Functions.h"
#include "ZoneServer.h"
#include "UIUpdateThread.h"

extern char							g_ZoneAgentIp[16];
extern int							g_ZoneAgentPort;
extern int							g_AgentId;
extern int							g_ServerId;
extern char							g_LoginServerIp[16];
extern int							g_LoginServerPort;
extern int							g_ZoneServersCount;
extern std::map<BYTE, CZoneServer*>	g_ZoneServers;
extern std::list<CPreparedPlayer>	g_PreparedPlayers;
extern std::map<DWORD, CPlayer*>	g_Players;

extern CWnd* g_lblMaxConnCount;
extern CWnd* g_lblConnCount;
extern CWnd* g_lblPrepConnCount;
extern CWnd* g_lblZAIP;
extern CWnd* g_lblZAPort;
extern CWnd* g_lblZAServerId;
extern CWnd* g_lblZAAgentId;
extern CWnd* g_lblZoneServerCount;
extern CWnd* g_lblZAStatus;
extern CWnd* g_lblLSStatus;
extern CListBox m_ZoneServers;

extern CListenServer		g_ListenServer;
extern CLoginServerManager	g_LoginServerManager;
extern CUIUpdateThread		g_uiThread;
extern CMsgCrypt			MsgCrypt;