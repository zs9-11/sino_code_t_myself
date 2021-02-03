// RtkType.h: interface for the CCrypTography class.
// [2012-08-03] 增加L1/L2/B1/B3 RTK，用于K501B
// [2012-10-23] RTK历元增加高灵敏度模式和观测值精度指标
// [2017-11-16] tag_RtkEpoch更新
// [2020-10-27] 修改tag_RtkEpoch，增加伪距的RMS，去除多普勒，将wIonoDrift修改为short类型
// 注意，这里的格式应该与RtkType.h保持同步
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTKTYPE_H__INCLUDED_)
#define AFX_RTKTYPE_H__INCLUDED_

/*
#include "gnss_macro.h"

#define POLARIS_HEADER		// 用于与QRtkInterface.h适配

//typedef unsigned long       DWORD;
typedef unsigned char		BYTE;
typedef unsigned short    WORD;
typedef unsigned long		RTK_HANDLE;
*/

// 注意需要与接收机固件格式同步(不可轻易改动！)
typedef struct tag_RtkEphemeris
{
	unsigned short	wSize;		// 本结构的大小
	unsigned char	blFlag;		// 有效否？
	unsigned char	bHealth;	// 星历的健康状况
	unsigned char	ID;			// 卫星ID
	unsigned char	bSateType;	// 保留，为了32位对齐
	unsigned short	wMsgID;		// 消息的ID
	short			m_wIdleTime;	// 空闲时间，内存清空用
	short			iodc;			
	short			accuracy;	// 精度
	unsigned short	week;		// 星期数
	int				iode;		// Issue of data, ephemeris[];
	int				tow;		// 发送时间__int32
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
//	double			fit_interval;	// 删除fit_interval, 2001A-04-08
	double			tgd;
	double			tgd2;			// 2010-04-02: 新增
}RtkEphemeris;


// 在GNSServer中应用，注意需要与接收机固件格式同步
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
	int		iType;	// 0. 最小二乘; 1.Kalman;.2. Least+Kalman 3. Least+Kalman(Solved)
	int		iSolveMode;	// 见上方的说明
	double	dRatio;
	float	fAdop;		// 模糊度精度因子
	int		iFixSearch;	// [2020-08-19]将m_iFixHole修改为m_iFixSearch，表示模糊度空间中搜索的次数
	int		iFixPrecent;
	float	fPdop;
	float	fQxyh[6];	// 平面Qxx, Qxy, Qyy, Qxh, Qyh, Qhh
	float 	fQpxyz[6];	// 位置XYZ的精度Qxx,Qyy,Qzz,Qxy,Qxz,Qzz
	float	fQvxyz[6];	// 速度XYZ的精度Qxx,Qyy,Qzz,Qxy,Qxz,Qzz
	double	t;
	unsigned char nUsing;	// 使用的卫星数
	unsigned char nFix;	// 固定解使用的观测量数
	double	dPhaseRms;	// Added at 2006-3-31
	double	dRangeRms;	// Added at 2015-11-22
	double	x, y, z;		// 更新为xyz坐标
	double	b, l, h;
	unsigned char	bObsL1;
	unsigned char	bObsL2;
	unsigned char	bObsL5;
	unsigned char	pbSvnL1[MAXSAT];	//	使用L1的卫星
	unsigned char	pbSvnL2[MAXSAT];	//	使用L2的卫星
	unsigned char	pbSvnL5[MAXSAT];	//	使用L5的卫星
	short	nObs;						//  观测卫星数
	BYTE	pbObsID[MAXSAT];
	BYTE	pbElevation[MAXSAT];
	WORD	pwAzimuth[MAXSAT];
}RtkPosition;

typedef struct tag_RtkResult
{
	bool	bUpdate;	// 是否更新了。
	bool	bVector;	// m_Baseline结果，还是m_Vector结果
	int		iSolveMode;	// 解算模式
	int		nEpoch;
	BYTE	nUsing;		// 使用的卫星数
	BYTE	nTracking;	// 跟踪的卫星数
	BYTE	nFix;		// 固定解使用的卫星数
	BYTE	nDeleted;	// 删除的卫星数
	BYTE	bDiffClockValidForGPS;
	BYTE	bDiffClockValidForBD2;
	BYTE	bDiffClockValidForBD3;
	BYTE	bDiffClockValidForGLO;
	BYTE	bDiffClockValidForGAL;
	char	cReviewCheck;		// 检查标志，2020-10-20新增
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
	double	xb;			// 参考站坐标
	double	yb;
	double	zb;	
	double	x;			// 流动站解算结果
	double	y;
	double	z;
	double	vx;
	double	vy;
	double	vz;
	double	ax, ay, az;	// 加速度
	float	fSigmaV;	// 高程精度
	float	fSigmaH;	// 平面精度
	float	pfQpxyz[6];	// 位置的协方差
	float	pfQvxyz[6];	// 速度的协方差
	BYTE	pbID[MAXSAT];
	BYTE	pbL1[MAXSAT];	// 数据使用状态
	BYTE	pbL2[MAXSAT];
	BYTE	pbL5[MAXSAT];
	BYTE	pbDeleted[MAXSAT];	// 被删除的卫星
	double	dClockDrift;		// 关键值，表示前后历元钟差变化（米）
	double	dFilterDeep;		// 滤波深度
	int		nFixedSlip;
	BYTE	pbFixedID[MAXSAT];
	BYTE	pbFixedType[MAXSAT];
	int		piFixedSlip[MAXSAT];	// Slip = Obs - Model;
	int		nDiffSat;
	BYTE	pbDiffID[MAXSAT];		// 记录时差数据
	BYTE	pbSlip[MAXSAT];
	int		piDiff[MAXSAT];			// 记录时差值(0.1mm)
}RtkResult;

typedef struct tag_RtkWeather
{
	double		m_dAlpha[4];
	double		m_dBeta[4];
	double		m_dTS, m_dPS, m_dHS;	// 对流层改正的温度、气压、湿度
	int			m_iTrop;
	int			m_iIono;
}RtkWeather;

typedef struct tag_RtkEpoch
{
	BYTE	bFreq;			// 用于标识频率： [G2 G1 B3 B2 B1 L5 L2 L1]
	BYTE	bFreq2;			
	BYTE	bExternalFlag;	// 用与表示(SBAS_DIFF|BASE_FAIL|HEADING|EVENT)等信息
	BYTE	bDataLink;
	BYTE	bBluetooth;
	BYTE	bPowerStatus;	// 电源状态
	BYTE	bGPSStatus;		// 接收机状态
	BYTE	bCompassStatus;	// BD2的工作状态
	BYTE	bGloStatus;
	BYTE	bGalStatus;
	BYTE	bImuValid;		// IMU Valid
	WORD	wMsgID;			// 消息号
	WORD	wRefID;			// 参考站ID号
	WORD	wCompassCheck;	// 授权信息
	double	dGPSOffset;		// GPS参考站的时间偏移
	double	dCompassOffset;	// BD2参考站的时间偏移
	double	dGloOffset;
	double	dGalOffset;
	double	dRovOffset;		// 流动站时间偏移，对于单差观测值而言，此值为0.
	double	pdSysClock[4];	// 四系统的钟差
	double 	dGlonassSystemOffset;	// GLONASS与GPS系统时差
	int		iStationID;		// 对应的测站或接收机号
	int		iType;			// Receiver Type.
	int		iLag;			// Diff Delay.
	int		n;				// Obs Number.
	int		wWeek;
	double	t;
	double	dTimeOffset;	// 非单差的时间偏移（单点定位结果）
	double	dVTEC;			// 垂直方向的电子总量
	float	fPdop;
	double	vx;				// PVT的速度分量
	double	vy;
	double	vz;
	double	xBase;			// 参考站的固定坐标，用于流动基站
	double	yBase;
	double	zBase;
	int		iRoverX, iRoverY, iRoverZ;
	double	xVector;			// 基线增量
	double	yVector;
	double	zVector;
	double	dBaseOrgB;				// 新增参考站的初始经纬度
	double	dBaseOrgL;
	double	dBaseOrgH;
	double	dRoverOrgB;			// 新增流动站的初始经纬度
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
	double	dImuNonFixTime;	// 上一次GNSS更新到现在的时间；秒
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
	double	pdR5[MAXSAT];		// L5或B3的伪距单差
#ifdef _DEBUG
	double	pdClock[MAXSAT];	// 卫星钟差快速改正
#endif
	double	pdL1[MAXSAT];
	double	pdL2[MAXSAT];
	double	pdL5[MAXSAT];		// L5或B3的载波相位单差观测值
	BYTE	pbSN1[MAXSAT];
	BYTE	pbSN2[MAXSAT];
	BYTE	pbSN5[MAXSAT];	
	BYTE	pbRmsL1[MAXSAT];	// 伪距和载波的精度（2012-10-23）
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
// [2019-10-25] 混合了基站，主天线，从天线所有的观测数据，大小约4KB
typedef struct tag_MixEpoch
{
	WORD	wSize;						// 用于表示这个结构的大小
	WORD	wMsgID;						// 消息号
	WORD	wReceiverStatus;			// 接收机状态(代替：bExternalFlag..用于表示EVENT等信息等)
	WORD	wFreq;						// 用于表示所用的频率，BDS、GAL系统提供的信号不止一个频率，但结构仅使用了3个频率，需要标识
	WORD	wBaseID;					// 参考站ID号
	WORD	wRoverID;					// 接收机ID号
	WORD	wWeek;						// GPS周
	BYTE	bVersion;					// 版本号
	BYTE	bImuValid;					// IMU Valid(除了IMU外，还将包含其他信息)
	int		lTimeMS;					// 秒(ms)
	int		lRovOffset;					// 流动站钟差，单位为(mm)
	short	nObs;						// Obs Number(保留位32位的整数，留待日后使用，最大40)
	short	iEventOffset;				// Event时间相对于t的时间偏差；
	short	iGPSLag;					// GPS参考站的时间偏移（相对于t的整秒的偏移，单位0.01秒）
	short	iBDSLag;					// BD2参考站的时间偏移（0.01秒）
	short	iGloLag;
	short	iGalLag;					// GAL的时间偏移（0.01秒）	
	short	iBaseGpsOffset;				// 参考站观测量的钟差；(0.1us)
	short	iBaseBdsOffset;
	short	iBaseGloOffset;
	short	iBaseGalOffset;	
	// ---------------------- 坐标 ---------------------------------------------------------------------------------------------
	double	dBaseX, dBaseY, dBaseZ;		// 参考站的固定坐标，用于流动基站
	int		lBaseLat, lBaseLon, lBaseHcm;	// 基站的初始经纬度和高程，用于计算高度角等，对精度无要求（单位：0.000000001弧度和厘米）;	
	int		lRoverX, lRoverY, lRoverZ;	// 移动站相对于参考站的坐标，单位(mm)，移动站相对基站不能超过20000km
	int		lVelX, lVelY, lVelZ;		// PVT的速度分量(mm/s)
	// ---------------------- IMU Data ------------------------------------------------------------------------------------------
	double	dImuX, dImuY, dImuZ;
	double	dImuVelX, dImuVelY, dImuVelZ;
	double	dImuAccX, dImuAccY, dImuAccZ;
	double	dImuHeader, dImuPitch, dImuRoll;
	double	dImuNonFixTime;						// 上一次GNSS更新到现在的时间；秒
	double	dImuLastPosError;
	double	dImuLastVelError;
	// ===========================================================================================================================
	BYTE	pbID[MAXSAT];
	BYTE	pbFlag[MAXSAT];		// 用来表示Base\Rover\Slave三个数据有效性，或者伪距差分改正模式，定义参照相关文件！
	char	pbElevation[MAXSAT];
	short	piAzimuth[MAXSAT];
	int		plClock[MAXSAT];	// 卫星钟差快速改正(mm), 当数据工作在RTD模式下时，此值用来表示伪距改正数
	short	piIonoDelay[MAXSAT];// 模型化的大气改正（移动站处，单位cm），在PDP单点定位时将会使用
	short	piTropDelay[MAXSAT];// 对流层的大气改正（单位cm）	
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbBaseM1[MAXSAT];	// 观测量的指示,建议将pbModeL1和pbRmsL1合并为一个字节(高四位：L2C,L2P,低载噪比，低四位：载波RMS）
	BYTE	pbBaseM2[MAXSAT];
	BYTE	pbBaseM5[MAXSAT];
	BYTE	pbBaseCN1[MAXSAT];
	BYTE	pbBaseCN2[MAXSAT];
	BYTE	pbBaseCN5[MAXSAT];
	BYTE	pbBaseS1[MAXSAT];	// 周跳计数器
	BYTE	pbBaseS2[MAXSAT];
	BYTE	pbBaseS5[MAXSAT];
	BYTE	pbSoc[MAXSAT];		// non-dispersive interpolation residuals(mm)
	BYTE	pbSlc[MAXSAT];		// dispersive interpolation residuals(输入为WORD，这里改为BYTE)
	int		plBaseR1[MAXSAT];	// 单位：0.1米
	short	piBaseR21[MAXSAT];	// R2-R1(0.1米，绝对值小于3000米)
	short	piBaseR51[MAXSAT];	// R2-R1(0.1米，绝对值小于3000米)
	int		plBaseL1[MAXSAT];	// 单位1/256周
	int		plBaseL2[MAXSAT];
	int		plBaseL5[MAXSAT];	// L5或B3的载波相位单差观测值
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbRoverM1[MAXSAT];	// 观测量的指示,建议将pbModeL1和pbRmsL1合并为一个字节
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
	int		plRoverR1[MAXSAT];	// 单位：0.1米
	short	piRoverR21[MAXSAT];	// R2-R1(0.1米，绝对值小于3000米)
	short	piRoverR51[MAXSAT];	// R5-R1(0.1米，绝对值小于3000米)
	int		plRoverL1[MAXSAT];	// 单位1/256周
	int		plRoverL2[MAXSAT];
	int		plRoverL5[MAXSAT];	// L5或B3的载波相位观测值
	// ----------------------------------------------------------------------------------------------------------------------------
	BYTE	pbSlaveM1[MAXSAT];	// 观测量的指示,建议将pbModeL1和pbRmsL1合并为一个字节
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
	int		plSlaveR1[MAXSAT];	// 单位：0.1米
	short	piSlaveR21[MAXSAT];	// R2-R1（0.1米，绝对值小于3000米）
	short	piSlaveR51[MAXSAT];	// R5-R1（0.1米，绝对值小于3000米）
	int		plSlaveL1[MAXSAT];	// 单位：1/256周
	int		plSlaveL2[MAXSAT];
	int		plSlaveL5[MAXSAT];	// L5或B3的载波相位观测值
}MixEpoch, * pMixEpoch;
*/

// [2013-06-02]单点定位结果[V2]
typedef struct tag_RtkSinglePosition
{
	unsigned long	dwSize;
	BYTE			nUsing;		// 总卫星数
	BYTE			nFix;
	WORD			nBad;		// 粗差数量
	WORD			nStep;		// 最小二乘计算迭代次数
	BYTE			bElevationCutOff;	// 单点定位计算使用的高度截止角（输入）
	WORD			wWeek;
	double			t;
	double			dRms;		// 均方差
	double			dPdop;		// 几何精度因子
	double			dQxx;
	double			dQxy;
	double			dQxz;
	double			dQyy;
	double			dQyz;
	double			dQzz;
	double			x;			// 单点定位结果
	double			y;
	double			z;
	double			pdTimeOffset[MAX_SYSTEM];
	BYTE			pbClockIndex[MAX_SYSTEM];	// pdTimeOffset对应的卫星系统编号：0-GPS; 1-BDS; 2-GLO; 3-GAL; FF-NONE
	BYTE			pbID[MAXSAT];				// 单点定位卫星数
	BYTE			pbFlag[MAXSAT];				// 卫星使用状态
	short int		piRes[MAXSAT];				// 单位为0.01米的残差(Max: 300; Min: -300); 
}RtkSinglePosition, *pRtkSinglePosition;

#endif // !defined(AFX_RTKTYPE_H__INCLUDED_)
