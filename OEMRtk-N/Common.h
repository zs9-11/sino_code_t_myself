#pragma once

//#ifndef COMMON_H
//#define COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
	/* Windows - set up dll import/export decorators. */
# if defined(BUILDING_RTK_SHARED)
	/* Building shared library. */
#   define RTK_EXTERN __declspec(dllexport)
# elif defined(USING_RTK_SHARED)
	/* Using shared library. */
#   define RTK_EXTERN __declspec(dllimport)
# else
	/* Building static library. */
#   define RTK_EXTERN /* nothing */
# endif
#elif __GNUC__ >= 4
# define RTK_EXTERN __attribute__((visibility("default")))
#else
# define RTK_EXTERN /* nothing */
#endif

#define RTK_SAT_ARRAY_SIZE  			64
//#define MAXSAT 		    				40

#ifdef  _EC100Y_PLATFORM
/**2020-12-25,meituan MAXSAT too big and then crash*/
//#define MAXSAT 		    				40
//#define MAXSAT 		    				30
#define MAXSAT 		    				25
/**2020-12-05,wangEpoch extend MAXSAT*/
#define MAXSAT_EXT	    				40
#else
#define MAXSAT 		    				40
#define MAXSAT_EXT	    				40
#endif


#define MAX_SYSTEM						5			// 最多5个系统(GPS, BD2, BD3, GAL, GLO)

#define NZ								1.0E-20
#define PI								3.14159265358979323846264338327950288419716939937510

//#define MAXPRN							200			// 卫星编号(GPS:1-37,GLONASS:38-61,SBAS:120-138, BD2:141-178)
#define MAXPRN							204			// 卫星编号(GPS:1-37,GLONASS:38-61,SBAS:120-138, BD2:141-202);[019-11-14]200=>204
#define MAX_SYS_PRN						38			// 单个系统内的最大卫星编号数（BDS：35）
#define MAX_POINT_NAME					256			// 点名改为256字节，2012-07-29
#define GPS_PRN_NUM						37
#define MIN_GPS_PRN						1
#define MAX_GPS_PRN						(MIN_GPS_PRN + GPS_PRN_NUM - 1)
//#define BD2_PRN_NUM						38
#define BD2_PRN_NUM						63        //[2019-11-14]38=>63
//#define BD2_PRN_NUM						18        //[2019-11-14]38=>63
#define MIN_BD2_PRN						141
#define	MAX_BD2_PRN						(MIN_BD2_PRN + BD2_PRN_NUM - 1)
#define BD3_PRN_NUM						45			// [2019-11-14] 45(159-203)
#define MIN_BD3_PRN						159
#define	MAX_BD3_PRN						(MIN_BD3_PRN + BD3_PRN_NUM - 1)
#define GLO_PRN_NUM						24
#define MIN_GLO_PRN						38
#define MAX_GLO_PRN						(MIN_GLO_PRN + GLO_PRN_NUM - 1)
//#define GAL_PRN_NUM						35
#define GAL_PRN_NUM						36
#define MIN_GAL_PRN						71
#define MAX_GAL_PRN						(MIN_GAL_PRN + GAL_PRN_NUM - 1)

#define LEAP_SECONDS					18			// 闰秒，陆小路

#define SOURCE_TYPE_ROVER               0   //信号来源，移动站
#define SOURCE_TYPE_BASE                1

#define   PARSE_SUCCESS                0     //数据解析成功
#define   PARSE_ERR_MISMATCH_TYPE      1    //传入的数据包类型不正确
#define   PARSE_ERR_DATAFRAME_BROKEN   2    //数据包不完整
#define   PARSE_ERR_INVALID_SATID      3    //卫星ID无效
#define   PARSE_ERR_INVALID_EPHEM      4    //无效星历
#define   PARSE_ERR_INVALID_EPHOCH     5    //无效	1074:GPS MSM4，1084:GLONASS MSM4，1124:BeiDou MSM4 包
#define   PARSE_FAILURE                6    //解析数据包失败
#define	  PARSE_EPOCH_SYNC_WAIT	       7    //继续等待RTCM 1074,1084,1124电文

#define   QRTK_SINGLE_POSITION_FAIL    8    //单点定位失败
#define   QRTK_SOLVE_POSITION_FAIL     9    //RTK解算失败

//#define   QRTK_RTCM_BUFFER_MAX_SIZE    10240 
//#define   QRTK_RTCM_BUFFER_MAX_SIZE    8192 
//#define   QRTK_RTCM_BUFFER_MAX_SIZE    4096
#ifdef  _EC100Y_PLATFORM
#define   QRTK_RTCM_BUFFER_MAX_SIZE    2048
#else
#define   QRTK_RTCM_BUFFER_MAX_SIZE    4096 
#endif

//support sat system
#define RTK_GNSS_CONSTELLATION_UNKNOWN  0x0000
/** GPS Constellation */
#define RTK_GNSS_CONSTELLATION_GPS 0x0001
/** Galileo Constellation */
#define RTK_GNSS_CONSTELLATION_GALILEO 0x0002
/** GLONASS Constellation */
#define RTK_GNSS_CONSTELLATION_GLONASS 0x0004 //qcom,meituan not support
/** BEIDOU Constellation */
#define RTK_GNSS_CONSTELLATION_BEIDOU 0x0008

#define RTK_GNSS_CONSTELLATION_ALL 0x000F

//rtk solution format
#define RTK_GNSS_SOLUTION_SINO 0x0001
#define RTK_GNSS_SOLUTION_NMEA 0x0002

//typedef int 		   BOOL;
typedef unsigned long      DWORD;
typedef unsigned char  	   BYTE;
typedef unsigned short 	   WORD;
typedef void*		   HANDLE;
//typedef void*		   RTK_HANDLE;
//typedef unsigned long	RTK_HANDLE;

#ifdef  _EC100Y_PLATFORM
typedef void*		   RTK_HANDLE;
#else
typedef unsigned long	RTK_HANDLE;
#endif


typedef unsigned int       UINT32, *PUINT32;
typedef unsigned short     UINT16, *PUINT16;
typedef long long          INT64, *PINT64;
typedef int                INT;

typedef struct tag_WangGloEphemeris
{
	unsigned long dwSize;
	unsigned short uMsgID;
	BYTE bSlot;
	BYTE bValid;
	short wIdleTime;
	short health;
	short week;
	short neweph;
	short newflag;
	short unused;
	short rfchnl;
	short uLeapSecond;
	BYTE hh;
	BYTE mm;
	BYTE ss;
	double tog;
	unsigned int tglo;
	int tb;
	BYTE M;
	BYTE Bn;
	short P;
	short NT;
	BYTE FT;
	BYTE En;
	BYTE P1;
	BYTE P2;
	BYTE P3;
	BYTE P4;
	BYTE ln;
	BYTE N4;
	double gamman;
	double taun;
	double deltataun;
	double xpos;
	double ypos;
	double zpos;
	double xvel;
	double yvel;
	double zvel;
	double xacc;
	double yacc;
	double zacc;
} 	ServerGloEphemeris;
const int GLO_EPHEMERIS_SIZE = sizeof(ServerGloEphemeris);

typedef struct tag_ServerEphemeris
{
	unsigned short wSize;
	unsigned char blFlag;
	unsigned char bHealth;
	unsigned char ID;
	unsigned char bIonoValid;
	unsigned short uMsgID;
	short m_wIdleTime;
	short iodc;
	short accuracy;
	unsigned short week;
	int iode;
	int tow;
	double toe;
	double toc;
	double af2;
	double af1;
	double af0;
	double Ms0;
	double deltan;
	double es;
	double roota;
	double omega0;
	double i0;
	double ws;
	double omegaot;
	double itoet;
	double Cuc;
	double Cus;
	double Crc;
	double Crs;
	double Cic;
	double Cis;
	double tgd;
	double tgd2;
	double alpha[4];
	double beita[4];
} 	ServerEphemeris;
const int EPHEMERIS_SIZE = sizeof(ServerEphemeris);

typedef struct tag_WangEpoch
{
	WORD wVersion;
	WORD wFreq;
	BYTE bGPRS;
	BYTE bBluetooth;
	BYTE bBattery;
	BYTE bGPSStatus;
	BYTE bCompassStatus;
	BYTE bGlonassStatus;
	BYTE bGalileoStatus;
	WORD wMsgID;
	DWORD dwCompassCheck;
	double dGPSOffset;
	double dCompassOffset;
	double dGlonassOffset;
	double dGalileoOffset;
	double dRovOffset;
	DWORD dwStation;
	int m_iType;
	int m_nIndex;
	int iPort;
	int n;
	int wWeek;
	double t;
	double dTimeOffset;
	double dVTEC;
	float fPdop;
	double x;
	double y;
	double z;
	double xBase;
	double yBase;
	double zBase;
	double rms;
	double ratio;
	int iSolveType;
	BYTE pbMode1[MAXSAT];
	BYTE pbMode2[MAXSAT];
	BYTE pbMode5[MAXSAT];
	BYTE pbID[MAXSAT];
	double pdR1[MAXSAT];
	double pdP2[MAXSAT];
	double pdR5[MAXSAT];
	double pdL1[MAXSAT];
	double pdL2[MAXSAT];
	double pdL5[MAXSAT];
	double pdD1[MAXSAT];
	double pdD2[MAXSAT];
	double pdD5[MAXSAT];
	BYTE pbSN1[MAXSAT];
	BYTE pbSN2[MAXSAT];
	BYTE pbSN5[MAXSAT];
	DWORD pdwSlip1[MAXSAT];
	DWORD pdwSlip2[MAXSAT];
	DWORD pdwSlip5[MAXSAT];
	short pwAzimuth[MAXSAT];
	unsigned char pbElevation[MAXSAT];
#ifdef BD3_TEST
	double pdR1_BD3[MAXSAT];
	double pdP2_BD3[MAXSAT];
	double pdL1_BD3[MAXSAT];
	double pdL2_BD3[MAXSAT];
	BYTE pbSN1_BD3[MAXSAT];
	BYTE pbSN2_BD3[MAXSAT];
	DWORD pdwSlip1_BD3[MAXSAT];
	DWORD pdwSlip2_BD3[MAXSAT];
#endif
} 	WangEpoch;
const int EPOCH_SIZE = sizeof(WangEpoch);

typedef struct tag_MixEpoch
{
    WORD    wSize;
    WORD    wMsgID;
    WORD    wReceiverStatus;
    WORD    wFreq;
    WORD    wBaseID;
    WORD    wRoverID;
    WORD    wWeek;
    BYTE    bVersion;
    BYTE    bImuValid;
    int     lTimeMS;
    int     lRovOffset;
    short   nObs;
    short   iEventOffset;
    short   iGPSLag;
    short   iBDSLag;
    short   iGloLag;
    short   iGalLag;
    short   iBaseGpsOffset;
    short   iBaseBdsOffset;
    short   iBaseGloOffset;
    short   iBaseGalOffset;
    
    double  dBaseX, dBaseY, dBaseZ;
    int  lBaseLat, lBaseLon, lBaseHcm;
    int  lRoverX, lRoverY, lRoverZ;
    int  lVelX, lVelY, lVelZ;

    double  dImuX, dImuY, dImuZ;
    double  dImuVelX, dImuVelY, dImuVelZ;
    double  dImuAccX, dImuAccY, dImuAccZ;
    double  dImuHeader, dImuPitch, dImuRoll;
    double  dImuNonFixTime;
    double  dImuLastPosError;
    double  dImuLastVelError;
    
    BYTE    pbID[MAXSAT];
    BYTE    pbFlag[MAXSAT];
    char    pbElevation[MAXSAT];
    short   piAzimuth[MAXSAT];
    int     plClock[MAXSAT];
    short   piIonoDelay[MAXSAT];
    short   piTropDelay[MAXSAT];
    
    BYTE    pbBaseM1[MAXSAT];
    BYTE    pbBaseM2[MAXSAT];
    BYTE    pbBaseM5[MAXSAT];
    BYTE    pbBaseCN1[MAXSAT];
    BYTE    pbBaseCN2[MAXSAT];
    BYTE    pbBaseCN5[MAXSAT];
    BYTE    pbBaseS1[MAXSAT];
    BYTE    pbBaseS2[MAXSAT];
    BYTE    pbBaseS5[MAXSAT];
    BYTE    pbSoc[MAXSAT];
    BYTE    pbSlc[MAXSAT];
    int     plBaseR1[MAXSAT];           //单位0.1米
    short   piBaseR21[MAXSAT];          //R2-R1(0.1米,绝对值小于3000米)
    short   piBaseR51[MAXSAT];          //R2-R1(0.1米,绝对值小于3000米)
    int     plBaseL1[MAXSAT];
    int     plBaseL2[MAXSAT];
    int     plBaseL5[MAXSAT];            //L5或B3的载波相位观测

    BYTE    pbRoverM1[MAXSAT];
    BYTE    pbRoverM2[MAXSAT];
    BYTE    pbRoverM5[MAXSAT];
    BYTE    pbRoverCN1[MAXSAT];
    BYTE    pbRoverCN2[MAXSAT];
    BYTE    pbRoverCN5[MAXSAT];
    BYTE    pbRoverS1[MAXSAT];
    BYTE    pbRoverS2[MAXSAT];
    BYTE    pbRoverS5[MAXSAT];
    int     plRoverR1[MAXSAT];
    short   piRoverR21[MAXSAT];
    short   piRoverR51[MAXSAT];
    int     plRoverL1[MAXSAT];
    int     plRoverL2[MAXSAT];
    int     plRoverL5[MAXSAT];          //L5或B3的载波相位观测

    BYTE    pbSlaveM1[MAXSAT];
    BYTE    pbSlaveM2[MAXSAT];
    BYTE    pbSlaveM5[MAXSAT];
    BYTE    pbSlaveCN1[MAXSAT];
    BYTE    pbSlaveCN2[MAXSAT];
    BYTE    pbSlaveCN5[MAXSAT];
    BYTE    pbSlaveS1[MAXSAT];
    BYTE    pbSlaveS2[MAXSAT];
    BYTE    pbSlaveS5[MAXSAT];
    int     plSlaveR1[MAXSAT];
    short   piSlaveR21[MAXSAT];
    short   piSlaveR51[MAXSAT];
    int     plSlaveL1[MAXSAT];
    int     plSlaveL2[MAXSAT];
    int     plSlaveL5[MAXSAT];          //L5或B3的载波相位观测

}MixEpoch,  *pMixEpoch;
const int MIX_EPOCH_SIZE = sizeof(MixEpoch);

typedef struct __RTK_RESULT
{
	BYTE bStatusFlag;// 解类型？和原有定义一样？
	BYTE bSatNo;
	short wRefID;

	BYTE 	bTracking;
	BYTE 	bReserved1;
	BYTE 	bReserved2;
	BYTE 	bReserved3;

	double dRTK_Resolution_Time;
	double dRTK_Diff_Time;

	double dRov_Pos_x;
	double dRov_Pos_y;
	double dRov_Pos_z;
	double dHDOP;

	float	fRangeRms;
	float	fPhaseRms;
	float	fSigmaH;	// 平面精度
	float	fSigmaV;
	float 	fPDOP;
	float	fQpx;		// 位置和速度在XYZ上的精度
	float	fQpy;
	float	fQpz;
	float 	fQpxy;
	float	fQpxz;
	float	fQpyz;

	float	fQvx;
	float	fQvy;
	float	fQvz;
	float	fQvxy;
	float	fQvxz;
	float	fQvyz;

	BYTE pbUsedPrn[RTK_SAT_ARRAY_SIZE];
	BYTE pbUsedL1[RTK_SAT_ARRAY_SIZE];// 什么意思？
	BYTE pbUsedL2[RTK_SAT_ARRAY_SIZE];
	BYTE pbUsedL3[RTK_SAT_ARRAY_SIZE];

	double dRov_Vel_x;
	double dRov_Vel_y;
	double dRov_Vel_z;

	int iDeltaT_GPS;//差分授时时间差；单位0.1ns
	int iDeltaT_GLO;
	int iDeltaT_BD2;
	int iDeltaT_GAL;

	BYTE bDeltaT_GPS_Valid;
	BYTE bDeltaT_GLO_Valid;
	BYTE bDeltaT_BD2_Valid;
	BYTE bDeltaT_GAL_Valid;
	double dClockDrift;
    int nFixedSlip;
    BYTE pbFixedID[RTK_SAT_ARRAY_SIZE];
    BYTE pbFixedType[RTK_SAT_ARRAY_SIZE];
    int piFixedSlip[RTK_SAT_ARRAY_SIZE];

}RTK_result;

typedef struct _Coordinate
{
    /**useless*/
    unsigned int staid;
	unsigned int IRTF;
    unsigned int GPS_ind;
    unsigned int GLO_ind;
    unsigned int Gli_ind;
    unsigned int sta_ind;
    unsigned int Sro_ind;
    unsigned int QC_ind;
    /***/
    double       ARP[3];
} Coordinate;
const int COORD_SIZE = sizeof(Coordinate);

typedef struct tag_XYZ
{
	double x;
	double y;
	double z;
} XYZ;

#ifdef __cplusplus
}
#endif
