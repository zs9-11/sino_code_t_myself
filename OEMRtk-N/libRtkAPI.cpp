
extern "C" {
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "libRtkAPI.h"
#include "logger.h"
}

#include "QRtkImpl.h"

CQRtkImpl   *_pRtkEngine = nullptr;
uint16_t    _constellationMask = (RTK_GNSS_CONSTELLATION_GPS| RTK_GNSS_CONSTELLATION_GALILEO| RTK_GNSS_CONSTELLATION_BEIDOU);
uint16_t    _solutionFormat = RTK_GNSS_SOLUTION_NMEA;

int rtk_init(pRtkReport callback, uint16_t constellationMask, uint16_t solutionFormat)
{
    if (!callback){
        return -1;
    }
    LOG_INFO("RTK Engine Initialize\n");
    _constellationMask = constellationMask;
    _solutionFormat =  solutionFormat;
#ifdef  _EC100Y_PLATFORM
    static CQRtkImpl   rtkEngine(callback);
    _pRtkEngine = &rtkEngine;
#else
    if (_pRtkEngine == nullptr){
	    _pRtkEngine = new CQRtkImpl(callback);
    }
#endif
    return 0;
}

void Clearup()
{
    LOG_INFO("RTK Engine Freed\n");
#ifndef  _EC100Y_PLATFORM
    if(_pRtkEngine){
        delete _pRtkEngine;
        _pRtkEngine = nullptr;
    }
#endif
}

int PushRtcm(int source, const char* msg, int len)
{
    if (_pRtkEngine){
        return _pRtkEngine->PushRtcm(source, msg, len);
    }
    return -1;
}

int PushGGA(int source, const char* msg, int len)
{
    if (_pRtkEngine){
        return _pRtkEngine->PushGGA(source, msg, len);
    }
    return -1;
}

int PushEphemeris(const ServerEphemeris *pEphem)
{
    if (_pRtkEngine){
        _pRtkEngine->SendEphemerisToQRtk(const_cast<ServerEphemeris *>(pEphem));
    }else{
        return -1;
    }
    return 0;
}

int PushGloEphemeris(const ServerGloEphemeris *pEphem)
{
    if (_pRtkEngine){
        _pRtkEngine->SendGloEphemToQRtk(const_cast<ServerGloEphemeris *>(pEphem));
    }else{
        return -1;
    }
    return 0;
}

int PushEpoch(int source, const WangEpoch *pEpoch)
{
    if (_pRtkEngine){
        _pRtkEngine->PushEpoch(source, pEpoch);
    }else{
        return -1;
    }
    return 0;
}
