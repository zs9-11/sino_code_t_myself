// RtkType.h: interface for the CCrypTography class.
// [2012-08-03] ����L1/L2/B1/B3 RTK������K501B
// [2012-10-23] RTK��Ԫ���Ӹ�������ģʽ�͹۲�ֵ����ָ��
// [2017-11-16] tag_RtkEpoch����
// [2020-10-27] �޸�tag_RtkEpoch������α���RMS��ȥ�������գ���wIonoDrift�޸�Ϊshort����
// ע�⣬����ĸ�ʽӦ����RtkType.h����ͬ��
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTKTYPE_H__INCLUDED_)
#define AFX_RTKTYPE_H__INCLUDED_

/*
#include "gnss_macro.h"

#define POLARIS_HEADER		// ������QRtkInterface.h����

//typedef unsigned long       DWORD;
typedef unsigned char		BYTE;
typedef unsigned short    WORD;
typedef unsigned long		RTK_HANDLE;
*/

// ע����Ҫ����ջ��̼���ʽͬ��(�������׸Ķ���)
typedef struct tag_RtkEphemeris
{
	unsigned short	wSize;		// ���ṹ�Ĵ�С
	unsigned char	blFlag;		// ��Ч��
	unsigned char	bHealth;	// �����Ľ���״��
	unsigned char	ID;			// ����ID
	unsigned char	bSateType;	// ������Ϊ��32λ����
	unsigned short	wMsgID;		// ��Ϣ��ID
	short			m_wIdleTime;	// ����ʱ�䣬�ڴ������
	short			iodc;			
	short			accuracy;	// ����
	unsigned short	week;		// ������
	int				iode;		// Issue of data, ephemeris[];
	int				tow;		// ����ʱ��__int32
	double			toe;
	double			toc;
	double			af2;
	double			af1;
	double			af0;
	double			Ms0;		// Mean Anomaly at reference time.
	double			DeltaN;		// Mean motion difference from computed value.
	double			es;
	double			RootA;		// Eccentricity and square root.
	double			omega0;		// Longitude of ascending node of orbit plane at weekly epoch.
	double			i0;			// Inclination angle at ref. times.
	double			ws;			// Argument of perigee.
	double			OmegaDot;	// Rate of right ascension.
	double			itoet;		// Rate of inclination angle.
	double			Cuc;		// Amplitude of the cosine harmonic correction term to the augument of latitude.
	double			Cus;		// Amplitude of the sine harmonic correction term to the augument of latitude
	double			Crc;		// Amplitude of the cosine harmonic correction term to the orbit radius.
	double			Crs;		// Amplitude of the sine harmonic correction term to the orbit radius.
	double			Cic;		// Amplitude of the cosine harmonic correction term to the angle of inclination.
	double			Cis;		// Amplitude of the sine harmonic correction term to the angle of inclination.
//	double			fit_interval;	// ɾ��fit_interval, 2001A-04-08
	double			tgd;
	double			tgd2;			// 2010-04-02: ����
}RtkEphemeris;


// ��GNSServer��Ӧ�ã�ע����Ҫ����ջ��̼���ʽͬ��
typedef struct tag_RtkGloEphemeris
{
  	UINT32 	iPrn;
	WORD	wSloto;	// iPrn
	WORD	wFreqo;
	BYTE	bSatType;
	BYTE	bRes1;
	WORD	wEphWeek;
	UINT32	iEphTime;	//tog
	UINT32	iTimeOffset;
	WORD	wDateNum;
	BYTE	bRes2;
	BYTE	bRes3;
	UINT32	iIssue;
	UINT32	iHealth;
	double	dPosX;
	double	dPosY;
	double	dPosZ;
	double	dVelX;
	double	dVelY;
	double	dVelZ;
	double	dAccX;
	double	dAccY;
	double	dAccZ;
	double	dTauN;
	double	dDeltaTauN;
	double	dGamma;
	UINT32	iTk;
	UINT32	iP;
	UINT32	iFt;
	UINT32	iAge;
	UINT32	iFlag;	
	UINT32	iValid;
}RtkGloEphemeris;


// 	iSolveMode: 0..None; 
//				1..Single; 
//				2..RTD; 
//				3..Float; 
//				4..Fix; 
//				5..Wide Fix; 
//				6..Iono-free fix.
//				7..Fix Checking
typedef struct tag_RtkPosition
{
	unsigned char	bStatus;
	int		iType;	// 0. ��С����; 1.Kalman;.2. Least+Kalman 3. Least+Kalman(Solved)
	int		iSolveMode;	// ���Ϸ���˵��
	double	dRatio;
	float	fAdop;		// ģ���Ⱦ�������
	int		iFixSearch;	// [2020-08-19]��m_iFixHole�޸�Ϊm_iFixSearch����ʾģ���ȿռ��������Ĵ���
	int		iFixPrecent;
	float	fPdop;
	float	fQxyh[6];	// ƽ��Qxx, Qxy, Qyy, Qxh, Qyh, Qhh
	float 	fQpxyz[6];	// λ��XYZ�ľ���Qxx,Qyy,Qzz,Qxy,Qxz,Qzz
	float	fQvxyz[6];	// �ٶ�XYZ�ľ���Qxx,Qyy,Qzz,Qxy,Qxz,Qzz
	double	t;
	unsigned char nUsing;	// ʹ�õ�������
	unsigned char nFix;	// �̶���ʹ�õĹ۲�����
	double	dPhaseRms;	// Added at 2006-3-31
	double	dRangeRms;	// Added at 2015-11-22
	double	x, y, z;		// ����Ϊxyz����
	double	b, l, h;
	unsigned char	bObsL1;
	unsigned char	bObsL2;
	unsigned char	bObsL5;
	unsigned char	pbSvnL1[MAXSAT];	//	ʹ��L1������
	unsigned char	pbSvnL2[MAXSAT];	//	ʹ��L2������
	unsigned char	pbSvnL5[MAXSAT];	//	ʹ��L5������
	short	nObs;						//  �۲�������
	BYTE	pbObsID[MAXSAT];
	BYTE	pbElevation[MAXSAT];
	WORD	pwAzimuth[MAXSAT];
}RtkPosition;

typedef struct tag_RtkResult
{
	bool	bUpdate;	// �Ƿ�����ˡ�
	bool	bVector;	// m_Baseline���������m_Vector���
	int		iSolveMode;	// ����ģʽ
	int		nEpoch;
	BYTE	nUsing;		// ʹ�õ�������
	BYTE	nTracking;	// ���ٵ�������
	BYTE	nFix;		// �̶���ʹ�õ�������
	BYTE	nDeleted;	// ɾ����������
	BYTE	bDiffClockValidForGPS;
	BYTE	bDiffClockValidForBD2;
	BYTE	bDiffClockValidForBD3;
	BYTE	bDiffClockValidForGLO;
	BYTE	bDiffClockValidForGAL;
	char	cReviewCheck;		// ����־��2020-10-20����
	char	cSolveCheck;
	int		iDiffClockForGPS;
	int		iDiffClockForBD2;
	int		iDiffClockForBD3;
	int		iDiffClockForGAL;
	int		iDiffClockForGLO;
	float	fRangeRms;
	float	fPhaseRms;
	float	fPDOP;
	float	fFloatRdop;	// 2018-03-27
	float	fFixRdop;	// 2018-03-27
	float	fRatio;		// 2018-03-27
	float	fFirst;		// 2018-03-27
	float	fFixSat;	// 2018-03-27
	double	t;
	double	xb;			// �ο�վ����
	double	yb;
	double	zb;	
	double	x;			// ����վ������
	double	y;
	double	z;
	double	vx;
	double	vy;
	double	vz;
	double	ax, ay, az;	// ���ٶ�
	float	fSigmaV;	// �߳̾���
	float	fSigmaH;	// ƽ�澫��
	float	pfQpxyz[6];	// λ�õ�Э����
	float	pfQvxyz[6];	// �ٶȵ�Э����
	BYTE	pbID[MAXSAT];
	BYTE	pbL1[MAXSAT];	// ����ʹ��״̬
	BYTE	pbL2[MAXSAT];
	BYTE	pbL5[MAXSAT];
	BYTE	pbDeleted[MAXSAT];	// ��ɾ��������
	double	dClockDrift;		// �ؼ�ֵ����ʾǰ����Ԫ�Ӳ�仯���ף�
	double	dFilterDeep;		// �˲����
	int		nFixedSlip;
	BYTE	pbFixedID[MAXSAT];
	BYTE	pbFixedType[MAXSAT];
	int		piFixedSlip[MAXSAT];	// Slip = Obs - Model;
	int		nDiffSat;
	BYTE	pbDiffID[MAXSAT];		// ��¼ʱ������
	BYTE	pbSlip[MAXSAT];
	int		piDiff[MAXSAT];			// ��¼ʱ��ֵ(0.1mm)
}RtkResult;

typedef struct tag_RtkWeather
{
	double		m_dAlpha[4];
	double		m_dBeta[4];
	double		m_dTS, m_dPS, m_dHS;	// ������������¶ȡ���ѹ��ʪ��
	int			m_iTrop;
	int			m_iIono;
}RtkWeather;

typedef struct tag_RtkEpoch
{
	BYTE	bFreq;			// ���ڱ�ʶƵ�ʣ� [G2 G1 B3 B2 B1 L5 L2 L1]
	BYTE	bFreq2;			
	BYTE	bExternalFlag;	// �����ʾ(SBAS_DIFF|BASE_FAIL|HEADING|EVENT)����Ϣ
	BYTE	bDataLink;
	BYTE	bBluetooth;
	BYTE	bPowerStatus;	// ��Դ״̬
	BYTE	bGPSStatus;		// ���ջ�״̬
	BYTE	bCompassStatus;	// BD2�Ĺ���״̬
	BYTE	bGloStatus;
	BYTE	bGalStatus;
	BYTE	bImuValid;		// IMU Valid
	WORD	wMsgID;			// ��Ϣ��
	WORD	wRefID;			// �ο�վID��
	WORD	wCompassCheck;	// ��Ȩ��Ϣ
	double	dGPSOffset;		// GPS�ο�վ��ʱ��ƫ��
	double	dCompassOffset;	// BD2�ο�վ��ʱ��ƫ��
	double	dGloOffset;
	double	dGalOffset;
	double	dRovOffset;		// ����վʱ��ƫ�ƣ����ڵ���۲�ֵ���ԣ���ֵΪ0.
	double	pdSysClock[4];	// ��ϵͳ���Ӳ�
	double 	dGlonassSystemOffset;	// GLONASS��GPSϵͳʱ��
	int		iStationID;		// ��Ӧ�Ĳ�վ����ջ���
	int		iType;			// Receiver Type.
	int		iLag;			// Diff Delay.
	int		n;				// Obs Number.
	int		wWeek;
	double	t;
	double	dTimeOffset;	// �ǵ����ʱ��ƫ�ƣ����㶨λ�����
	double	dVTEC;			// ��ֱ����ĵ�������
	float	fPdop;
	double	vx;				// PVT���ٶȷ���
	double	vy;
	double	vz;
	double	xBase;			// �ο�վ�Ĺ̶����꣬����������վ
	double	yBase;
	double	zBase;
	int		iRoverX, iRoverY, iRoverZ;
	double	xVector;			// ��������
	double	yVector;
	double	zVector;
	double	dBaseOrgB;				// �����ο�վ�ĳ�ʼ��γ��
	double	dBaseOrgL;
	double	dBaseOrgH;
	double	dRoverOrgB;			// ��������վ�ĳ�ʼ��γ��
	double	dRoverOrgL;
	double	dRoverOrgH;
	// ---------------------- IMU Data ------------------------
	double	dImuX;				
	double	dImuY;
	double	dImuZ;
	double	dImuVelX;
	double	dImuVelY;
	double	dImuVelZ;
	double	dImuAccX;
	double	dImuAccY;
	double	dImuAccZ;
	double	dImuHeader;
	double	dImuPitch;
	double	dImuRoll;
	double	dImuNonFixTime;	// ��һ��GNSS���µ����ڵ�ʱ�䣻��
	double	dImuLastPosError;
	double	dImuLastVelError;
	// ---------------------------------------------------------
	int		iSolveType;
	BYTE	pbID[MAXSAT];
	BYTE	pbDeleted[MAXSAT];
	BYTE	pbModeL1[MAXSAT];
	BYTE	pbModeL2[MAXSAT];
	BYTE	pbModeL5[MAXSAT];
	double	pdR1[MAXSAT];
	double	pdP2[MAXSAT];
	double	pdR5[MAXSAT];		// L5��B3��α�൥��
#ifdef _DEBUG
	double	pdClock[MAXSAT];	// �����Ӳ���ٸ���
#endif
	double	pdL1[MAXSAT];
	double	pdL2[MAXSAT];
	double	pdL5[MAXSAT];		// L5��B3���ز���λ����۲�ֵ
	BYTE	pbSN1[MAXSAT];
	BYTE	pbSN2[MAXSAT];
	BYTE	pbSN5[MAXSAT];	
	BYTE	pbRmsL1[MAXSAT];	// α����ز��ľ��ȣ�2012-10-23��
	BYTE	pbRmsL2[MAXSAT];
	BYTE	pbRmsL5[MAXSAT];
	BYTE	pbSoc[MAXSAT];		// non-dispersive interpolation residuals(mm)
	WORD	pwSlc[MAXSAT];		// dispersive interpolation residuals
	DWORD	pdwSlip1[MAXSAT];
	DWORD	pdwSlip2[MAXSAT];
	DWORD	pdwSlip5[MAXSAT];
	WORD	pwAzimuth[MAXSAT];
	char	pbElevation[MAXSAT];
	double	pdIonoDelay[MAXSAT];
	WORD	pwIonoDrift[MAXSAT];
	short	piSatDX[MAXSAT];
	short	piSatDY[MAXSAT];
	short	piSatDZ[MAXSAT];
}RtkEpoch;

/*
// [2019-10-25] ����˻�վ�������ߣ����������еĹ۲����ݣ���СԼ4KB
typedef struct tag_MixEpoch
{
	WORD	wSize;						// ���ڱ�ʾ����ṹ�Ĵ�С
	WORD	wMsgID;						// ��Ϣ��
	WORD	wReceiverStatus;			// ���ջ�״̬(���棺bExternalFlag..���ڱ�ʾEVENT����Ϣ��)
	WORD	wFreq;						// ���ڱ�ʾ���õ�Ƶ�ʣ�BDS��GALϵͳ�ṩ���źŲ�ֹһ��Ƶ�ʣ����ṹ��ʹ����3��Ƶ�ʣ���Ҫ��ʶ
	WORD	wBaseID;					// �ο�վID��
	WORD	wRoverID;					// ���ջ�ID��
	WORD	wWeek;						// GPS��
	BYTE	bVersion;					// �汾��
	BYTE	bImuValid;					// IMU Valid(����IMU�⣬��������������Ϣ)
	int		lTimeMS;					// ��(ms)
	int		lRovOffset;					// ����վ�Ӳ��λΪ(mm)
	short	nObs;						// Obs Number(����λ32λ�������������պ�ʹ�ã����40)
	short	iEventOffset;				// Eventʱ�������t��ʱ��ƫ�
	short	iGPSLag;					// GPS�ο�վ��ʱ��ƫ�ƣ������t�������ƫ�ƣ���λ0.01�룩
	short	iBDSLag;					// BD2�ο�վ��ʱ��ƫ�ƣ�0.01�룩
	short	iGloLag;
	short	iGalLag;					// GAL��ʱ��ƫ�ƣ�0.01�룩	
	short	iBaseGpsOffset;				// �ο�վ�۲������Ӳ(0.1us)
	short	iBaseBdsOffset;
	short	iBaseGloOffset;
	short	iBaseGalOffset;	
	// ---------------------- ���� ---------------------------------------------------------------------------------------------
	double	dBaseX, dBaseY, dBaseZ;		// �ο�վ�Ĺ̶����꣬����������վ
	int		lBaseLat, lBaseLon, lBaseHcm;	// ��վ�ĳ�ʼ��γ�Ⱥ͸̣߳����ڼ���߶Ƚǵȣ��Ծ�����Ҫ�󣨵�λ��0.000000001���Ⱥ����ף�;	
	int		lRoverX, lRoverY, lRoverZ;	// �ƶ�վ����ڲο�վ�����꣬��λ(mm)���ƶ�վ��Ի�վ���ܳ���20000km
	int		lVelX, lVelY, lVelZ;		// PVT���ٶȷ���(mm/s)
	// ---------------------- IMU Data ------------------------------------------------------------------------------------------
	double	dImuX, dImuY, dImuZ;
	double	dImuVelX, dImuVelY, dImuVelZ;
	double	dImuAccX, dImuAccY, dImuAccZ;
	double	dImuHeader, dImuPitch, dImuRoll;
	double	dImuNonFixTime;						// ��һ��GNSS���µ����ڵ�ʱ�䣻��
	double	dImuLastPosError;
	double	dImuLastVelError;
	// ===========================================================================================================================
	BYTE	pbID[MAXSAT];
	BYTE	pbFlag[MAXSAT];		// ������ʾBase\Rover\Slave����������Ч�ԣ�����α���ָ���ģʽ�������������ļ���
	char	pbElevation[MAXSAT];
	short	piAzimuth[MAXSAT];
	int		plClock[MAXSAT];	// �����Ӳ���ٸ���(mm), �����ݹ�����RTDģʽ��ʱ����ֵ������ʾα�������
	short	piIonoDelay[MAXSAT];// ģ�ͻ��Ĵ����������ƶ�վ������λcm������PDP���㶨λʱ����ʹ��
	short	piTropDelay[MAXSAT];// ������Ĵ�����������λcm��	
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbBaseM1[MAXSAT];	// �۲�����ָʾ,���齫pbModeL1��pbRmsL1�ϲ�Ϊһ���ֽ�(����λ��L2C,L2P,������ȣ�����λ���ز�RMS��
	BYTE	pbBaseM2[MAXSAT];
	BYTE	pbBaseM5[MAXSAT];
	BYTE	pbBaseCN1[MAXSAT];
	BYTE	pbBaseCN2[MAXSAT];
	BYTE	pbBaseCN5[MAXSAT];
	BYTE	pbBaseS1[MAXSAT];	// ����������
	BYTE	pbBaseS2[MAXSAT];
	BYTE	pbBaseS5[MAXSAT];
	BYTE	pbSoc[MAXSAT];		// non-dispersive interpolation residuals(mm)
	BYTE	pbSlc[MAXSAT];		// dispersive interpolation residuals(����ΪWORD�������ΪBYTE)
	int		plBaseR1[MAXSAT];	// ��λ��0.1��
	short	piBaseR21[MAXSAT];	// R2-R1(0.1�ף�����ֵС��3000��)
	short	piBaseR51[MAXSAT];	// R2-R1(0.1�ף�����ֵС��3000��)
	int		plBaseL1[MAXSAT];	// ��λ1/256��
	int		plBaseL2[MAXSAT];
	int		plBaseL5[MAXSAT];	// L5��B3���ز���λ����۲�ֵ
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbRoverM1[MAXSAT];	// �۲�����ָʾ,���齫pbModeL1��pbRmsL1�ϲ�Ϊһ���ֽ�
	BYTE	pbRoverM2[MAXSAT];
	BYTE	pbRoverM5[MAXSAT];
	BYTE	pbRoverCN1[MAXSAT];
	BYTE	pbRoverCN2[MAXSAT];
	BYTE	pbRoverCN5[MAXSAT];
	BYTE	pbRoverS1[MAXSAT];
	BYTE	pbRoverS2[MAXSAT];
	BYTE	pbRoverS5[MAXSAT];
	char	pcRoverDelta1[MAXSAT];
	char	pcRoverDelta2[MAXSAT];
	char	pcRoverDelta5[MAXSAT];
	char	pcRoverTheta1[MAXSAT];
	char	pcRoverTheta2[MAXSAT];
	char	pcRoverTheta5[MAXSAT];
	int		plRoverR1[MAXSAT];	// ��λ��0.1��
	short	piRoverR21[MAXSAT];	// R2-R1(0.1�ף�����ֵС��3000��)
	short	piRoverR51[MAXSAT];	// R5-R1(0.1�ף�����ֵС��3000��)
	int		plRoverL1[MAXSAT];	// ��λ1/256��
	int		plRoverL2[MAXSAT];
	int		plRoverL5[MAXSAT];	// L5��B3���ز���λ�۲�ֵ
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbSlaveM1[MAXSAT];	// �۲�����ָʾ,���齫pbModeL1��pbRmsL1�ϲ�Ϊһ���ֽ�
	BYTE	pbSlaveM2[MAXSAT];
	BYTE	pbSlaveM5[MAXSAT];
	BYTE	pbSlaveCN1[MAXSAT];
	BYTE	pbSlaveCN2[MAXSAT];
	BYTE	pbSlaveCN5[MAXSAT];
	BYTE	pbSlaveS1[MAXSAT];
	BYTE	pbSlaveS2[MAXSAT];
	BYTE	pbSlaveS5[MAXSAT];
	char	pcSlaveDelta1[MAXSAT];
	char	pcSlaveDelta2[MAXSAT];
	char	pcSlaveDelta5[MAXSAT];
	char	pcSlaveTheta1[MAXSAT];
	char	pcSlaveTheta2[MAXSAT];
	char	pcSlaveTheta5[MAXSAT];
	int		plSlaveR1[MAXSAT];	// ��λ��0.1��
	short	piSlaveR21[MAXSAT];	// R2-R1��0.1�ף�����ֵС��3000�ף�
	short	piSlaveR51[MAXSAT];	// R5-R1��0.1�ף�����ֵС��3000�ף�
	int		plSlaveL1[MAXSAT];	// ��λ��1/256��
	int		plSlaveL2[MAXSAT];
	int		plSlaveL5[MAXSAT];	// L5��B3���ز���λ�۲�ֵ
}MixEpoch, * pMixEpoch;
*/

// [2013-06-02]���㶨λ���[V2]
typedef struct tag_RtkSinglePosition
{
	unsigned long	dwSize;
	BYTE			nUsing;		// ��������
	BYTE			nFix;
	WORD			nBad;		// �ֲ�����
	WORD			nStep;		// ��С���˼����������
	BYTE			bElevationCutOff;	// ���㶨λ����ʹ�õĸ߶Ƚ�ֹ�ǣ����룩
	WORD			wWeek;
	double			t;
	double			dRms;		// ������
	double			dPdop;		// ���ξ�������
	double			dQxx;
	double			dQxy;
	double			dQxz;
	double			dQyy;
	double			dQyz;
	double			dQzz;
	double			x;			// ���㶨λ���
	double			y;
	double			z;
	double			pdTimeOffset[MAX_SYSTEM];
	BYTE			pbClockIndex[MAX_SYSTEM];	// pdTimeOffset��Ӧ������ϵͳ��ţ�0-GPS; 1-BDS; 2-GLO; 3-GAL; FF-NONE
	BYTE			pbID[MAXSAT];				// ���㶨λ������
	BYTE			pbFlag[MAXSAT];				// ����ʹ��״̬
	short int		piRes[MAXSAT];				// ��λΪ0.01�׵Ĳв�(Max: 300; Min: -300); 
}RtkSinglePosition, *pRtkSinglePosition;

#endif // !defined(AFX_RTKTYPE_H__INCLUDED_)
