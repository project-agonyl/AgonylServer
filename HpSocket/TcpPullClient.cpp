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
#include "TcpPullClient.h"

EnHandleResult CTcpPullClient::FireReceive(IClient* pClient, const BYTE* pData, int iLength)
{
	m_lsBuffer.Cat(pData, iLength);
	m_iTotalLength += iLength;

	return __super::FireReceive(pClient, m_iTotalLength);
}

EnFetchResult CTcpPullClient::Fetch(BYTE* pData, int iLength)
{
	ASSERT(pData != nullptr && iLength > 0);

	if(m_iTotalLength >= iLength)
	{
		m_lsBuffer.Fetch(pData, iLength);
		m_iTotalLength -= iLength;

		return FR_OK;
	}

	return FR_LENGTH_TOO_LONG;
}

EnFetchResult CTcpPullClient::Peek(BYTE* pData, int iLength)
{
	ASSERT(pData != nullptr && iLength > 0);

	if(m_iTotalLength >= iLength)
	{
		m_lsBuffer.Peek(pData, iLength);
		return FR_OK;
	}

	return FR_LENGTH_TOO_LONG;
}
