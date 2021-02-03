#include "libsockets.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>
#include <ql_type.h>
#include <ql_rtos.h>
#include <ql_data_call.h>
#include <sockets.h>
#include <netdb.h>


#include "RingBuffer.h"

//#define TCP_SERVER_DOMAIN "139.199.240.17"
//#define TCP_SERVER_DOMAIN "120.204.202.101"
//#define TCP_SERVER_PORT 80
//#define TCP_SERVER_PORT 8105
#define TCP_SERVER_DOMAIN "140.207.166.210"
//#define TCP_SERVER_PORT 7906
#define TCP_SERVER_PORT 25001
#define TCP_CONNECT_TIMEOUT_S 10
#define TCP_RECV_TIMEOUT_S 10
#define TCP_CLOSE_LINGER_TIME_S 10
//#define TCP_CLIENT_SEND_STR "tcp client send string"
#define PROFILE_IDX 1

struct in_addr ip4_addr = {0};

struct addrinfo* res = NULL, hints;
int	sock_fd     = -1;

RtcmDataResponse    *dataCallback = NULL;
RtcmStatusResponse  *statusCallback = NULL;
//测试
//char gga[] = "$GPGGA,023352.55,3113.2180600,N,12148.8000000,E,1,00,1.0,-6.078,M,11.078,M,0.0,*60\n";
char gga[] = "$GNGGA,160008.00,3121.0111141,N,12117.5496604,E,4,20,0.5,36.3324,M,0.000,M,08,0009*41\n";
/*
osThreadId_t socketThr=NULL;
osMutexId_t  ggaMtx=NULL;

osThreadAttr_t *socketThrAttr;
*/

ql_task_t socketThr = NULL;
ql_mutex_t  ggaMtx = NULL;
//#define TASK_STACK_SIZE 1024
//#define TASK_STACK_SIZE 4096
#define TASK_STACK_SIZE 10240
//#define TASK_PRIORITY 100
#define TASK_PRIORITY 10
/**2020-11-24 correct data is less ,task pri improved.*/
//#define TASK_PRIORITY 3
#define SUB_TASK_NAME "rtk_correct"
#define ARGV_TO_SUB_TASK "argument passed to sub task"


ringbuffer  *ggaRingBuffer = NULL;


#define RTCM_READ_BUF_SIZE        1024
//#define LOGIN_REQUEST_BUF_SIZE    1024
#define LOGIN_REQUEST_BUF_SIZE    512
//#define GGA_REQUEST_BUF_SIZE    1024
#define GGA_REQUEST_BUF_SIZE    256
//#define DATA_RING_BUF_SIZE    8192
#define DATA_RING_BUF_SIZE    1024

char rtcmReadBuffer[RTCM_READ_BUF_SIZE] = {0};
char loginRequestBuffer[LOGIN_REQUEST_BUF_SIZE] = {0};
char ggaRequestBuffer[GGA_REQUEST_BUF_SIZE] = {0};
 
/*
char*       rtcmReadBuffer = NULL;
char*       loginRequestBuffer = NULL;
char*       ggaRequestBuffer = NULL;
*/

//char kCasterAgent[] = "NTRIP NTRIPCaster/20191018";
char kClientAgent[] = "NTRIP NTRIPClient/20191018";
//char kServerAgent[] = "NTRIP NTRIPServer/20191018";

//char mountCode[]  = "RTCM33GRCE1";
//char usrPwd[]  = "123:123";
int isStopNtripService = 0;

static void close_socket(void)
{
    if(res){
        freeaddrinfo(res);
    }

	if(sock_fd >= 0){
		struct linger linger = {0};

		linger.l_onoff = 1;
		linger.l_linger = TCP_CLOSE_LINGER_TIME_S;

		setsockopt(sock_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
        setsockopt(sock_fd, IPPROTO_TCP, TCP_CLOSE_TIMEROUT, &linger.l_linger, sizeof(linger.l_linger));

		close(sock_fd);
	}
	//printf("###close socket###\r\n");
}

static int open_socket(void)
{
	//printf("###open socket enter###\r\n");
	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo_with_pcid(TCP_SERVER_DOMAIN, NULL, &hints, &res, PROFILE_IDX) != 0)
	{
		printf("*** DNS fail ***\r\n");
        //statusCallback
        return -1;
	}

	//printf("###socket GetAddrinfo###\r\n");

	int ret = socket(AF_INET, SOCK_STREAM, 0);
	if(ret < 0)
	{
		printf("*** socket create fail ***\r\n");
        //statusCallback
        return -1;
	}

	//printf("###socket Created###\r\n");
	sock_fd = ret;

    struct timeval	t;
	//struct addrinfo		* res, hints;
	struct sockaddr_in	* ip4_svr_addr;
	struct sockaddr_in	ip4_local_addr = {0};
	int				sock_nbio	= 1;
    
    ioctl(sock_fd, FIONBIO, &sock_nbio);

	ip4_local_addr.sin_family = AF_INET;
	ip4_local_addr.sin_port = htons(ql_soc_generate_port());
	ip4_local_addr.sin_addr = ip4_addr;
	
	ret = bind(sock_fd, (struct sockaddr *)&ip4_local_addr, sizeof(ip4_local_addr));
	if(ret < 0)
	{
		printf("*** bind fail ***\r\n");
		return ret;
	}
	
	//printf("###socket binded###\r\n");
	ip4_svr_addr = (struct sockaddr_in *)res->ai_addr;
	ip4_svr_addr->sin_port = htons(TCP_SERVER_PORT);

	ret = connect(sock_fd, (struct sockaddr *)ip4_svr_addr, sizeof(struct sockaddr));

	//printf("connect ret: %d, errno: %u\r\n", ret, errno);

	if(ret == -1 && errno != EINPROGRESS)
	{
		printf("*** connect fail ***\r\n");
        //statusCallback
		return ret;
	}
    
	//printf("###open socket exit###\r\n");
    //statusCallback
	return 0;
}

static int write_socket(const char*  data, int length)
{
	int ret = send(sock_fd, (const void*)data, length, 0);
	if(ret < 0)
	{
		printf("*** send fail ***\r\n");
	}

	return ret;
}

static int read_socket(char* data, int length)
{
    int ret = recv(sock_fd, data, length, 0);
	if(ret > 0)	{
		//printf("recv data: [%d]%s\r\n", ret, data);
	}else if(ret == 0){
		printf("*** peer closed ***\r\n");
		//return -1;
		ret = -1;
		//printf("*** read nothing ***\r\n");
	}else{
		if(!(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)){
			printf("*** error occurs ***\r\n");
		    //return -1;
		    ret = -1;
		}else{
			printf("wait for a while\r\n");
		    //return 0;
		    ret = 0;
            ql_rtos_task_sleep_ms(20);
		}
	}
    return ret;
}
/*
static void ql_nw_status_callback(int profile_idx, int nw_status)
{
	printf("profile(%d) status: %d\r\n", profile_idx, nw_status);
}

static void datacall_start(void)
{
	printf("wait for network register done\r\n");

	if(ql_network_register_wait(120) != 0)
	{
		printf("*** network register fail ***\r\n");
	}
	else
	{
		printf("doing network activing ...\r\n");
		
		ql_wan_start(ql_nw_status_callback);
		ql_set_auto_connect(1, TRUE);
		//ql_start_data_call(1, 0, "3gnet.mnc001.mcc460.gprs", NULL, NULL, 0);
		ql_start_data_call(1, 0, "CMIOTDDCX.JS", NULL, NULL, 0);
	}
}
*/
void run_socket()
{
    //应用拔号
    //datacall_start();

    struct ql_data_call_info info = {0};
	char ip4_addr_str[16] = {0};

	ql_get_data_call_info(1, 0, &info);
/*
	printf("info.profile_idx: %d\r\n", info.profile_idx);
	printf("info.ip_version: %d\r\n", info.ip_version);
	printf("info.v4.state: %d\r\n", info.v4.state);
	printf("info.v4.reconnect: %d\r\n", info.v4.reconnect);

	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.ip: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	printf("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
*/
    //获取ip
	ip4_addr = info.v4.addr.ip;

	//if(info.v4.state)
    ql_task_t task_id; 
    ql_rtos_task_get_current_ref(&task_id);
    printf("*******Correct Task:%p STARTING*******\r\n", task_id);

	int				ret			= 0;
	int				sock_error  = 0;
	fd_set 			read_fds, write_fds;
    struct timeval	t;
    while(!isStopNtripService){
        int      isLogined  = 0;
        int      isAuthed  = 0;
        close_socket();
        if(open_socket() < 0){
            continue;
        }
    
        //printf("###ntrip connected###\r\n");
        //printf("*******Correct Task:%p ntrip connected*******\r\n", task_id);
        while(1){
	        t.tv_sec = TCP_CONNECT_TIMEOUT_S;
	        t.tv_usec = 0;

	        FD_ZERO(&read_fds);
	        FD_ZERO(&write_fds);

	        FD_SET(sock_fd, &read_fds);
	        FD_SET(sock_fd, &write_fds);

	        //ret = select(sock_fd + 1, &read_fds, &write_fds, NULL, &t);
	        ret = select(sock_fd + 1, &read_fds, &write_fds, NULL, NULL);

	        //printf("select ret: %d\r\n", ret);
            //printf("*******Correct Task:%p, select ret: %d*******\r\n", task_id, ret);
	        if(ret < 0)	{
		        printf("*** select error ***\r\n");
                //statusCallback
                break;
            }else if( ret == 0 ){
		        printf("*** select timeout ***\r\n");
                continue;
            }else{
	            if(!FD_ISSET(sock_fd, &read_fds) && !FD_ISSET(sock_fd, &write_fds))	{
		            printf("*** connect fail ***\r\n");
                    //statusCallback
                    break;
		        }else if(FD_ISSET(sock_fd, &read_fds) && !FD_ISSET(sock_fd, &write_fds)){
		            printf("*** connect fail ***\r\n");
                    //statusCallback
                    break;
	            }
                
                if(FD_ISSET(sock_fd, &write_fds))	{
                    //printf("*******Correct Task:%p Ready for Writing *******\r\n", task_id);
                    if(!isLogined){
		                //printf("*** send login request***\r\n");
                        printf("*******Correct Task:%p send login request*******\r\n", task_id);
                        write_socket(loginRequestBuffer, strlen(loginRequestBuffer));
                        isLogined=1; 
                    }else if(isAuthed){
                        //write_socket(gga, strlen(gga));
                        /*
	                    ql_rtos_mutex_lock(ggaMtx, QL_WAIT_FOREVER);
                        int ggaRequestSize = 0;
                        if(rb_read(ggaRingBuffer, (char*)&ggaRequestSize, sizeof(int))){
                            printf("*******Correct Task:%p get GGA request:%d*******\r\n", task_id, ggaRequestSize);
                            rb_read(ggaRingBuffer, ggaRequestBuffer, ggaRequestSize);
                        }
	                    ql_rtos_mutex_unlock(ggaMtx);

                        if(ggaRequestSize){
		                    //printf("*** send GGA request:%s***\r\n", ggaRequestBuffer);
                            printf("*******Correct Task:%p send GGA request*******\r\n", task_id);
                            write_socket(ggaRequestBuffer, ggaRequestSize);
                        }
                        */
	                    ql_rtos_mutex_lock(ggaMtx, QL_WAIT_FOREVER);
                        int ggaRequestSize = strlen(ggaRequestBuffer);
                        if(ggaRequestSize > 0){
                            //printf("*******Correct Task:%p send GGA request*******\r\n", task_id);
                            write_socket(ggaRequestBuffer, ggaRequestSize);
                            memset(ggaRequestBuffer, 0x00, sizeof(ggaRequestBuffer));
                        }
	                    ql_rtos_mutex_unlock(ggaMtx);
	                }//如果登录没有返回，超时重新登录；超过次数断开连接
                }
                if(FD_ISSET(sock_fd, &read_fds))	{
                    //if(read() < 0){
                    //    break;
                    //}
                    //printf("*******Correct Task:%p Ready for reading *******\r\n", task_id);
                    int ret = read_socket(rtcmReadBuffer, RTCM_READ_BUF_SIZE);
                    if(ret > 0){
                        if(isLogined && !isAuthed){
                            //认证登录
		                    //printf("*** Auth info:%s ***\r\n", rtcmReadBuffer);
                            printf("*******Correct Task:%p,Auth info:%s, len:%d*******\r\n", task_id, rtcmReadBuffer, ret);
                            //if(!strncmp(rtcmReadBuffer, "ICY 200 OK\r\n", 12)){
		                        //printf("*** Authed :%s ***\r\n", rtcmReadBuffer);
                                isAuthed  = 1;
                                memset(rtcmReadBuffer, 0x00, RTCM_READ_BUF_SIZE);
                            //}
                        }else{
                            dataCallback->cb_rtcmdata(rtcmReadBuffer, ret);
                        }       
                    }else if(ret < 0){
                        break;
                    }
	            }
                ql_rtos_task_sleep_ms(10);
            }
	    }
    }
    printf("*******Correct Task:%p EXITING*******\r\n", task_id);
}

const char kBase64CodingTable[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char GetCharByIndex(const int index) {
  return kBase64CodingTable[index];
}

int Base64Encode(const char *src, char *result) {
  char temp[3] = {0};
  int i = 0, j = 0, count = 0;
  int len = strlen(src);
  if (len == 0) {
    return -1;
  }

  if (len%3 != 0) {
    count = 3 - len%3;
  }

  while (i < len) {
    strncpy(temp, src+i, 3);
    result[j+0] = GetCharByIndex((temp[0]&0xFC) >> 2);
    result[j+1] = GetCharByIndex(((temp[0]&0x3) << 4) | ((temp[1]&0xF0) >> 4));
    if (temp[1] == 0) {
      break;
    }
    result[j+2] = GetCharByIndex(((temp[1]&0xF) << 2) | ((temp[2]&0xC0) >> 6));
    if (temp[2] == 0) {
      break;
    }
    result[j+3] = GetCharByIndex(temp[2]&0x3F);
    i += 3;
    j += 4;
    memset(temp, 0x0, 3);
  }

  while (count) {
    result[j+4-count] = '=';
    --count;
  }

  return 0;
}

void start_ntrip(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp)
{
    printf("###start_ntrip enter###\r\n");
    dataCallback = data_rsp;
    statusCallback = status_rsp;
    /* 
    rtcmReadBuffer = calloc(1, RTCM_READ_BUF_SIZE);
    
    loginRequestBuffer = calloc(1, LOGIN_REQUEST_BUF_SIZE);
    ggaRequestBuffer = calloc(1, GGA_REQUEST_BUF_SIZE);
    */
    //ggaMtx = osMutexNew(NULL);
    ql_rtos_mutex_create(&ggaMtx);

    //ggaRingBuffer = rb_create(DATA_RING_BUF_SIZE);

    //usrPwd 加密？
    //char mountCode[]  = "RTCM33GRCE1";
    //char usrPwd[]  = "123:123";
   
//    char mountCode[]  = "RTCM32";
//    char usrPwd[]  = "LZQ:123";
    char mountCode[]  = "LZQtest";
    //char mountCode[]  = "GS-K708BD3-RTCM32";
    //char mountCode[]  = "8KM-RTCM32";
    //char mountCode[]  = "GS-707-FU-RTCM32";
    //char mountCode[]  = "GS-K708-BH-RTCM32";
    //char mountCode[]  = "708-C6-RTCM32";
    char usrPwd[]  = "LZQ-didi:123";
    char userinfo[64] = {0};
    Base64Encode(usrPwd, userinfo);
    snprintf(loginRequestBuffer, LOGIN_REQUEST_BUF_SIZE,
           "GET /%s HTTP/1.1\r\n"
           "User-Agent: %s\r\n"
           "Accept: */*\r\n"
           "Connection: close\r\n"
           "Authorization: Basic %s\r\n"
           "\r\n",
           mountCode, kClientAgent, userinfo);
          // mountCode, "", usrPwd);
    /*
    socketThrAttr= calloc(1, sizeof(osThreadAttr_t ));
    socketThrAttr->name = "socketThread";
    socketThrAttr->stack_mem = calloc(1, 4096);
    socketThrAttr->stack_size = 4096;
    socketThrAttr->priority = osPriorityNormal;
    socketThr = osThreadNew(run_socket, NULL, socketThrAttr);

	//socketThr = osThreadNew(run_socket, NULL, NULL);
    */
	ql_rtos_task_create(&socketThr, TASK_STACK_SIZE, TASK_PRIORITY, SUB_TASK_NAME, run_socket, ARGV_TO_SUB_TASK);
    //run_socket();
    printf("###start_ntrip exit###\r\n");
}

void send_request(const char* data, int length)
{
    //mutex
    //memcpy ggaRequestBuffer或C队列 
    /*
    while (1){
	    ql_rtos_mutex_lock(ggaMtx, QL_WAIT_FOREVER);
        if(rb_get_space_free(ggaRingBuffer) < (length + sizeof(int))){
            ql_rtos_mutex_unlock(ggaMtx);
            printf("###ntrip gga RingBuff Not Enough###\r\n");
            continue;
        }

        int writenSize = rb_write(ggaRingBuffer, (char*)&length, sizeof(int));
        writenSize = rb_write(ggaRingBuffer, (char*) data, length);
        printf("###ntrip get gga request:%d###\r\n",writenSize);
        break;
    }
    ql_rtos_mutex_unlock(ggaMtx);
    */
    /*
	ql_rtos_mutex_lock(ggaMtx, QL_WAIT_FOREVER);
    if(rb_get_space_free(ggaRingBuffer) < (length + sizeof(int))){
        //ql_rtos_mutex_unlock(ggaMtx);
        printf("###ntrip gga RingBuff Not Enough###\r\n");
        rb_cleanup(ggaRingBuffer);
    }

    int writenSize = rb_write(ggaRingBuffer, (char*)&length, sizeof(int));
    writenSize = rb_write(ggaRingBuffer, (char*) data, length);
    printf("###ntrip get gga request:%d###\r\n",writenSize);
 
    ql_rtos_mutex_unlock(ggaMtx);
    */
	ql_rtos_mutex_lock(ggaMtx, QL_WAIT_FOREVER);
    memset(ggaRequestBuffer, 0x00, GGA_REQUEST_BUF_SIZE);
    memcpy(ggaRequestBuffer, data, length);
    ql_rtos_mutex_unlock(ggaMtx);
}

void stop_ntrip(void)
{
    //停止线程，释放分配的内存 
    isStopNtripService = 1;
    //wait thread terminate
}
