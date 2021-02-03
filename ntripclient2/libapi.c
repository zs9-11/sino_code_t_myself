#include "libapi.h"
#include "libsockets.h"
#include <stdio.h>
//RtcmDataResponse    *dataCallback = nullptr;
//RtcmStatusResponse  *statusCallback = nullptr;


void startSdk(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp)
{
    if(!data_rsp){
        return;
    }
    //dataCallback = data_rsp;
    //statusCallback = status_rsp;
    printf("###startSdk###\r\n");
    start_ntrip(data_rsp, status_rsp);
}


int sendGGA(IN  const char* ggaRawBuf, IN const int length)
{
    send_request(ggaRawBuf, length);
}

#define CMCC_SDK_VERSION    "1.0.0"  
const char* getSdkVersion(void)
{
    return CMCC_SDK_VERSION;
}

void stopSdk()
{
    stop_ntrip();
    return;
}

int sendBestPos(IN  const void* data, IN  int  size)
{
    return 0;
}

