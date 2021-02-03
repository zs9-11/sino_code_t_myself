#include <ctype.h>
#include <string.h>
#include "RawFormat.h"
#include "RtcmDecode.h"

using namespace std;

CXRawFormat::CXRawFormat()
{
	m_iPage = 0;
	m_wLength = 0;
	m_wWholeLength = 0;
	m_iState = 0;
	m_iPosition = 0;
    //m_buffer = new char[MAX_NOV_LONG + 16];
    m_buffer = new BYTE[MAX_NOV_LONG + 16];
}

CXRawFormat::~CXRawFormat()
{
    if(m_buffer){
        delete[] m_buffer; 
    }
}

void CXRawFormat::reset()
{
    m_iPage = 0;
	m_wLength = 0;
	m_wWholeLength = 0;
	m_iState = 0;
	m_iPosition = 0;
    memset(m_buffer, 0x00, MAX_NOV_LONG + 16);
}


WORD CXRawFormat::wGetOneCharOfRTCM32(unsigned char ch)
{
	//BOOL	bGetWholeFrame = false;
	bool	bGetWholeFrame = false;
	
    switch (m_iState)
	{
	case 0:
		if ((ch & 0xFF) == 0xD3)
		{
			m_iPosition = 0;
			m_buffer[m_iPosition++] = ch;
			m_iState++;

            //printf("RTK Debug, wGetOneCharOfRTCM32 parse RTCM:%d\n", m_iState);
		}
		break;
	case 1:
		m_buffer[m_iPosition++] = ch;
		m_iState++;

        //printf("RTK Debug, wGetOneCharOfRTCM32 parse RTCM:%d\n", m_iState);
		break;
	case 2:
	{
		m_buffer[m_iPosition++] = ch;
		m_wLength = getbitu(m_buffer, 14, 10);

		int nCheck = getbitu(m_buffer, 8, 6);
		if ((m_wLength <= 4) || nCheck)
		{
			m_iState = 0;
		}
		else{
			m_iState++;
            //printf("RTK Debug, wGetOneCharOfRTCM32 parse RTCM:%d\n", m_iState);
        }
	}
		break;
	case 3:
		m_buffer[m_iPosition++] = ch;
		if (m_iPosition == (m_wLength + 6))
		{
			/*bGetWholeFrame = iCheckSumForTrimble();*/
			if (crc24q(m_buffer, (3 + m_wLength)) == getbitu(m_buffer + 3 + m_wLength, 0, 24))
			{
				m_wType = getbitu(m_buffer, 24, 12);
				m_wWholeLength = m_wLength + 6;
				//bGetWholeFrame = TRUE;
				bGetWholeFrame = true;
			}
			else
			{
				printf("RTK Debug, parse RTCM CRC fail pos:%d, state:%d\n", m_iPosition, m_iState);
			}

			m_iState = 0;
			m_iPosition = 0;
		}
		break;
	default:
		m_iState = 0;
		break;
	}
	if (bGetWholeFrame) return m_wType;
	else return 0;
}
