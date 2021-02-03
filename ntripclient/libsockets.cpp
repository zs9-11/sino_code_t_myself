#include <string.h>

#include "libsockets.h"

#include "ini.h"
#include "ntrip_client.h"
using libntrip::NtripClient;

RtcmDataResponse    *dataCallback = NULL;
RtcmStatusResponse  *statusCallback = NULL;


CONNECT_CONFIG  con_conf;
NtripClient ntrip_client;

#ifdef __cplusplus
extern "C" {
#endif

static int  handler_conn_conf(void* user, const char* section, const char* name,
                 const char* value)
{
    CONNECT_CONFIG* pconfig = (CONNECT_CONFIG*)user;

    if (MATCH("SERVICE", "CORRECT_SERVER_IP")) {
        snprintf(pconfig->_server_ip, 32, "%s", value);
    }else if (MATCH("SERVICE", "CORRECT_SERVER_PORT")) {
        pconfig->_server_port = atoi(value);
    }else if (MATCH("SERVICE", "CORRECT_MOUNTCODE")) {
        snprintf(pconfig->_mount_code, 32, "%s", value);
    }else if (MATCH("SERVICE", "CORRECT_LOGIN_USER")) {
        snprintf(pconfig->_user, 12, "%s", value);
    }else if (MATCH("SERVICE", "CORRECT_LOGIN_PASSWD")) {
        snprintf(pconfig->_passwd, 8, "%s", value);
    }else{
        return 0;  
                 
    }
    return 1;
}
#ifdef __cplusplus
}
#endif


static void private_conn_default_conf()
{
    snprintf(con_conf._server_ip, 32, "%s", TCP_SERVER_DOMAIN);
    con_conf._server_port = TCP_SERVER_PORT;
    snprintf(con_conf._mount_code, 32, "%s", NTRIP_DEFAULT_MOUNT_POINT);
    snprintf(con_conf._user, 12, "%s", NTRIP_DEFAULT_LOGIN_USER);
    snprintf(con_conf._passwd, 8, "%s", NTRIP_DEFAULT_LOGIN_PASSWD);
}

void onReceiveRtcm(const char *data, const int& length)
{
    if(dataCallback){
        dataCallback->cb_rtcmdata((void*) data, length);
    }
}

void start_ntrip(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp)
{
    printf("###start_ntrip enter###\n");
    memset((void*)&con_conf, 0x00, CONNECT_CONFIG_SIZE);
    dataCallback = data_rsp;
    statusCallback = status_rsp;
    if (ini_parse(NTRIP_DEFAULT_CONFIG_FILE, handler_conn_conf, &con_conf) < 0) {
        private_conn_default_conf();
        //LOG_ERROR("Can't load log config:%s\n", config);
        //return ;
    }


    ntrip_client.Init( con_conf._server_ip, con_conf._server_port,
                      con_conf._user, con_conf._passwd, con_conf._mount_code);

    ntrip_client.OnReceived(onReceiveRtcm); 
 
    ntrip_client.Start();

    printf("###start_ntrip exit###\n");
}

void send_request(const char* data, int length)
{
    //mutex
    ntrip_client.set_gga_buffer(std::string(data, length));
}

void stop_ntrip(void)
{
    //停止线程，释放分配的内存 
    //wait thread terminate
    ntrip_client.Stop();
}
