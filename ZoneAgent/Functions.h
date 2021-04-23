#pragma once

//General Helper Functions
extern CPlayer* Func_GET_PLAYER(CONNID dwConnID);

// LoginServer to ZoneAgent msg proc func
extern void				Func_LS2ZA_USER_INFO(MSG_LS2ZA_USER_INFO* pMsg);
extern void				Func_LS2ZA_USER_DROP(MSG_LS2ZA_USER_DROP* pMsg);

// Zoneagent to LoginServer msg proc func
extern void				Func_ZA2LS_USER_CONNECT(list<CPreparedPlayer>::iterator it);

// Client to ZoneAgent msg proc func
extern	MessageType		Func_IDENTIFY_PACKET(MSG_S2C_HEADER header);
extern	void			Func_MSG_CL2ZA_USER_LOGIN(MSG_CL2ZA_USER_LOGIN* pMsg, CONNID dwConnID);

// Server to ZoneAgent msg proc func
extern	void			Func_S2ZA_MESSAGE_PROCESS(const BYTE* pMsg, int nLen);

extern	void			ReloadServerList();