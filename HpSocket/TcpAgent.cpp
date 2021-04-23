﻿/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "stdafx.h"
#include <atlfile.h>
#include "TcpAgent.h"
#include "Common/WaitFor.h"

#include <malloc.h>
#include <process.h>

EnHandleResult CTcpAgent::FireReceive(TSocketObj* pSocketObj, const BYTE* pData, int iLength)
{
	if(m_enRecvPolicy == RP_SERIAL)
	{
		if(TSocketObj::IsValid(pSocketObj))
		{
			CReentrantSpinLock locallock(pSocketObj->csRecv);

			if(TSocketObj::IsValid(pSocketObj))
			{
				return m_psoListener->OnReceive(pSocketObj->connID, pData, iLength);
			}
		}

		return (EnHandleResult)HR_CLOSED;
	}

	return m_psoListener->OnReceive(pSocketObj->connID, pData, iLength);
}

EnHandleResult CTcpAgent::FireReceive(TSocketObj* pSocketObj, int iLength)
{
	if(m_enRecvPolicy == RP_SERIAL)
	{
		if(TSocketObj::IsValid(pSocketObj))
		{
			CReentrantSpinLock locallock(pSocketObj->csRecv);

			if(TSocketObj::IsValid(pSocketObj))
			{
				return m_psoListener->OnReceive(pSocketObj->connID, iLength);
			}
		}

		return (EnHandleResult)HR_CLOSED;
	}

	return m_psoListener->OnReceive(pSocketObj->connID, iLength);
}

EnHandleResult CTcpAgent::FireClose(TSocketObj* pSocketObj)
{
	if(m_enRecvPolicy == RP_SERIAL)
	{
		CReentrantSpinLock locallock(pSocketObj->csRecv);
		return m_psoListener->OnClose(pSocketObj->connID);
	}

	return m_psoListener->OnClose(pSocketObj->connID);
}

EnHandleResult CTcpAgent::FireError(TSocketObj* pSocketObj, EnSocketOperation enOperation, int iErrorCode)
{
	if(m_enRecvPolicy == RP_SERIAL)
	{
		CReentrantSpinLock locallock(pSocketObj->csRecv);
		return m_psoListener->OnError(pSocketObj->connID, enOperation, iErrorCode);
	}

	return m_psoListener->OnError(pSocketObj->connID, enOperation, iErrorCode);
}

void CTcpAgent::SetLastError(EnSocketError code, LPCSTR func, int ec)
{
	m_enLastError = code;

	TRACE("%s --> Error: %d, EC: %d\n", func, code, ec);
}

BOOL CTcpAgent::Start(LPCTSTR pszBindAddress, BOOL bAsyncConnect)
{
	if(!CheckParams() || !CheckStarting())
		return FALSE;

	if(ParseBindAddress(pszBindAddress, bAsyncConnect))
		if(CreateCompletePort())
			if(CreateWorkerThreads())
			{
				m_bAsyncConnect	= bAsyncConnect;
				m_enState		= SS_STARTED;

				return TRUE;
			}

	Stop();

	return FALSE;
}

BOOL CTcpAgent::CheckParams()
{
	m_itPool.SetItemCapacity((int)m_dwSocketBufferSize);
	m_itPool.SetPoolSize((int)m_dwFreeBufferObjPool);
	m_itPool.SetPoolHold((int)m_dwFreeBufferObjHold);

	if(m_enSendPolicy >= SP_PACK && m_enSendPolicy <= SP_DIRECT)
		if(m_enRecvPolicy >= RP_SERIAL && m_enRecvPolicy <= RP_PARALLEL)
			if((int)m_dwWorkerThreadCount > 0 && m_dwWorkerThreadCount <= MAX_WORKER_THREAD_COUNT)
				if((int)m_dwSocketBufferSize >= MIN_SOCKET_BUFFER_SIZE)
					if((int)m_dwFreeSocketObjLockTime >= 0 && m_dwFreeSocketObjLockTime <= MAXLONG)
						if((int)m_dwFreeSocketObjPool >= 0)
							if((int)m_dwFreeBufferObjPool >= 0)
								if((int)m_dwFreeSocketObjHold >= m_dwFreeSocketObjPool)
									if((int)m_dwFreeBufferObjHold >= m_dwFreeBufferObjPool)
										if((int)m_dwKeepAliveTime >= 0)
											if((int)m_dwKeepAliveInterval >= 0)
												if((int)m_dwMaxShutdownWaitTime >= 0)
													return TRUE;

	SetLastError(SE_INVALID_PARAM, __FUNCTION__, ERROR_INVALID_PARAMETER);
	return FALSE;
}

BOOL CTcpAgent::CheckStarting()
{
	CSpinLock locallock(m_csState);

	if(m_enState == SS_STOPPED)
		m_enState = SS_STARTING;
	else
	{
		SetLastError(SE_ILLEGAL_STATE, __FUNCTION__, ERROR_INVALID_OPERATION);
		return FALSE;
	}

	return TRUE;
}

BOOL CTcpAgent::CheckStoping()
{
	CSpinLock locallock(m_csState);

	if(m_enState == SS_STARTED || m_enState == SS_STARTING)
		m_enState = SS_STOPPING;
	else
	{
		SetLastError(SE_ILLEGAL_STATE, __FUNCTION__, ERROR_INVALID_OPERATION);
		return FALSE;
	}

	return TRUE;
}

BOOL CTcpAgent::ParseBindAddress(LPCTSTR pszBindAddress, BOOL bAsyncConnect)
{
	BOOL isOK	= FALSE;
	SOCKET sock	= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(sock != INVALID_SOCKET)
	{
		if(!pszBindAddress)
			pszBindAddress = DEFAULT_BIND_ADDRESS;

		::sockaddr_A_2_IN(AF_INET, pszBindAddress, 0, m_soAddrIN);

		if(!bAsyncConnect || ::bind(sock, (SOCKADDR*)&m_soAddrIN, sizeof(SOCKADDR_IN)) != SOCKET_ERROR)
		{
			m_pfnConnectEx		= ::Get_ConnectEx_FuncPtr(sock);
			m_pfnDisconnectEx	= ::Get_DisconnectEx_FuncPtr(sock);

			ASSERT(m_pfnConnectEx);
			ASSERT(m_pfnDisconnectEx);

			isOK = TRUE;
		}
		else
			SetLastError(SE_SOCKET_BIND, __FUNCTION__, ::WSAGetLastError());

		::ManualCloseSocket(sock);
	}
	else
		SetLastError(SE_SOCKET_CREATE, __FUNCTION__, ::WSAGetLastError());

	return isOK;
}

BOOL CTcpAgent::CreateCompletePort()
{
	m_hCompletePort	= ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if(m_hCompletePort == nullptr)
		SetLastError(SE_CP_CREATE, __FUNCTION__, ::GetLastError());

	return (m_hCompletePort != nullptr);
}

BOOL CTcpAgent::CreateWorkerThreads()
{
	BOOL isOK = TRUE;

	for(DWORD i = 0; i < m_dwWorkerThreadCount; i++)
	{
		HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, WorkerThreadProc, (LPVOID)this, 0, nullptr);
		if(hThread)
			m_vtWorkerThreads.push_back(hThread);
		else
		{
			SetLastError(SE_WORKER_THREAD_CREATE, __FUNCTION__, ::GetLastError());
			isOK = FALSE;
			break;
		}
	}

	return isOK;
}

BOOL CTcpAgent::Stop()
{
	if(!CheckStoping())
		return FALSE;
	
	::WaitWithMessageLoop(150);

	DisconnectClientSocket();
	WaitForClientSocketClose();
	WaitForWorkerThreadEnd();
	
	ReleaseClientSocket();

	FireShutdown();

	ReleaseFreeSocket();
	ReleaseFreeBuffer();

	CloseCompletePort();

	Reset();

	return TRUE;
}

void CTcpAgent::Reset(BOOL bAll)
{
	if(bAll)
	{
		m_phSocket.Reset();
		m_phBuffer.Reset();
		m_itPool.Clear();
	}

	::ZeroMemory((void*)&m_soAddrIN, sizeof(SOCKADDR_IN));

	m_pfnConnectEx				= nullptr;
	m_pfnDisconnectEx			= nullptr;
	m_enState					= SS_STOPPED;
}

void CTcpAgent::DisconnectClientSocket()
{
	CReentrantReadLock locallock(m_csClientSocket);

	for(TSocketObjPtrMapI it = m_mpClientSocket.begin(), end = m_mpClientSocket.end(); it != end; ++it)
		Disconnect(it->first);
}

void CTcpAgent::ReleaseClientSocket()
{
	CReentrantWriteLock locallock(m_csClientSocket);

	for(TSocketObjPtrMapI it = m_mpClientSocket.begin(), end = m_mpClientSocket.end(); it != end; ++it)
	{
		TSocketObj* pSocketObj = it->second;

		CloseClientSocketObj(pSocketObj);
		DeleteSocketObj(pSocketObj);
	}

	m_mpClientSocket.clear();
}

TSocketObj*	CTcpAgent::GetFreeSocketObj(CONNID dwConnID, SOCKET soClient)
{
	TSocketObj* pSocketObj = nullptr;

	if(m_lsFreeSocket.size() > 0)
	{
		CCriSecLock locallock(m_csFreeSocket);

		if(m_lsFreeSocket.size() > 0)
		{
			pSocketObj = m_lsFreeSocket.front();

			if(::GetTimeGap32(pSocketObj->freeTime) >= m_dwFreeSocketObjLockTime)
				m_lsFreeSocket.pop_front();
			else
				pSocketObj = nullptr;
		}
	}

	if(!pSocketObj) pSocketObj = CreateSocketObj();
	pSocketObj->Reset(dwConnID, soClient);

	return pSocketObj;
}

void CTcpAgent::AddFreeSocketObj(TSocketObj* pSocketObj, EnSocketCloseFlag enFlag, EnSocketOperation enOperation, int iErrorCode)
{
	if(InvalidSocketObj(pSocketObj))
	{
		CloseClientSocketObj(pSocketObj, enFlag, enOperation, iErrorCode);
		TSocketObj::Release(pSocketObj);

		{
			CReentrantWriteLock locallock(m_csClientSocket);
			m_mpClientSocket.erase(pSocketObj->connID);
		}

		{
			CCriSecLock locallock(m_csFreeSocket);
			m_lsFreeSocket.push_back(pSocketObj);
		}

		if(m_lsFreeSocket.size() > m_dwFreeSocketObjHold)
			CompressFreeSocket(m_dwFreeSocketObjPool);
	}
}

BOOL CTcpAgent::InvalidSocketObj(TSocketObj* pSocketObj)
{
	BOOL bDone = FALSE;

	if(m_enRecvPolicy == RP_SERIAL)
	{
		if(TSocketObj::IsValid(pSocketObj))
		{
			CReentrantSpinLock	locallock(pSocketObj->csRecv);
			CCriSecLock			locallock2(pSocketObj->csSend);

			if(TSocketObj::IsValid(pSocketObj))
			{
				TSocketObj::Invalid(pSocketObj);
				bDone = TRUE;
			}
		}
	}
	else
	{
		if(TSocketObj::IsValid(pSocketObj))
		{
			CCriSecLock locallock(pSocketObj->csSend);

			if(TSocketObj::IsValid(pSocketObj))
			{
				TSocketObj::Invalid(pSocketObj);
				bDone = TRUE;
			}
		}
	}

	return bDone;
}

void CTcpAgent::AddClientSocketObj(CONNID dwConnID, TSocketObj* pSocketObj)
{
	ASSERT(FindSocketObj(dwConnID) == nullptr);

	pSocketObj->connTime	= ::TimeGetTime();
	pSocketObj->activeTime	= pSocketObj->connTime;

	CReentrantWriteLock locallock(m_csClientSocket);
	m_mpClientSocket[dwConnID] = pSocketObj;
}

void CTcpAgent::ReleaseFreeSocket()
{
	CompressFreeSocket(0, TRUE);
}

void CTcpAgent::CompressFreeSocket(size_t size, BOOL bForce)
{
	CCriSecLock locallock(m_csFreeSocket);

	DWORD now = ::TimeGetTime();

	while(m_lsFreeSocket.size() > size)
	{
		TSocketObj* pSocketObj = m_lsFreeSocket.front();

		if(bForce || (now - pSocketObj->freeTime) >= m_dwFreeSocketObjLockTime)
		{
			m_lsFreeSocket.pop_front();
			DeleteSocketObj(pSocketObj);
		}
		else
			break;
	}
}

TSocketObj* CTcpAgent::CreateSocketObj()
{
	TSocketObj* pSocketObj = (TSocketObj*)m_phSocket.Alloc(sizeof(TSocketObj));
	ASSERT(pSocketObj);

	pSocketObj->TSocketObj::TSocketObj(m_itPool);
	
	return pSocketObj;
}

void CTcpAgent::DeleteSocketObj(TSocketObj* pSocketObj)
{
	ASSERT(pSocketObj);

	pSocketObj->TSocketObj::~TSocketObj();
	m_phSocket.Free(pSocketObj);
}

TBufferObj* CTcpAgent::GetFreeBufferObj(int iLen)
{
	ASSERT(iLen >= 0 && iLen <= (int)m_dwSocketBufferSize);

	TBufferObj* pBufferObj = nullptr;

	if(m_lsFreeBuffer.size() > 0)
	{
		CCriSecLock locallock(m_csFreeBuffer);

		if(m_lsFreeBuffer.size() > 0)
		{
			pBufferObj = m_lsFreeBuffer.front();
			m_lsFreeBuffer.pop_front();
		}
	}

	if(!pBufferObj) pBufferObj = CreateBufferObj();

	if(iLen <= 0) iLen	 = m_dwSocketBufferSize;
	pBufferObj->buff.len = iLen;

	return pBufferObj;
}

void CTcpAgent::AddFreeBufferObj(TBufferObj* pBufferObj)
{
	if(m_lsFreeBuffer.size() < m_dwFreeBufferObjHold)
	{
		CCriSecLock locallock(m_csFreeBuffer);
		m_lsFreeBuffer.push_back(pBufferObj);
	}
	else
	{
		DeleteBufferObj(pBufferObj);
		CompressFreeBuffer(m_dwFreeBufferObjPool);
	}
}

void CTcpAgent::ReleaseFreeBuffer()
{
	CompressFreeBuffer(0);
}

void CTcpAgent::CompressFreeBuffer(size_t size)
{
	CCriSecLock locallock(m_csFreeBuffer);

	while(m_lsFreeBuffer.size() > size)
	{
		TBufferObj* pBufferObj = m_lsFreeBuffer.front();

		m_lsFreeBuffer.pop_front();
		DeleteBufferObj(pBufferObj);
	}
}

TBufferObj* CTcpAgent::CreateBufferObj()
{
	TBufferObj* pBufferObj = (TBufferObj*)m_phBuffer.Alloc(sizeof(TBufferObj) + m_dwSocketBufferSize);
	ASSERT(pBufferObj);

	::ZeroMemory(pBufferObj, sizeof(TBufferObj));
	pBufferObj->buff.buf = ((char*)pBufferObj) + sizeof(TBufferObj);

	return pBufferObj;
}

void CTcpAgent::DeleteBufferObj(TBufferObj* pBufferObj)
{
	ASSERT(pBufferObj);
	m_phBuffer.Free(pBufferObj);
}

TSocketObj* CTcpAgent::FindSocketObj(CONNID dwConnID)
{
	TSocketObj* pSocketObj = nullptr;

	CReentrantReadLock locallock(m_csClientSocket);

	TSocketObjPtrMapCI it = m_mpClientSocket.find(dwConnID);
	if(it != m_mpClientSocket.end())
		pSocketObj = it->second;

	return pSocketObj;
}

void CTcpAgent::CloseClientSocketObj(TSocketObj* pSocketObj, EnSocketCloseFlag enFlag, EnSocketOperation enOperation, int iErrorCode, int iShutdownFlag)
{
	ASSERT(TSocketObj::IsExist(pSocketObj));

	if(enFlag == SCF_CLOSE)
		FireClose(pSocketObj);
	else if(enFlag == SCF_ERROR)
		FireError(pSocketObj, enOperation, iErrorCode);

	SOCKET socket = pSocketObj->socket;
	pSocketObj->socket = INVALID_SOCKET;

	::ManualCloseSocket(socket, iShutdownFlag);
}

BOOL CTcpAgent::GetLocalAddress(CONNID dwConnID, TCHAR lpszAddress[], int& iAddressLen, USHORT& usPort)
{
	ASSERT(lpszAddress != nullptr && iAddressLen > 0);

	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
		return ::GetSocketLocalAddress(pSocketObj->socket, lpszAddress, iAddressLen, usPort);

	return FALSE;
}

BOOL CTcpAgent::GetRemoteAddress(CONNID dwConnID, TCHAR lpszAddress[], int& iAddressLen, USHORT& usPort)
{
	ASSERT(lpszAddress != nullptr && iAddressLen > 0);

	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsExist(pSocketObj))
	{
		ADDRESS_FAMILY usFamily;
		return ::sockaddr_IN_2_A(pSocketObj->remoteAddr, usFamily, lpszAddress, iAddressLen, usPort);
	}

	return FALSE;
}

BOOL CTcpAgent::SetConnectionExtra(CONNID dwConnID, PVOID pExtra)
{
	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsExist(pSocketObj))
	{
		pSocketObj->extra = pExtra;
		return TRUE;
	}

	return FALSE;
}

BOOL CTcpAgent::GetConnectionExtra(CONNID dwConnID, PVOID* ppExtra)
{
	ASSERT(ppExtra != nullptr);

	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsExist(pSocketObj))
	{
		*ppExtra = pSocketObj->extra;
		return TRUE;
	}

	return FALSE;
}

BOOL CTcpAgent::GetPendingDataLength(CONNID dwConnID, int& iPending)
{
	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
	{
		iPending = pSocketObj->Pending();
		return TRUE;
	}

	return FALSE;
}

DWORD CTcpAgent::GetConnectionCount()
{
	return (DWORD)m_mpClientSocket.size();
}

BOOL CTcpAgent::GetAllConnectionIDs(CONNID pIDs[], DWORD& dwCount)
{
	BOOL isOK	 = FALSE;
	DWORD dwSize = 0;

	{
		CReentrantReadLock locallock(m_csClientSocket);

		dwSize = (DWORD)m_mpClientSocket.size();

		if(pIDs != nullptr && dwSize <= dwCount)
		{
			TSocketObjPtrMapCI it	= m_mpClientSocket.begin();
			TSocketObjPtrMapCI end	= m_mpClientSocket.end();

			for(int i = 0; it != end; ++it, ++i)
				pIDs[i] = it->first;

			isOK = TRUE;
		}
	}

	dwCount = dwSize;
	return isOK;
}

BOOL CTcpAgent::GetConnectPeriod(CONNID dwConnID, DWORD& dwPeriod)
{
	BOOL isOK				= TRUE;
	TSocketObj* pSocketObj	= FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
		dwPeriod = ::GetTimeGap32(pSocketObj->connTime);
	else
		isOK = FALSE;

	return isOK;
}

BOOL CTcpAgent::GetSilencePeriod(CONNID dwConnID, DWORD& dwPeriod)
{
	if(!m_bMarkSilence)
		return FALSE;

	BOOL isOK				= TRUE;
	TSocketObj* pSocketObj	= FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
		dwPeriod = ::GetTimeGap32(pSocketObj->activeTime);
	else
		isOK = FALSE;

	return isOK;
}

BOOL CTcpAgent::Disconnect(CONNID dwConnID, BOOL bForce)
{
	BOOL isOK				= FALSE;
	TSocketObj* pSocketObj	= FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
	{
		if(bForce)
			isOK = ::PostIocpDisconnect(m_hCompletePort, dwConnID);
		else
			isOK = m_pfnDisconnectEx(pSocketObj->socket, nullptr, 0, 0);
	}

	return isOK;
}

BOOL CTcpAgent::DisconnectLongConnections(DWORD dwPeriod, BOOL bForce)
{
	ulong_ptr_deque ls;

	{
		CReentrantReadLock locallock(m_csClientSocket);

		DWORD now = ::TimeGetTime();

		for(TSocketObjPtrMapCI it = m_mpClientSocket.begin(), end = m_mpClientSocket.end(); it != end; ++it)
		{
			if(now - it->second->connTime >= dwPeriod)
				ls.push_back(it->first);
		}
	}
	
	for(ulong_ptr_deque::const_iterator it = ls.begin(), end = ls.end(); it != end; ++it)
		Disconnect(*it, bForce);

	return ls.size() > 0;
}

BOOL CTcpAgent::DisconnectSilenceConnections(DWORD dwPeriod, BOOL bForce)
{
	if(!m_bMarkSilence)
		return FALSE;

	ulong_ptr_deque ls;

	{
		CReentrantReadLock locallock(m_csClientSocket);

		DWORD now = ::TimeGetTime();

		for(TSocketObjPtrMapCI it = m_mpClientSocket.begin(), end = m_mpClientSocket.end(); it != end; ++it)
		{
			if(now - it->second->activeTime >= dwPeriod)
				ls.push_back(it->first);
		}
	}

	for(ulong_ptr_deque::const_iterator it = ls.begin(), end = ls.end(); it != end; ++it)
		Disconnect(*it, bForce);

	return ls.size() > 0;
}

void CTcpAgent::WaitForClientSocketClose()
{
	DWORD dwWait = 0;
	DWORD dwOrig = ::TimeGetTime();

	while(m_mpClientSocket.size() > 0)
	{
		::WaitWithMessageLoop(100);
		dwWait = ::GetTimeGap32(dwOrig);
	}

	ASSERT(m_mpClientSocket.size() == 0);
}

void CTcpAgent::WaitForWorkerThreadEnd()
{
	int count = (int)m_vtWorkerThreads.size();

	for(int i = 0; i < count; i++)
		::PostIocpExit(m_hCompletePort);

	int remain	= count;
	int index	= 0;

	while(remain > 0)
	{
		int wait = min(remain, MAXIMUM_WAIT_OBJECTS);
		HANDLE* pHandles = (HANDLE*)_alloca(sizeof(HANDLE) * wait);

		for(int i = 0; i < wait; i++)
			pHandles[i]	= m_vtWorkerThreads[i + index];

		VERIFY(::WaitForMultipleObjects((DWORD)wait, pHandles, TRUE, INFINITE) == WAIT_OBJECT_0);

		for(int i = 0; i < wait; i++)
			::CloseHandle(pHandles[i]);

		remain	-= wait;
		index	+= wait;
	}

	m_vtWorkerThreads.clear();
}

void CTcpAgent::CloseCompletePort()
{
	if(m_hCompletePort != nullptr)
	{
		::CloseHandle(m_hCompletePort);
		m_hCompletePort = nullptr;
	}
}

UINT WINAPI CTcpAgent::WorkerThreadProc(LPVOID pv)
{
	CTcpAgent* pServer = (CTcpAgent*)pv;

	while(TRUE)
	{
		DWORD dwErrorCode = NO_ERROR;

		DWORD dwBytes;
		OVERLAPPED* pOverlapped;
		TSocketObj* pSocketObj;
		
		BOOL result = ::GetQueuedCompletionStatus
												(
													pServer->m_hCompletePort,
													&dwBytes,
													(PULONG_PTR)&pSocketObj,
													&pOverlapped,
													INFINITE
												);

		if(pOverlapped == nullptr)
		{
			EnIocpAction action = pServer->CheckIocpCommand(pOverlapped, dwBytes, (ULONG_PTR)pSocketObj);

			if(action == IOCP_ACT_CONTINUE)
				continue;
			else if(action == IOCP_ACT_BREAK)
				break;
		}

		TBufferObj* pBufferObj	= CONTAINING_RECORD(pOverlapped, TBufferObj, ov);
		CONNID dwConnID			= pSocketObj->connID;

		if (!result)
		{
			DWORD dwFlag	= 0;
			DWORD dwSysCode = ::GetLastError();

			if(pServer->HasStarted())
			{
				SOCKET sock	= pBufferObj->client;
				result		= ::WSAGetOverlappedResult(sock, &pBufferObj->ov, &dwBytes, FALSE, &dwFlag);

				if (!result)
				{
					dwErrorCode = ::WSAGetLastError();
					TRACE("GetQueuedCompletionStatus error (<A-CNNID: %Iu> SYS: %d, SOCK: %d, FLAG: %d)\n", dwConnID, dwSysCode, dwErrorCode, dwFlag);
				}
			}
			else
				dwErrorCode = dwSysCode;

			ASSERT(dwSysCode != 0 && dwErrorCode != 0);
		}

		pServer->HandleIo(dwConnID, pSocketObj, pBufferObj, dwBytes, dwErrorCode);
	}

	return 0;
}

EnIocpAction CTcpAgent::CheckIocpCommand(OVERLAPPED* pOverlapped, DWORD dwBytes, ULONG_PTR ulCompKey)
{
	ASSERT(pOverlapped == nullptr);

	EnIocpAction action = IOCP_ACT_CONTINUE;

		if(dwBytes == IOCP_CMD_SEND)
			DoSend((CONNID)ulCompKey);
		else if(dwBytes == IOCP_CMD_DISCONNECT)
			ForceDisconnect((CONNID)ulCompKey);
		else if(dwBytes == IOCP_CMD_EXIT && ulCompKey == 0)
			action = IOCP_ACT_BREAK;
		else
			VERIFY(FALSE);

	return action;
}

void CTcpAgent::ForceDisconnect(CONNID dwConnID)
{
	AddFreeSocketObj(FindSocketObj(dwConnID), SCF_CLOSE);
}

void CTcpAgent::HandleIo(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj, DWORD dwBytes, DWORD dwErrorCode)
{
	ASSERT(pBufferObj != nullptr);
	ASSERT(pSocketObj != nullptr);

	if(dwErrorCode != NO_ERROR)
	{
		HandleError(dwConnID, pSocketObj, pBufferObj, dwErrorCode);
		return;
	}

	if(dwBytes == 0 && pBufferObj->operation != SO_CONNECT)
	{
		AddFreeSocketObj(pSocketObj, SCF_CLOSE);
		AddFreeBufferObj(pBufferObj);
		return;
	}

	pBufferObj->buff.len = dwBytes;

	switch(pBufferObj->operation)
	{
	case SO_CONNECT:
		HandleConnect(dwConnID, pSocketObj, pBufferObj);
		break;
	case SO_SEND:
		HandleSend(dwConnID, pSocketObj, pBufferObj);
		break;
	case SO_RECEIVE:
		HandleReceive(dwConnID, pSocketObj, pBufferObj);
		break;
	default:
		ASSERT(FALSE);
	}
}

void CTcpAgent::HandleError(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj, DWORD dwErrorCode)
{
	CheckError(pSocketObj, pBufferObj->operation, dwErrorCode);
	AddFreeBufferObj(pBufferObj);
}

void CTcpAgent::HandleConnect(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj)
{
	::SSO_UpdateConnectContext(pBufferObj->client, 0);

	BOOL bOnOff	= (m_dwKeepAliveTime > 0 && m_dwKeepAliveInterval > 0);
	::SSO_KeepAliveVals(pBufferObj->client, bOnOff, m_dwKeepAliveTime, m_dwKeepAliveInterval);

	if(FireConnect(dwConnID) != HR_ERROR)
		DoReceive(dwConnID, pSocketObj, pBufferObj);
	else
	{
		AddFreeSocketObj(pSocketObj, SCF_NONE);
		AddFreeBufferObj(pBufferObj);
	}
}

void CTcpAgent::HandleSend(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj)
{
	switch(m_enSendPolicy)
	{
	case SP_PACK:
		{
			long sndCount = ::InterlockedDecrement(&pSocketObj->sndCount);

			TriggerFireSend(dwConnID, pBufferObj);
			if(sndCount == 0) DoSendPack(pSocketObj);
		}

		break;
	case SP_SAFE:
		{
			long sndCount = ::InterlockedDecrement(&pSocketObj->sndCount);

			if(sndCount == 0 && !pSocketObj->smooth)
			{
				CCriSecLock locallock(pSocketObj->csSend);

				if((sndCount = pSocketObj->sndCount) == 0)
					pSocketObj->smooth = TRUE;
			}

			TriggerFireSend(dwConnID, pBufferObj);
			if(sndCount == 0) DoSendSafe(pSocketObj);
		}

		break;
	case SP_DIRECT:
		{
			TriggerFireSend(dwConnID, pBufferObj);
		}

		break;
	default:
		ASSERT(FALSE);
	}
}

void CTcpAgent::TriggerFireSend(CONNID dwConnID, TBufferObj* pBufferObj)
{
	if(FireSend(dwConnID, (BYTE*)pBufferObj->buff.buf, pBufferObj->buff.len) == HR_ERROR)
	{
		TRACE("<A-CNNID: %Iu> OnSend() event should not return 'HR_ERROR' !!\n", dwConnID);
		ASSERT(FALSE);
	}

	AddFreeBufferObj(pBufferObj);
}

void CTcpAgent::HandleReceive(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj)
{
	if(m_bMarkSilence) pSocketObj->activeTime = ::TimeGetTime();
	EnHandleResult hr = FireReceive(pSocketObj, (BYTE*)pBufferObj->buff.buf, pBufferObj->buff.len);

	if(hr == HR_OK || hr == HR_IGNORE)
		DoReceive(dwConnID, pSocketObj, pBufferObj);
	else if(hr == HR_CLOSED)
	{
		ASSERT(m_enRecvPolicy == RP_SERIAL);
		AddFreeBufferObj(pBufferObj);
	}
	else
	{
		TRACE("<A-CNNID: %Iu> OnReceive() event return 'HR_ERROR', connection will be closed !\n", dwConnID);
		AddFreeSocketObj(pSocketObj, SCF_ERROR, SO_RECEIVE, ERROR_CANCELLED);
		AddFreeBufferObj(pBufferObj);
	}
}

int CTcpAgent::DoReceive(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj)
{
	pBufferObj->buff.len = m_dwSocketBufferSize;
	int result =::PostReceive(pSocketObj, pBufferObj);

	if(result != NO_ERROR)
	{
		CheckError(pSocketObj, SO_RECEIVE, result);
		AddFreeBufferObj(pBufferObj);
	}

	return result;
}

BOOL CTcpAgent::Connect(LPCTSTR pszRemoteAddress, USHORT usPort, CONNID* pdwConnID)
{
	ASSERT(pszRemoteAddress && usPort != 0);

	if(pdwConnID) *pdwConnID = 0;

	DWORD result	= NO_ERROR;
	SOCKET soClient	= INVALID_SOCKET;

	if(!HasStarted())
		result = ERROR_INVALID_STATE;
	else
	{
		result = CreateClientSocket(soClient);

		if(result == NO_ERROR)
		{
			CONNID dwConnID	= ::GenerateConnectionID();
			if(pdwConnID)	*pdwConnID = dwConnID;

			if(FirePrepareConnect(dwConnID, soClient) != HR_ERROR)
				result = ConnectToServer(dwConnID, soClient, pszRemoteAddress, usPort);
			else
				result = ERROR_CANCELLED;
		}
	}

	if(result != NO_ERROR)
	{
		if(soClient != INVALID_SOCKET)
			::ManualCloseSocket(soClient);

		::SetLastError(result);
	}

	return (result == NO_ERROR);
}

DWORD CTcpAgent::CreateClientSocket(SOCKET& soClient)
{
	DWORD result = NO_ERROR;
	soClient	 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(soClient != INVALID_SOCKET)
	{
		VERIFY(::SSO_ReuseAddress(soClient, m_bReuseAddress) != SOCKET_ERROR);
		if(m_bAsyncConnect && ::bind(soClient, (SOCKADDR*)&m_soAddrIN, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			result = ::WSAGetLastError();
	}
	else
		result = ::WSAGetLastError();

	return result;
}

DWORD CTcpAgent::ConnectToServer(CONNID dwConnID, SOCKET& soClient, LPCTSTR pszRemoteAddress, USHORT usPort)
{
	TCHAR szAddress[40];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);

	if(!::GetIPAddress(pszRemoteAddress, szAddress, iAddressLen))
		return WSAEADDRNOTAVAIL;

	SOCKADDR_IN addr;
	if(!::sockaddr_A_2_IN(AF_INET, szAddress, usPort, addr))
		return WSAEADDRNOTAVAIL;

	TBufferObj* pBufferObj = GetFreeBufferObj();
	TSocketObj* pSocketObj = GetFreeSocketObj(dwConnID, soClient);

	memcpy(&pSocketObj->remoteAddr, &addr, sizeof(SOCKADDR_IN));
	AddClientSocketObj(dwConnID, pSocketObj);

	DWORD result	= NO_ERROR;
	BOOL bNeedFree	= TRUE;

	if(m_bAsyncConnect)
	{
		if(::CreateIoCompletionPort((HANDLE)soClient, m_hCompletePort, (ULONG_PTR)pSocketObj, 0))
		{
			result		= DoConnect(dwConnID, pSocketObj, pBufferObj);
			bNeedFree	= FALSE;
		}
		else
			result = ::GetLastError();
	}
	else
	{
		if(::connect(soClient, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) != SOCKET_ERROR)
		{
			if(::CreateIoCompletionPort((HANDLE)soClient, m_hCompletePort, (ULONG_PTR)pSocketObj, 0))
			{
				BOOL bOnOff	= (m_dwKeepAliveTime > 0 && m_dwKeepAliveInterval > 0);
				::SSO_KeepAliveVals(pSocketObj->socket, bOnOff, m_dwKeepAliveTime, m_dwKeepAliveInterval);

				if(FireConnect(dwConnID) != HR_ERROR)
				{
					result		= DoReceive(dwConnID, pSocketObj, pBufferObj);
					bNeedFree	= FALSE;
				}
				else
					result = ERROR_CANCELLED;
			}
			else
				result = ::GetLastError();
		}
		else
			result = ::WSAGetLastError();
	}

	if(result != NO_ERROR)
	{
		if(bNeedFree)
		{
			AddFreeSocketObj(pSocketObj, SCF_NONE);
			AddFreeBufferObj(pBufferObj);
		}

		soClient = INVALID_SOCKET;
	}

	return result;
}

int CTcpAgent::DoConnect(CONNID dwConnID, TSocketObj* pSocketObj, TBufferObj* pBufferObj)
{
	int result = ::PostConnect(m_pfnConnectEx, pSocketObj->socket, pSocketObj->remoteAddr, pBufferObj);

	if(result != NO_ERROR)
	{
		CheckError(pSocketObj, SO_CONNECT, result);
		AddFreeBufferObj(pBufferObj);
	}

	return result;
}

BOOL CTcpAgent::Send(CONNID dwConnID, const BYTE* pBuffer, int iLength, int iOffset)
{
	ASSERT(pBuffer && iLength > 0);

	if(iOffset != 0) pBuffer += iOffset;

	WSABUF buffer;
	buffer.len = iLength;
	buffer.buf = (char*)pBuffer;

	return SendPackets(dwConnID, &buffer, 1);
}

BOOL CTcpAgent::SendPackets(CONNID dwConnID, const WSABUF pBuffers[], int iCount)
{
	int result				= NO_ERROR;
	TSocketObj* pSocketObj	= nullptr;

	ASSERT(pBuffers && iCount > 0);

	if(pBuffers && iCount > 0)
	{
		pSocketObj = FindSocketObj(dwConnID);

		if(TSocketObj::IsValid(pSocketObj))
		{
			CCriSecLock locallock(pSocketObj->csSend);

			if(TSocketObj::IsValid(pSocketObj))
				result = SendInternal(pSocketObj, pBuffers, iCount);
			else
				result = ERROR_OBJECT_NOT_FOUND;
		}
		else
			result = ERROR_OBJECT_NOT_FOUND;
	}
	else
		result = ERROR_INVALID_PARAMETER;

	if(result != NO_ERROR)
	{
		if(m_enSendPolicy == SP_DIRECT && TSocketObj::IsValid(pSocketObj))
			CheckError(pSocketObj, SO_SEND, result);

		::SetLastError(result);
	}

	return (result == NO_ERROR);
}

int CTcpAgent::SendInternal(TSocketObj* pSocketObj, const WSABUF pBuffers[], int iCount)
{
	int result = NO_ERROR;

	for(int i = 0; i < iCount; i++)
	{
		int iBufLen = pBuffers[i].len;

		if(iBufLen > 0)
		{
			BYTE* pBuffer = (BYTE*)pBuffers[i].buf;
			ASSERT(pBuffer);

			switch(m_enSendPolicy)
			{
			case SP_PACK:	result = SendPack(pSocketObj, pBuffer, iBufLen);	break;
			case SP_SAFE:	result = SendSafe(pSocketObj, pBuffer, iBufLen);	break;
			case SP_DIRECT:	result = SendDirect(pSocketObj, pBuffer, iBufLen);	break;
			default: ASSERT(FALSE);	result = ERROR_INVALID_INDEX;				break;
			}

			if(result != NO_ERROR)
				break;
		}
	}

	return result;
}

int CTcpAgent::SendPack(TSocketObj* pSocketObj, const BYTE* pBuffer, int iLength)
{
	BOOL isPostSend = !TSocketObj::IsPending(pSocketObj);
	return CatAndPost(pSocketObj, pBuffer, iLength, isPostSend);
}

int CTcpAgent::SendSafe(TSocketObj* pSocketObj, const BYTE* pBuffer, int iLength)
{
	BOOL isPostSend = !TSocketObj::IsPending(pSocketObj) && TSocketObj::IsSmooth(pSocketObj);
	return CatAndPost(pSocketObj, pBuffer, iLength, isPostSend);
}

int CTcpAgent::SendDirect(TSocketObj* pSocketObj, const BYTE* pBuffer, int iLength)
{
	int result	= NO_ERROR;
	int iRemain	= iLength;

	while(iRemain > 0)
	{
		int iBufferSize = min(iRemain, (int)m_dwSocketBufferSize);
		TBufferObj* pBufferObj = GetFreeBufferObj(iBufferSize);
		memcpy(pBufferObj->buff.buf, pBuffer, iBufferSize);

		result = ::PostSend(pSocketObj, pBufferObj);

		if(result != NO_ERROR)
		{
			AddFreeBufferObj(pBufferObj);
			break;
		}

		iRemain -= iBufferSize;
		pBuffer += iBufferSize;
	}

	return result;
}

int CTcpAgent::CatAndPost(TSocketObj* pSocketObj, const BYTE* pBuffer, int iLength, BOOL isPostSend)
{
	int result = NO_ERROR;

	pSocketObj->sndBuff.Cat(pBuffer, iLength);
	pSocketObj->pending += iLength;

	if(isPostSend && !::PostIocpSend(m_hCompletePort, pSocketObj->connID))
		result = ::GetLastError();

	return result;
}

int CTcpAgent::DoSend(CONNID dwConnID)
{
	TSocketObj* pSocketObj = FindSocketObj(dwConnID);

	if(TSocketObj::IsValid(pSocketObj))
		return DoSend(pSocketObj);

	return ERROR_OBJECT_NOT_FOUND;
}

int CTcpAgent::DoSend(TSocketObj* pSocketObj)
{
	switch(m_enSendPolicy)
	{
	case SP_PACK:			return DoSendPack(pSocketObj);
	case SP_SAFE:			return DoSendSafe(pSocketObj);
	default: ASSERT(FALSE);	return ERROR_INVALID_INDEX;
	}
}

int CTcpAgent::DoSendPack(TSocketObj* pSocketObj)
{
	int result = NO_ERROR;

	if(TSocketObj::IsPending(pSocketObj))
	{
		CCriSecLock locallock(pSocketObj->csSend);

		if(TSocketObj::IsValid(pSocketObj))
			result = SendItem(pSocketObj);
	}

	if(!IOCP_SUCCESS(result))
		CheckError(pSocketObj, SO_SEND, result);

	return result;
}

int CTcpAgent::DoSendSafe(TSocketObj* pSocketObj)
{
	int result = NO_ERROR;

	if(TSocketObj::IsPending(pSocketObj) && TSocketObj::IsSmooth(pSocketObj))
	{
		CCriSecLock locallock(pSocketObj->csSend);

		if(TSocketObj::IsPending(pSocketObj) && TSocketObj::IsSmooth(pSocketObj))
		{
			pSocketObj->smooth = FALSE;

			result = SendItem(pSocketObj);

			if(result == NO_ERROR)
				pSocketObj->smooth = TRUE;
		}
	}

	if(!IOCP_SUCCESS(result))
		CheckError(pSocketObj, SO_SEND, result);

	return result;
}

int CTcpAgent::SendItem(TSocketObj* pSocketObj)
{
	int result = NO_ERROR;

	while(pSocketObj->sndBuff.Size() > 0)
	{
		::InterlockedIncrement(&pSocketObj->sndCount);

		TItemPtr itPtr(m_itPool, pSocketObj->sndBuff.PopFront());

		int iBufferSize = itPtr->Size();
		ASSERT(iBufferSize > 0 && iBufferSize <= (int)m_dwSocketBufferSize);

		pSocketObj->pending   -= iBufferSize;
		TBufferObj* pBufferObj = GetFreeBufferObj(iBufferSize);
		memcpy(pBufferObj->buff.buf, itPtr->Ptr(), iBufferSize);

		result = ::PostSendNotCheck(pSocketObj, pBufferObj);

		if(result != NO_ERROR)
		{
			if(result != WSA_IO_PENDING)
				AddFreeBufferObj(pBufferObj);;

			break;
		}
	}

	return result;
}

BOOL CTcpAgent::SendSmallFile(CONNID dwConnID, LPCTSTR lpszFileName, const LPWSABUF pHead, const LPWSABUF pTail)
{
	ASSERT(lpszFileName != nullptr);

	CAtlFile file;
	HRESULT hr = file.Create(lpszFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);

	if(SUCCEEDED(hr))
	{
		ULONGLONG ullLen;
		hr = file.GetSize(ullLen);

		if(SUCCEEDED(hr))
		{
			if(ullLen > 0 && ullLen <= MAX_SMALL_FILE_SIZE)
			{
				CAtlFileMapping<> fmap;
				hr = fmap.MapFile(file);

				if(SUCCEEDED(hr))
				{
					WSABUF bufs[3] = {0};

					bufs[1].len = (ULONG)ullLen;
					bufs[1].buf = fmap;

					if(pHead) memcpy(&bufs[0], pHead, sizeof(WSABUF));
					if(pTail) memcpy(&bufs[2], pTail, sizeof(WSABUF));

					return SendPackets(dwConnID, bufs, 3);
				}
			}
			else if(ullLen == 0)
				hr = HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
			else
				hr = HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
		}
	}

	::SetLastError(hr & 0x0000FFFF);

	return FALSE;
}

void CTcpAgent::CheckError(TSocketObj* pSocketObj, EnSocketOperation enOperation, int iErrorCode)
{
	if(iErrorCode != WSAENOTSOCK && iErrorCode != ERROR_OPERATION_ABORTED)
		AddFreeSocketObj(pSocketObj, SCF_ERROR, enOperation, iErrorCode);
}
