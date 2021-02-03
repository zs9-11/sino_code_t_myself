#ifndef _LIB_API_H
#define _LIB_API_H

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT
#define MAX_RTCM_LEN 20480
#define MAX_MSG_LEN 20480
#define MAX_ENCRY_MSG_LEN 40960
#define MAX_GGA_LEN 2560
#define MAX_CACHE_LEN 20480

typedef void (*SdkRtcmData)(void *data, int length);
typedef void (*SdkRtcmStatus)(int status);

typedef struct _rtcm_data_response_{
    SdkRtcmData cb_rtcmdata;
} RtcmDataResponse;

typedef struct _rtcm_status_response_{
    SdkRtcmStatus cb_status;
} RtcmStatusResponse;

/* This is a non-blocking API for send GGA to CORS
 *
 * @param[in]  ggaRawBuf : data to send in ASCII character
 * @param[in]  length    : data len to send in ASCII character
 *
 * @return:
 *   0 if success
 *  -1 if fail
 */
int sendGGA(IN  const char* ggaRawBuf, IN const int length);


/**
 * @brief : get SDK version
 *
 * @return:
 *   a pointer to version
 */
const char* getSdkVersion(void);

/**
 * @brief : start SDK statemachine
 * @param[in]  data_rsp  : include a callback function for pass the rtcm data
 * @param[in]  status_rsp: include a callback function for pass the rtcm status
 * @return:
 *   void
 */
void startSdk(RtcmDataResponse *data_rsp, RtcmStatusResponse *status_rsp);

/**
 * @brief : stop SDK statemachine
 * @return:
 *   void
 */
void stopSdk();

/* This is a Synchronous block API for get RTCM data from CORS platform(only for DEMO)
 *
 * @param[in]  data: best pos data
 * @param[in]  size: data size
 *
 * @return:
 *   0 if success
 *  -1 if fail
 */
int sendBestPos(IN  const void* data,
                IN  int         size);
#ifdef __cplusplus
}
#endif

#endif//_LIB_API_H

