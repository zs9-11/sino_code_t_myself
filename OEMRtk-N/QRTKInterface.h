#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "Common.h"
 
typedef struct __M723DATA
{
	UINT32 iPrn;
	UINT16 wSloto;
	UINT16 wFreqo;
	BYTE bSatType;
	BYTE bRes1;
	UINT16 wEphWeek;
	UINT32 iEphTime;
	UINT32 iTimeOffset;
	UINT16 wDateNum;
	BYTE bRes2;
	BYTE bRes3;
	UINT32 iIssue;
	UINT32 iHealth;
	double dPosX;
	double dPosY;
	double dPosZ;
	double dVelX;
	double dVelY;
	double dVelZ;
	double dAccX;
	double dAccY;
	double dAccZ;
	double dTauN;
	double dDeltaTauN;
	double dGamma;
	UINT32 iTk;
	UINT32 iP;
	UINT32 iFt;
	UINT32 iAge;
	UINT32 iFlag;
	UINT32 iValid;
}Mes723Infor;

typedef struct _SAT_TO_STATION_PARA_
{
	int iValid;
	float fI;
	double dL, dM, dN, dR;
}SAT_TO_STN_PARA;

typedef struct __M91xFrqData
{
	BYTE 	Status;
	BYTE 	SNR;
	BYTE 	RMS;
	BYTE 	LostCount;


	double	PR_SD_or_Rov_PR;//
	float	CP_SDMinusPR_SD;
	UINT32	iCp;
	float	fSatClkDt;

}M91xFrqInfor;

typedef struct __M919SATDATA
{
	BYTE 	PRN;
	BYTE   	ElevationDegree;
	short 	AzimuthDegree;

	M91xFrqInfor Lx[3];

	SAT_TO_STN_PARA SatToRovInfo;
	SAT_TO_STN_PARA SatToRefInfo;

	BYTE	IonoDly;
	BYTE    bSoc;
	UINT16  dwDeltaPhi;

	short	dwDx;				//SBAS卫星播发的卫星位置矫正量；
	short	dwDy;
	short	dwDz;
	UINT16	wSIc;

	short 	dwRefPrDiff;
	//float   fSatClkDtSbas;		//来自SBAS改正信息的卫星钟差延迟；			20160509
}Mes919Infor;

typedef struct __M919SPPREFDATA
{
	double dRov_SPP_Time;
	double dRov_SPP_x;
	double dRov_SPP_y;
	double dRov_SPP_z;

	double dRov_SPP_vx;
	double dRov_SPP_vy;
	double dRov_SPP_vz;

	UINT32 iRef_ID;
	double dRef_Pos_Time;
	double dRef_Pos_x;
	double dRef_Pos_y;
	double dRef_Pos_z;
}Message919spprefdata;

typedef struct __M919MesTotal
{
	unsigned short wVersion;
	unsigned short wWeek_GPS;
	double dSecond_GPS;

	BYTE bSatNo;
	BYTE bFrqFlag;
	BYTE bFrqFlagHigh;
	BYTE bReserved0;

	BYTE bMesSta_GPS;
	BYTE bMesSta_BD2;
	BYTE bMesSta_GLO;
	BYTE bMesSta_GAL;

	int iTimeOffset_GPS;
	int iTimeOffset_BD2;
	int iTimeOffset_GLO;
	int iTimeOffset_GAL;

	UINT32 iRefPpsDiffValid;
	UINT32 iRefPpsDiff;
	UINT32 iRovPpsDiffValid;
	UINT32 iRovPpsDiff;

	double dRefPvtTimeDiff_GPS;
	double dRefPvtTimeDiff_BD2;
	double dRefPvtTimeDiff_GLO;
	double dRefPvtTimeDiff_GAL;

	double dRovPvtTimeDiff_GPS;
	double dRovPvtTimeDiff_BD2;
	double dRovPvtTimeDiff_GLO;
	double dRovPvtTimeDiff_GAL;

	Mes919Infor SatObsInfor[40];//20151027;TOTAL_CHANNELS+1

	Message919spprefdata SppRefInfor;

	double dBd2Time2GpsTime;	//北斗时间与GPS时间差的小数部分；
	double dGloTime2GpsTime;	//GLONASS时间与GPS时间差的小数部分；
	double dGalTime2GpsTime;	//GALILEO时间与GPS时间差的小数部分；

	double dDtGps;				//PVT中GPS钟差；
	double dDtBds;				//PVT中BDS钟差；
	double dDtGlo;				//PVT中GLO钟差；
	double dDtGal;				//PVT中GAL钟差；	

	BYTE bRefRcvrTypeId;		//参考站接收机类型；lrp@20151118
	BYTE bReserved1;
	BYTE bReserved2;
	BYTE bReserved3;
	int iImuDataValid;
	UINT32 iImuDataSts;
	double pdGyro[ 3 ];
	double pdAcc[ 3 ];
	float fImuTemp;
}M919TotalInfor;

enum GnssType
{
	ALEX_GPS = 0,
	ALEX_GLONASS = 1,
	ALEX_SBAS = 2,
	ALEX_GALILEO = 3,
	ALEX_BD2 = 4,
	ALEX_OTHER = 7,
	ALEX_GNSS_ALL = 25
};

const GnssType GNSS_PRN[201] =
{
	ALEX_OTHER,																			//  0
	ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS,	//  1~ 16
	ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS, ALEX_GPS,	// 17~ 32

	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER,													// 33~ 37

	ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS,			// 38~ 45
	ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS,			// 46~ 53
	ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS, ALEX_GLONASS,			// 54~ 61

	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER,							// 62~ 70
	ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO,					// 71~ 80
	ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO,					// 81~ 90
	ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO,					// 91~100
	ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO, ALEX_GALILEO,					//101~110
	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER,							//111~119

	ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, 	//120~129								//120~129
	ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS, ALEX_SBAS,				//130~138								//130~138

	ALEX_OTHER, ALEX_OTHER,																	//139~140

	ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2,										//141~150
	ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2,										//151~160		
	ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2,										//161~170
	ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2, ALEX_BD2,													//171~177

	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, 															//178~180
	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER,					//181~190
	ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER, ALEX_OTHER,					//191~200
};

typedef struct __NOVATELRAWHEADER
{
	char Sync1;
	char Sync2;
	char Sync3;

	BYTE HeaderLength;
	unsigned short MessageID;
	char MessageType;
	BYTE PortAddress;
	unsigned short MessageLength;

	unsigned short Sequence;
	BYTE IdleTime;
	BYTE TimeStatus;
	UINT16 Week;
	UINT32 GPSec;
	unsigned int ReceiverStatus;
	unsigned short Reserved;
	unsigned short ReceiverVersion;
}RawHeader;

enum M919STATUS
{
	SINGLE_DIFF_MODE = 0,	//单差模式； 
	RAW_PR_MODE = 1, 		//原始伪距模式；
	SYN_TIME_NOT_SAME = 2, 	//同步模式中，几个系统的时间不一致，将较旧的数据标志为2；
	CORRECTED_PR_MODE = 3	//改正伪距模式；
};

enum BD2_FRQ
{
	B1 = 0,
	B2 = 1,
	B3 = 2
};

enum GPS_FRQ
{
	L1 = 0,
	L2 = 1,
	L5 = 2
};

enum GLO_FRQ
{
	G1 = 0,
	G2 = 1,
	G3 = 2
};
enum GAL_FRQ
{
	E1 = 0,
	E5B = 1,
	E5A = 2,
	E2 = 1,
	E3 = 2,
};

#define SizeOfRawHeader 					28
#define MaxSatelliteNumber 					50
//#define SizeOfRawHeader  					28
#define TOTAL_CHANNELS						64
//#define BD2_SatID_Offset 					140
////////////////////////////////////////////////////////////////
//// Contants for the GPS system.
//#define NZ			1.0E-20
//#define LARGE							1.0E20
//#define WE 								7.2921151467E-5      /* Earths rotation rate (rads/sec) */
//#define WL								0.1902936728
//#define OMEGA							7.2921151467E-5
//#define A84								6378137.0
//#define F84								1 / 298.2572235634

#define M919_VERSION					0x0056		// M919版本号
#define M916_VERSION					0x0001		// M919版本号

#define CYCLE_LIMIT						127.0

//#define MAXPRN							200			// 卫星编号(GPS:1-37,GLONASS:38-61,SBAS:120-138, BD2:141-178)
#define REFPRDIFF_FACTOR				128
#define LIGHT				299792458.0			// light speed
/*---------------------------- GPS -------------------------------------------*/
/* #define GPS_F1				1.57542E9 */
/* #define GPS_F2				1.22760E9 */
/* #define GPS_F5				1.17645E9 */
#define GPS_F1				1575420000
#define GPS_F2				1227600000
#define GPS_F5				1176450000
/*---------------------------- Glonass ---------------------------------------*/
/* #define GLO_L1_BASE_FRE		1602E6		//MHz; */
/* #define GLO_L2_BASE_FRE		1246E6		//MHz */
/* #define GLO_L1_SUB_BAND		0.5625E6	//; */
/* #define GLO_L2_SUB_BAND		0.4375E6	//; */

 #define GLO_L1_BASE_FRE		1602000000	//MHz
 #define GLO_L2_BASE_FRE		1246000000	//MHz 

#define GLO_L1		        1602000000		//MHz
#define GLO_L2		        1246000000		//MHz
#define GLO_L1_SUB_BAND		562500	        //KHz
#define GLO_L2_SUB_BAND		437500	        //KHz

/*---------------------------- Compass ---------------------------------------*/
/* #define BD2_F1				1.561098E9 */
/* #define BD2_F2				1.207140E9 */
/* #define BD2_F3				1.268520E9 */
/* #define BD3_F1C				1.575420E9 */
/* #define BD3_F2A				1.176450E9 */
#define BD2_F1				1561098000      //1561.098MHZ
#define BD2_F2				1207140000
#define BD2_F3				1268520000
#define BD3_F1C				1575420000
#define BD3_F2A				1176450000
/*----------------------------------------------------------------------------*/
#define GAL_F1              1575420000        //E1
#define GAL_F2              1207140000        //EB5
#define GAL_F3              1176450000        //E5A
#define GAL_E6              1278750000        //E6

// dInt采用64位整数转换，比iInt精度更高！
inline double dInt(double d) 
{	
	long long i64 = d > 0 ? (long long)(d + .5) : (long long)(d - .5); 
	return (double)i64;
}

inline long long dInt2(double d) 
{	
	long long i64 = d > 0 ? (long long)(d*1000 + .5) : (long long)(d*1000 - .5); 
	return i64;
}

inline int iInt(const double d)
{ 
	return d > 0 ? (long)(d + .5) : (long)(d - .5); 
}

#define RE_WGS84    6378137.0           /* earth semimajor axis (WGS84) (m) */
#define FE_WGS84    (1.0/298.257223563) /* earth flattening (WGS84) */
//#define MAXFIELD   64           /* max number of fields in a record */
#define MAXFIELD   16           /* max number of fields in a record */
#define D2R         (PI/180.0)          /* deg to rad */
#define R2D         (180.0/PI)          /* rad to deg */

#ifdef __cplusplus
}
#endif
