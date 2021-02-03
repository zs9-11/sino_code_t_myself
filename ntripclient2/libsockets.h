#ifndef _LIB_SOCKET_H
#define _LIB_SOCKET_H

#include "libapi.h"

#ifdef __cplusplus
extern "C" {
#endif
void start_ntrip(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp);

void send_request(const char* data, int length);

void stop_ntrip(void);

#ifdef __cplusplus
}
#endif

#endif//_LIB_SOCKET_H

