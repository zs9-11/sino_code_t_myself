#pragma once

#include "Common.h"

#define	WM_DOWNLOADING		100
#define	MAX_NOV_LONG		1024

class CXRawFormat
{
public:
	CXRawFormat();
	virtual ~CXRawFormat();
    void    reset();
	WORD	wGetOneCharOfRTCM32(unsigned char ch);

	WORD	m_wWholeLength;
	//BYTE	m_buffer[MAX_NOV_LONG + 16];
	BYTE	*m_buffer;

	int		m_iState;
private:
	//int		m_iState;
	BYTE	m_iPage;	
	int		m_iPosition;
	WORD	m_wType;
	WORD	m_wLength;

};


