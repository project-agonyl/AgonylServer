
// Server.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"

char							g_ZoneAgentIp[16];
int								g_ZoneAgentPort;
int								g_AgentId;
int								g_ServerId;
char							g_LoginServerIp[16];
int								g_LoginServerPort;
int								g_ZoneServersCount;
std::map<BYTE, CZoneServer*>	g_ZoneServers;
std::list<CPreparedPlayer>		g_PreparedPlayers;
std::map<DWORD, CPlayer*>		g_Players;

CListenServer		g_ListenServer;
CLoginServerManager g_LoginServerManager;
CUIUpdateThread		g_uiThread;

CWnd*			g_lblMaxConnCount;
CWnd*			g_lblConnCount;
CWnd*			g_lblPrepConnCount;
CWnd*			g_lblZAIP;
CWnd*			g_lblZAPort;
CWnd*			g_lblZAServerId;
CWnd*			g_lblZAAgentId;
CWnd*			g_lblZoneServerCount;
CWnd*			g_lblZAStatus;
CWnd*			g_lblLSStatus;
CListBox		m_ZoneServers;
CMsgCrypt		MsgCrypt;
// CServerApp

BEGIN_MESSAGE_MAP(CServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CServerApp construction

CServerApp::CServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CServerApp object

CServerApp theApp;


// CServerApp initialization

BOOL CServerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager* pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CServerDlg dlg;
	m_pMainWnd = &dlg;
	Load();
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CServerApp::Load()
{
	GetPrivateProfileStringA("ZONEAGENT", "IP", "127.0.0.2", g_ZoneAgentIp, 16, "./SvrInfo.ini");
	g_ZoneAgentPort = GetPrivateProfileIntA("ZONEAGENT", "PORT", 9982, "./SvrInfo.ini");
	g_AgentId = GetPrivateProfileIntA("ZONEAGENT", "AGENTID", 2, "./SvrInfo.ini");
	g_ServerId = GetPrivateProfileIntA("ZONEAGENT", "SERVERID", 2, "./SvrInfo.ini");
	GetPrivateProfileStringA("LOGINSERVER", "IP", "127.0.0.2", g_LoginServerIp, 16, "./SvrInfo.ini");
	g_LoginServerPort = GetPrivateProfileIntA("LOGINSERVER", "PORT", 3202, "./SvrInfo.ini");
	g_ZoneServersCount = GetPrivateProfileIntA("SERVERS", "COUNT", 2, "./SvrInfo.ini");
	LoadZoneServers();

	return TRUE;
}

BOOL CServerApp::LoadZoneServers()
{
	for (int i = 0; i < g_ZoneServersCount; i++)
	{
		CZoneServer* m_Server = new CZoneServer();
		std::string szId = "ID" + std::to_string(i);
		m_Server->nThisServerId = GetPrivateProfileIntA("SERVERS", szId.c_str(), 500, "./SvrInfo.ini");
		std::string szIp = "IP" + std::to_string(i);
		GetPrivateProfileStringA("SERVERS", szIp.c_str(), "127.0.0.1", m_Server->szThisServerIP, 16, "./SvrInfo.ini");
		std::string szPort = "PORT" + std::to_string(i);
		m_Server->usThisServerPort = GetPrivateProfileIntA("SERVERS", szPort.c_str(), 500, "./SvrInfo.ini");
		g_ZoneServers[m_Server->nThisServerId] = m_Server;
	}

	return TRUE;
}