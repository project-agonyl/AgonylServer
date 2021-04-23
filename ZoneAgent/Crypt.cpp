#include "stdafx.h"
#include "Crypt.h"

//////////////////////////////////////////////////////////////////////////////
//
//	Simple Crypt Base Class
//
//////////////////////////////////////////////////////////////////////////////


CSimpleCrypt::CSimpleCrypt()
{
	m_ConstKey1 = 2366183;
	m_ConstKey2 = 1432754;
	m_DynamicKey = 79984829;
}

CSimpleCrypt::~CSimpleCrypt()
{
}

void CSimpleCrypt::Encrypt32bit(const char* pSrc, char* pDest)
{
	int DynamicKey = m_DynamicKey;

	for (int i = 0; i < 4; i++)
	{
		pDest[i] = (char)((BYTE)(pSrc[i]) ^ (DynamicKey >> 8));
		DynamicKey = ((BYTE)(pDest[i]) + DynamicKey) * m_ConstKey1 + m_ConstKey2;
	}
}

void CSimpleCrypt::Decrypt32bit(const char* pSrc, char* pDest)
{
	int DynamicKey = m_DynamicKey;

	for (int i = 0; i < 4; i++)
	{
		pDest[i] = (char)((BYTE)(pSrc[i]) ^ (DynamicKey >> 8));
		DynamicKey = ((BYTE)(pSrc[i]) + DynamicKey) * m_ConstKey1 + m_ConstKey2;
	}
}

void CSimpleCrypt::SetKey(int ConstKey1, int ConstKey2, int DynamicKey)
{
	m_ConstKey1 = ConstKey1;
	m_ConstKey2 = ConstKey2;
	m_DynamicKey = DynamicKey;
}

void CSimpleCrypt::Encrypt(void* pData, int nSize)
{
	char pBufSrc[4], pBufDest[4];
	char* p = (char*)pData;
	int j = 0;
	for (int i = 0; i < (int)(nSize / 4); i++)
	{
		for (j = 0; j < 4; j++)
		{
			pBufSrc[j] = *(p + j);
		}

		Encrypt32bit(pBufSrc, pBufDest);

		for (j = 0; j < 4; j++)
		{
			*(p + j) = pBufDest[j];
		}

		p = p + 4;
	}
}

void CSimpleCrypt::Decrypt(void* pData, int nSize)
{
	char pBufSrc[4], pBufDest[4];
	char* p = (char*)pData;
	int j = 0;
	for (int i = 0; i < (int)(nSize / 4); i++)
	{
		for (j = 0; j < 4; j++)
		{
			pBufSrc[j] = *(p + j);
		}

		Decrypt32bit(pBufSrc, pBufDest);

		for (j = 0; j < 4; j++)
		{
			*(p + j) = pBufDest[j];
		}

		p = p + 4;
	}
}



//////////////////////////////////////////////////////////////////////////////
//
//	Optimized Crypt Class for A3
//
//////////////////////////////////////////////////////////////////////////////


CMsgCrypt::CMsgCrypt()
{
	m_iMsgHeaderSize = 12;
}

CMsgCrypt::~CMsgCrypt()
{
}

void	CMsgCrypt::SetMsgHeaderSize(int MsgHeaderSize)
{
	m_iMsgHeaderSize = MsgHeaderSize;
}

void	CMsgCrypt::EncryptMsgBody(void* pData, int nSize)
{
	char* pMsgBody = (char*)pData;
	pMsgBody += m_iMsgHeaderSize;

	Encrypt(pMsgBody, nSize - m_iMsgHeaderSize);
}

void	CMsgCrypt::DecryptMsgBody(void* pData, int nSize)
{
	char* pMsgBody = (char*)pData;
	pMsgBody += m_iMsgHeaderSize;

	Decrypt(pMsgBody, nSize - m_iMsgHeaderSize);
}