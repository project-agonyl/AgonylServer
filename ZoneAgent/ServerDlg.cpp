
// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include "UIUpdateThread.h"


// CServerDlg dialog


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ZONE_SVRS, m_ZoneServers);
	DDX_Control(pDX, IDC_LOGS, m_Logs);
	DDX_Control(pDX, IDC_CLOSE, m_Close);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CLOSE, &CServerDlg::OnBnClickedClose)
	ON_MESSAGE(USER_INFO_MSG, OnUserInfoMsg)
	ON_WM_VKEYTOITEM()
END_MESSAGE_MAP()


// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	::SetMainWnd(this);
	::SetInfoList(&m_Logs);
	g_lblMaxConnCount = GetDlgItem(IDC_MAX_CONN_COUNT);
	g_lblConnCount = GetDlgItem(IDC_CONN_COUNT);
	g_lblPrepConnCount = GetDlgItem(IDC_PREP_CONN_COUNT);
	g_lblZAIP = GetDlgItem(IDC_ZA_IP);
	g_lblZAPort = GetDlgItem(IDC_ZA_PORT);
	g_lblZAServerId = GetDlgItem(IDC_ZA_SVR_ID);
	g_lblZAAgentId = GetDlgItem(IDC_ZA_AGENT_ID);
	g_lblZoneServerCount = GetDlgItem(IDC_CONN_ZONE_SVR);
	g_lblZAStatus = GetDlgItem(IDC_ZA_STATUS);
	g_lblLSStatus = GetDlgItem(IDC_LS_STATUS);
	
	SetAppState(ST_STOPPED);
	
	g_lblZAIP->SetWindowTextW(CA2W(g_ZoneAgentIp));
	g_lblZAPort->SetWindowTextW(CA2W(std::to_string(g_ZoneAgentPort).c_str()));
	g_lblZAAgentId->SetWindowTextW(CA2W(std::to_string(g_AgentId).c_str()));
	g_lblZAServerId->SetWindowTextW(CA2W(std::to_string(g_ServerId).c_str()));
	g_lblZoneServerCount->SetWindowTextW(CA2W(std::to_string(g_ZoneServersCount).c_str()));
	g_lblZAStatus->SetWindowTextW(CA2W("Disconnected"));
	g_lblLSStatus->SetWindowTextW(CA2W("Disconnected"));
	
	//ReloadServerList();
	
	if (g_ListenServer.Start())
	{
		g_lblZAStatus->SetWindowTextW(CA2W("Connected"));
		g_LoginServerManager.Start();
		
		std::map<BYTE, CZoneServer*>::iterator it;
		for (it = g_ZoneServers.begin(); it != g_ZoneServers.end(); it++)
		{
			it->second->Start();
		}
		//ReloadServerList();
		g_uiThread.Start();
	}
	else
	{
		::LogMsg((CString)"ZoneAgent Start Fail !!!");
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (
			pMsg->message == WM_KEYDOWN		
			&&(	pMsg->wParam == VK_ESCAPE	 
			||	pMsg->wParam == VK_CANCEL	
			||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}

void CServerDlg::SetAppState(EnAppState state)
{
	m_enState = state;

	if(this->GetSafeHwnd() == nullptr)
		return;
}

/*void CServerDlg::OnBnClickedStart()
{
	CString strBindAddress;
	m_BindAddress.GetWindowText(strBindAddress);
	strBindAddress.Trim();

	if(strBindAddress.IsEmpty())
		strBindAddress = DEF_ADDRESS;

	m_RejectAddress.GetWindowText(m_strRejectAddress);
	m_strRejectAddress.Trim();

	SetAppState(ST_STARTING);

	if(m_Server.Start(strBindAddress, PORT))
	{
		::LogServerStart(strBindAddress, PORT);
		SetAppState(ST_STARTED);
	}
	else
	{
		::LogServerStartFail(m_Server.GetLastError(), m_Server.GetLastErrorDesc());
		SetAppState(ST_STOPPED);
	}
}*/

void CServerDlg::OnBnClickedClose()
{
	SetAppState(ST_STOPPING);
	
	/*if(m_Server.HasStarted() && m_Server.Stop())
	{
		::LogServerStop();
		SetAppState(ST_STOPPED);
	}*/
	
	//Check for memory leaks
	exit(0);
	
}

/*void CServerDlg::OnBnClickedDisconnect()
{
	CString strConnID;
	m_ConnID.GetWindowText(strConnID);
	CONNID dwConnID = (CONNID)_ttoi(strConnID);

	if(m_Server.Disconnect(dwConnID))
		::LogDisconnect(dwConnID);
	else
		::LogDisconnectFail(dwConnID);
}*/

/*void CServerDlg::OnEnChangeConnId()
{
	m_DisConn.EnableWindow(m_enState == ST_STARTED && m_ConnID.GetWindowTextLength() > 0);
}*/

int CServerDlg::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex)
{
	if(nKey == 'C')
		pListBox->ResetContent();

	return __super::OnVKeyToItem(nKey, pListBox, nIndex);
}

LRESULT CServerDlg::OnUserInfoMsg(WPARAM wp, LPARAM lp)
{
	info_msg* msg = (info_msg*)wp;

	::LogInfoMsg(msg);

	return 0;
}

/*EnHandleResult CServerDlg::OnPrepareListen(SOCKET soListen)
{
	TCHAR szAddress[40];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	m_Server.GetListenAddress(szAddress, iAddressLen, usPort);
	::PostOnPrepareListen(szAddress, usPort);
	return HR_OK;
}

EnHandleResult CServerDlg::OnAccept(CONNID dwConnID, SOCKET soClient)
{
	BOOL bPass = TRUE;
	TCHAR szAddress[40];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	m_Server.GetRemoteAddress(dwConnID, szAddress, iAddressLen, usPort);

	if (!m_strRejectAddress.IsEmpty())
	{
		if (m_strRejectAddress.CompareNoCase(szAddress) == 0)
			bPass = FALSE;
	}

	::PostOnAccept(dwConnID, szAddress, usPort, bPass);

	return bPass ? HR_OK : HR_ERROR;
}

EnHandleResult CServerDlg::OnSend(CONNID dwConnID, const BYTE* pData, int iLength)
{
	//static int t = 0;
	//if(++t % 3 == 0) return HR_ERROR;

	::PostOnSend(dwConnID, pData, iLength);
	return HR_OK;
}

EnHandleResult CServerDlg::OnReceive(CONNID dwConnID, const BYTE* pData, int iLength)
{
	//static int t = 0;
	//if(++t % 3 == 0) return HR_ERROR;

	::PostOnReceive(dwConnID, pData, iLength);

	if (m_Server.Send(dwConnID, pData, iLength))
		return HR_OK;
	else
		return HR_ERROR;
}

EnHandleResult CServerDlg::OnClose(CONNID dwConnID)
{
	::PostOnClose(dwConnID);
	return HR_OK;
}

EnHandleResult CServerDlg::OnError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	::PostOnError(dwConnID, enOperation, iErrorCode);
	return HR_OK;
}

EnHandleResult CServerDlg::OnShutdown()
{
	::PostOnShutdown();
	return HR_OK;
}*/
