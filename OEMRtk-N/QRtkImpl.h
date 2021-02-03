#pragma once

#include <fstream>

#ifdef  _EC100Y_PLATFORM
extern "C" {
#include <ql_rtos.h>
}
#else
#include <thread>
#include <chrono>
#include <mutex>
#endif
extern "C" {
#include "QRTKInterface.h"
#include "libRtkAPI.h"
}
#include "RawFormat.h"
#include "QRtkParameter.h"
#include "RtcmDecode.h"
typedef struct tag_SatAmAltitu
{
	BYTE pbID[MAXSAT];
	double pdDistance[MAXSAT];
#ifdef  _EC100Y_PLATFORM
	double pdSatClock[MAXSAT];
	double pddr[MAXSAT];
#endif
	double pdAzimuth[MAXSAT];
	double pdElevation[MAXSAT];
#ifdef  _EC100Y_PLATFORM
	double ARP[3];
	double pdSatX[MAXSAT];
	double pdSatY[MAXSAT];
	double pdSatZ[MAXSAT];
#endif
} SatAmAltitu;
#define SAT_AM_ALTIT_SIZE sizeof(SatAmAltitu)

#define   QRTK_ENCODE_BUFFER_MAX_SIZE    8192 

class CQRtkImpl
{
public:
	explicit CQRtkImpl(pRtkReport callback);
	~CQRtkImpl();

	void CheckKeyForQRtk();

    int PushRtcm(int source, const char *Buffer, int iDataSize);
    int PushEpoch(int source, const WangEpoch* pEpoch);
	void SendEphemerisToQRtk(ServerEphemeris* pEphemeris);
	void SendGloEphemToQRtk(ServerGloEphemeris *pEphem);

    int PushGGA(int source, const char* msg, int len);
    //void SetCoordinateofGGA(double *xyz);

	void SendEpochToQRtk(WangEpoch* pBSEpoch, WangEpoch *pRVEpoch, RTK_result *pRtkRes, char* RtkDebug);
	void SendCommandToQRtk(int iMode, int iParam);
	int  GetCommandOfQRtk(int iMode);
	void EncodeQRtkHeader(BYTE* pBuf, UINT32 iMsgId, UINT32 iLen, UINT16 wWeek, double dSecond);
	void SetQRtkViewEnable(const bool _bViewEnable);
	void SetQRtkLogEnable(const bool _bLogEnable);
	void SetQRtkOutputFormat(const int _iOutputFormat);

	void SetQRTKViewEnable(const bool _bViewEnable);
	void SetQRTKLogEnable(const bool _bLogEnable);
	void SetQRTKOutputFormat(const int _iOutputFormat);
	void SetupQRtk(int iMode, int iParam);
	int  GetQRtkSetup(const int iMode);
	UINT32 GetM919Info(WangEpoch *pBsEpoch,WangEpoch *pRvEpoch);
	UINT32 GetM916Info(WangEpoch *pBsEpoch,WangEpoch *pRvEpoch);
private:
	unsigned int CutDouble2UInt(double dsource);
	unsigned int CutDouble2Int(double dsource,unsigned int *pBuff);
	int  GetSatPos(const int nSat, const double t, double *xyz, SatAmAltitu *pSatAm);
	void GetSinglePosition(WangEpoch *pEpoch, WangEpoch *pLastEpoch, Coordinate *pXyz);
    int ParseRtcm(int source, const char *Buffer, int iDataSize);
	void CreateMes723Info(ServerGloEphemeris *pGloEph, Mes723Infor *pMes723);
    bool PrepareToSolve(int source);
    int Solve();
	void CaculateEpochDoppler(WangEpoch* pEpochA, WangEpoch* pEpochB);
	bool CreateM919TotalInfor(WangEpoch *pBsEpoch, WangEpoch *pRvEpoch, M919TotalInfor& m919TInfo );
	bool CreateM916TotalInfor(WangEpoch *pBsEpoch,WangEpoch *pRvEpoch,MixEpoch& m916TInfo);

	// ---------------919报文编码-陆小路-------------
	void GetNovHeader(UINT32 iMsgId, UINT32 iMsgLen, UINT32 iVer, RawHeader* pHeader, UINT32 iWeek_, UINT32 iSecond_);
	BYTE GetM919SatNum(GnssType Gnss, M919TotalInfor& M919Infor);
	BYTE GetM919FrqNum(GnssType Gnss, BYTE bGnssFrqFlag);
	void EncodeM919_2B(M919TotalInfor& M919Infor, BYTE* chbuff_M919_Total, UINT32& iMesLen, UINT32 iMsgId);
	void EncodeM916_2B(MixEpoch *pM,BYTE* pBuf,UINT32& iMesLen,UINT32 iMsgId);
	UINT32 EncodeOneDecEphGlo(UINT16 wWeek_GPS, UINT32 iSecond_GPS, Mes723Infor* pEph, BYTE* pMesTotal, const UINT32 KTOTALMESLEN);
	UINT32 Encode_BD2DecodedEphInfor(UINT16 wWeek_GPS, UINT32 iSecond_GPS, BYTE *M71_Total,	ServerEphemeris *EphData71, const UINT32 TotalLength_M71);
	
private:

    pRtkReport          m_pRtkReport = nullptr;

    int					m_iLeapSecond = 18;
	bool				m_bHasRovCoord = false;
    bool                m_bHasRovAllEpoch = false;
	WangEpoch			m_rovEpoch;
	ServerEphemeris		m_ephemGPS;
	ServerGloEphemeris	m_ephemGLO;
    Coordinate          m_rovCoord;
#ifdef  _EC100Y_PLATFORM
    char                m_rovRtcmBuffer[QRTK_RTCM_BUFFER_MAX_SIZE];
#else
    char                *m_rovRtcmBuffer = nullptr;
#endif
    int                 m_rovRtcmRemainSize = 0;
    CXRawFormat         m_rovRawFormat;

#ifdef  _EC100Y_PLATFORM
    ql_mutex_t          m_mtxBase = NULL;
#else
    std::mutex          m_mtxBase;
#endif
	bool				m_bHasBaseCoord = false;
    bool                m_bHasBaseAllEpoch = false;
    bool                m_bHasBaseLastAllEpoch = false;
    WangEpoch			m_baseEpoch;
    Coordinate          m_baseCoord;
#ifdef  _EC100Y_PLATFORM
    char                m_baseRtcmBuffer[QRTK_RTCM_BUFFER_MAX_SIZE];
#else
    char                *m_baseRtcmBuffer = nullptr;
#endif
    int                 m_baseRtcmRemainSize = 0;
    CXRawFormat         m_baseRawFormat;

	volatile int 		m_nWeekCurForRTCM;
	volatile double 	m_dSecondCurForRTCM;
	int 			    NCycle[3][MAXPRN];
	WangEpoch           m_rovLastEpoch ;
	WangEpoch           m_baseLastEpoch ;

    SatAmAltitu         m_satAm;
	XPublic			    m_theX;
	RTK_HANDLE			m_handle;
    msm_b_t             m_msmTemp;
#ifdef  _EC100Y_PLATFORM
    char                m_encodeBuffer[QRTK_ENCODE_BUFFER_MAX_SIZE];
#else
    char                *m_encodeBuffer = nullptr;
#endif
};
