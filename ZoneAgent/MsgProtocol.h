

/*-----------------------------------------------------------------------------

	0x0000 ~ 0x0fff	: reserved for system (error, system management, alert)
	0x1100 ~ 0x11ff	: login, logout, appear, disappear
	0x1200 ~ 0x12ff : move related (move, warp)
	0x1300 ~ 0x13ff : npc related (regen, move, attack)
	0x1400 ~ 0x14ff : attack related (attack, skill, die, alive)
	0x1500 ~ 0x15ff : magic related
	0x1600 ~ 0x16ff : ability related (level up, get skill
	0x1700 ~ 0x17ff : item related
	0x1800 ~ 0x1fff : reserved for basic function

	0x2100 ~ 0x21ff : quest
	0x2200 ~ 0x22ff : party
	0x2300 ~ 0x23ff : guild
	0x2400 ~ 0x24ff : nation
	0x2500 ~ 0x2fff : reserved for event

	0xa000 ~ 0xffff : main server <-> zone/account server

-----------------------------------------------------------------------------*/


//-----------------------------------------------------------------------------
//	Client <-> Zone / Account Server
//-----------------------------------------------------------------------------
#define CL2ZA_KH_CREST_1	0x2322
#define CL2ZA_KH_CREST_2	0x2323


#define	S2C_ERROR			0x0fff

#define	S2C_PC_APPEAR		0x1100
#define	S2C_PC_DISAPPEAR	0x1101
#define	S2C_CHAR_LIST		0x1105
#define	C2S_CHAR_LOGIN		0x1106
#define	S2C_CHAR_LOGIN_OK	0x1106
#define	C2S_WORLD_LOGIN		0x1107
#define	S2C_WORLD_LOGIN		0x1107
#define	C2S_CHAR_LOGOUT		0x1108

#define C2S_ASK_MOVE		0x1200
#define	S2C_ANS_MOVE		0x1200
#define	S2C_SEE_MOVE		0x1201
#define	C2S_PC_MOVE			0x1202
#define	S2C_FIX_MOVE		0x1203
#define S2C_SEE_STOP		0x1204

#define	C2S_ASK_CREATE_PLAYER	0xa001
#define	S2C_ANS_CREATE_PLAYER	0xa001
#define	C2S_ASK_DELETE_PLAYER	0xa002
#define	S2C_ANS_DELETE_PLAYER	0xa002

#define	S2M_ASK_CREATE_PLAYER	0xa00e
#define	M2S_ANS_CREATE_PLAYER	0xa00e
#define	S2M_ASK_DELETE_PLAYER	0xa00f
#define	M2S_ANS_DELETE_PLAYER	0xa00f

//-----------------------------------------------------------------------------
//	Zone Server <-> Main Server
//-----------------------------------------------------------------------------

#define	M2S_ERROR			0xa000
#define	S2M_SVR_LOGIN		0xa001

#define	S2M_CHAR_LOGIN		0xa010
#define	M2S_CHAR_LOGIN		0xa010
#define	S2M_WORLD_LOGIN		0xa011
#define	M2S_WORLD_LOGIN		0xa011
#define	S2M_CHAR_LOGOUT		0xa012
#define	M2S_CHAR_UPDATE		0xa013



//-----------------------------------------------------------------------------
//	Gate Server <-> Zone Server
//-----------------------------------------------------------------------------

#define	G2S_SVR_LOGIN		0xE0
#define	G2S_NEW_CLIENT		0xE1
#define G2S_DISCONNECT		0xE2
#define	S2G_DISCONNECT		0xE0
#define	S2G_SWITCH_ZONE		0xE1
#define	S2G_BROADCAST		0xE2

