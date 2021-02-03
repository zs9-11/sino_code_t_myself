#ifndef _LIB_SOCKET_H
#define _LIB_SOCKET_H

#include "libapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
//#define TCP_SERVER_DOMAIN "120.204.202.101"
//#define TCP_SERVER_PORT 8105

//公司内网
//#define TCP_SERVER_DOMAIN "192.168.2.192"
//#define TCP_SERVER_DOMAIN "192.168.2.80"

//公司外网
#define TCP_SERVER_DOMAIN "140.207.166.210"

//不包含星历
/*
#define TCP_SERVER_PORT 7906
char mountpoint[]  = "RTCM32";
*/
//包含星历
/**/
#define TCP_SERVER_PORT 25001 
//char mountpoint[]  = "56-707V1-RTCM32";
//char user[]  = "LZQ";
//char passwd[]  = "123";
/**/
#define NTRIP_DEFAULT_MOUNT_POINT   "56-707V1-RTCM32"
#define NTRIP_DEFAULT_LOGIN_USER    "LZQ"
#define NTRIP_DEFAULT_LOGIN_PASSWD  "123"

#define NTRIP_DEFAULT_CONFIG_FILE   "/etc/sino.conf"

typedef struct correct_context_{
    char    _server_ip[32];
    uint16_t _server_port;
    char    _mount_code[32];
    char    _user[12];
    char    _passwd[8];
}CONNECT_CONFIG;
#define  CONNECT_CONFIG_SIZE     sizeof(CONNECT_CONFIG)

void start_ntrip(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp);

void send_request(const char* data, int length);

void stop_ntrip(void);

#ifdef __cplusplus
}
#endif

#endif//_LIB_SOCKET_H

