#pragma once
class CPlayer
{
public:
	CPlayer(CONNID dwConnID);
public:
	char        szUsername[21];
	char        szPassword[21];
	DWORD       dwPCID;
	BYTE        byPlayerZone = 255;
	char        szCharName[21];
	int         nPing = 0;
	int         nPingPacketCounter = 0;
	char        szPlayerIP[16];
	CONNID		dwConnID;
	char		sendBuffer[4194304];
	int			sendBufferSize = 0;
};

