// [2019-09-19] 增加创建RTK引擎的接口

#ifndef _POLARIS_INTERFACE__
#define _POLARIS_INTERFACE__
#include "Common.h"
#include "QRTKInterface.h"

RTK_HANDLE CreateNewRtkEngine();
void RemoveRtkEngine(RTK_HANDLE hRtk);
#ifdef  _EC100Y_PLATFORM
#include "RtkType.h"
void RtkInitilization(RTK_HANDLE hRtk, const int iMode);
void PrintRTK(RTK_HANDLE hRtk,char* strDebug);
bool SendOneEphemerisToRtk(RTK_HANDLE hRtk, ServerEphemeris* pEphemeris, char* strDebug);
int iSinglePosition(RTK_HANDLE hRtk, RtkEpoch* pEpoch, RtkSinglePosition* pPos, char* strDebug);
void SendOneEpochToRtk(RTK_HANDLE hRtk, M919TotalInfor* pEpoch919, RTK_result *pRtkRes, char* RtkDebug);
void PrintRtkEpoch(RtkEpoch* pEpoch);
int	iGetSatPos(RTK_HANDLE hRtk, const int nSat, const double t, const double x, const double y, const double z, const unsigned char* pbID, double* pdDistance, double* pdAzimuth, double* pdElevation, double* pdSatClock);
int	iGetSatMsg(RTK_HANDLE hRtk, const int nSat, const double t, const unsigned char* pbID, double* pdSatX, double* pdSatY, double* pdSatZ, double* pdSatClock, char* strDebug);
RTK_HANDLE GetObjectHandle(RTK_HANDLE hRtk, char* RtkDebug, const int iObject);
#else
bool SendOneEphemerisToRtk(RTK_HANDLE hRtk, ServerEphemeris* pEphemeris);
int	iGetSatPos(RTK_HANDLE hRtk, const int nSat, const double t, const double x, const double y, const double z, const unsigned char* pbID, double* pdDistance, double* pdAzimuth, double* pdElevation);
#endif

bool SendOneGloEphemToRtk(RTK_HANDLE hRtk, Mes723Infor* pMsg723);
void SendOneEpochToRtk(RTK_HANDLE hRtk, M919TotalInfor* pEpoch919, RTK_result *pRtkRes, char* RtkDebug);
void GetRtkVersion(char* buff, int nLength);

void CheckKeyForPPK();
void SetViewEnable(RTK_HANDLE hRtk, const bool _bViewEnable);
void SetLogEnable(RTK_HANDLE hRtk, const bool _bLogEnable);
void SetOutputFormat(RTK_HANDLE hRtk, const int _iOutputFormat);
void SetupRtk(RTK_HANDLE hRtk, int iMode, int iParam);
int iGetRtkSetup(RTK_HANDLE hRtk, const int iMode);

#endif

