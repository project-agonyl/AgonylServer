#include "MsgProtocol.h"

#define		SIZEOFSIZE	0

#pragma pack(1)


//----------------------------------------------------------------------------------------------
// Message Header
//----------------------------------------------------------------------------------------------
class MSG_DFLT_HEADER	// Default header to Zone Server... this packet can be ether C2S or G2S
{
public:
	DWORD	dwSize;
	DWORD	dwPCID;
	BYTE	byCtrl;
	BYTE	byCmd;
};

class MSG_S2G_HEADER	// Zone Server <-> Gate Server
{
public:
	MSG_S2G_HEADER() { byCtrl = 0x01; byCmd = 0xFF; }

	DWORD	dwSize;
	DWORD	dwPCID;
	BYTE	byCtrl;
	BYTE	byCmd;
};

class MSG_S2C_HEADER	//Zone Server <-> Client
{
public:
	MSG_S2C_HEADER() { byCtrl = 0x03; byCmd = 0xFF; }

	DWORD	dwSize;
	DWORD	dwPCID;
	BYTE	byCtrl;
	BYTE	byCmd;
	WORD	wProtocol;
};


//----------------------------------------------------------------------------------------------
// ZoneAgent <-> LoginServer Message 
//----------------------------------------------------------------------------------------------

class MSG_ZA2LS_SVR_LOGIN
{
public:
	MSG_ZA2LS_SVR_LOGIN()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2LS_SVR_LOGIN) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x02;
		MsgHeader.byCmd = 0xE0;
	}
	MSG_DFLT_HEADER	MsgHeader;
	BYTE			byGateSvrID;
	BYTE			byGateAgntID;
	char			szGateSvrIP[16];
	DWORD			dwGateSvrPort;
};

class MSG_ZA2LS_USER_CONNECT
{
public:
	MSG_ZA2LS_USER_CONNECT()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2LS_USER_CONNECT) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x02;
		MsgHeader.byCmd = 0xE3;
	}
	MSG_DFLT_HEADER	MsgHeader;
	char			szUsername[21];
};

class MSG_ZA2LS_USER_DISCONNECT
{
public:
	MSG_ZA2LS_USER_DISCONNECT()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2LS_USER_DISCONNECT) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x02;
		MsgHeader.byCmd = 0xE2;
	}
	MSG_DFLT_HEADER	MsgHeader;
	BYTE			byExitStatus;
	char			szUsername[21];
	char			szTimeStamp[16];
};

class MSG_ZA2LS_USER_RECOVER
{
public:
	MSG_ZA2LS_USER_RECOVER()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2LS_USER_RECOVER) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x02;
		MsgHeader.byCmd = 0xE4;
	}
	MSG_DFLT_HEADER	MsgHeader;
	char			szUsername[21];
	char			szUserIP[16];
	DWORD			unknown1;
};

class MSG_ZA2LS_USER_COUNT_INFORM
{
public:
	MSG_ZA2LS_USER_COUNT_INFORM()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2LS_USER_COUNT_INFORM) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x02;
		MsgHeader.byCmd = 0xE1;
	}
	MSG_DFLT_HEADER	MsgHeader;
	DWORD			dwPlayerCount;
	BYTE			byZoneCount1;
	BYTE			byZoneCount2;
};



//----------------------------------------------------------------------------------------------
// LoginServer <-> ZoneAgent  Message 
//----------------------------------------------------------------------------------------------

class MSG_LS2ZA_USER_INFO
{
public:
	MSG_DFLT_HEADER	MsgHeader;
	char			szUsername[21];
	//char			szUserIP[16];
	char			unknown[9];
};

class MSG_LS2ZA_USER_DROP
{
public:
	MSG_DFLT_HEADER	MsgHeader;
	BYTE			unknown1;
	char			szUsername[21];
	char			unknown2[16];
};



//----------------------------------------------------------------------------------------------
// Client <-> ZoneAgent  Message 
//----------------------------------------------------------------------------------------------

class MSG_CL2ZA_USER_LOGIN
{
public:
	MSG_DFLT_HEADER	MsgHeader;
	DWORD			dwPCID;
	char			szUsername[21];
	char			szPassword[21];
};

//----------------------------------------------------------------------------------------------
// ZoneAgent <-> Servers  Message 
//----------------------------------------------------------------------------------------------

class MSG_ZA2S_CONNECT_REQUEST
{
public:
	MSG_ZA2S_CONNECT_REQUEST()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2S_CONNECT_REQUEST) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x01;
		MsgHeader.byCmd = 0xE0;
	}
	MSG_DFLT_HEADER MsgHeader;
	BYTE			byAgentId;
};

//----------------------------------------------------------------------------------------------
// Servers <-> ZoneAgent Message 
//----------------------------------------------------------------------------------------------
class MSG_S2ZA_SWITCH_ZONE
{
public:
	MSG_DFLT_HEADER MsgHeader;
	BYTE			byServerId;
};


//----------------------------------------------------------------------------------------------
// ZoneAgent <-> AccountServer  Message 
//----------------------------------------------------------------------------------------------
class MSG_ZA2AS_USER_LOGIN
{
public:
	MSG_ZA2AS_USER_LOGIN()
	{
		MsgHeader.dwSize = sizeof(MSG_ZA2AS_USER_LOGIN) - SIZEOFSIZE;
		MsgHeader.byCtrl = 0x01;
		MsgHeader.byCmd = 0xE1;
	}
	MSG_DFLT_HEADER	MsgHeader;
	char			szUsername[21];
	char			szPassword[21];
	char			szUserIP[16];
	char			szReserved[78];
};


#pragma pack()


//----------------------------------------------------------------------------------------------
// Message Structure Instances extern declaration
//----------------------------------------------------------------------------------------------

// ZoneAgent <-> LoginServer
extern MSG_ZA2LS_SVR_LOGIN			Msg_ZA2LS_Svr_Login;
extern MSG_ZA2LS_USER_CONNECT		Msg_ZA2LS_User_Connect;
extern MSG_ZA2LS_USER_DISCONNECT	Msg_ZA2LS_User_Disconnect;
extern MSG_ZA2LS_USER_RECOVER		Msg_ZA2LS_User_Recover;
extern MSG_ZA2LS_USER_COUNT_INFORM	Msg_ZA2LS_User_Count_Inform;

// LoginServer <-> ZoneAgent 
extern MSG_LS2ZA_USER_INFO		Msg_LS2ZA_User_Info;
extern MSG_LS2ZA_USER_DROP		Msg_LS2ZA_User_Drop;

// Client <-> ZoneAgent
extern MSG_CL2ZA_USER_LOGIN		Msg_CL2ZA_User_Login;

// ZoneAgent <-> Servers
extern MSG_ZA2S_CONNECT_REQUEST		Msg_ZA2S_Connect_Request;
extern MSG_ZA2AS_USER_LOGIN			Msg_ZA2AS_User_Login;

// Servers <-> ZoneAgent
extern MSG_S2ZA_SWITCH_ZONE			Msg_S2ZA_Switch_Zone;