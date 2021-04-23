#if !defined(AFX_CRYPT_H__413D055F_F135_4B2E_A4C3_19C1E8937B86__INCLUDED_)
#define AFX_CRYPT_H__413D055F_F135_4B2E_A4C3_19C1E8937B86__INCLUDED_


//////////////////////////////////////////////////////////////////////////////
//
//	Simple Crypt Base Class
//
//////////////////////////////////////////////////////////////////////////////

class CSimpleCrypt
{
public:
	CSimpleCrypt();
	virtual ~CSimpleCrypt();

protected:
	int		m_ConstKey1;
	int		m_ConstKey2;
	int		m_DynamicKey;

	void	Encrypt32bit(const char* pSrc, char* pDest);
	void	Decrypt32bit(const char* pSrc, char* pDest);

public:

	void	SetKey(int ConstKey1, int ConstKey2, int DynamicKey);
	void	Encrypt(void* pData, int nSize);
	void	Decrypt(void* pData, int nSize);
};


//////////////////////////////////////////////////////////////////////////////
//
//	Optimized Crypt Class for A3
//
//////////////////////////////////////////////////////////////////////////////

class CMsgCrypt : public CSimpleCrypt
{
public:
	CMsgCrypt();
	virtual ~CMsgCrypt();

protected:
	int		m_iMsgHeaderSize;

public:
	void	SetMsgHeaderSize(int MsgHeaderSize);

public:
	void	EncryptMsgBody(void* pData, int nSize);
	void	DecryptMsgBody(void* pData, int nSize);
};


#endif // !defined(AFX_CRYPT_H__413D055F_F135_4B2E_A4C3_19C1E8937B86__INCLUDED_)
