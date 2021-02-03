
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "EnginePluginAPI.h"

#include <thread>
#include <chrono>
#include <sstream>

#include "ppecom.h"
#include "PPELibAPI.h"
#include "libRtkAPI.h"
#include "logger.h"
#include "ini.h"

#include "libapi.h"
#include "RingBuffer.h"
#ifdef _LINK_SERIAL_PORT
#include "serialport.h"
#endif
#include "logger.h"
#include "ini.h"

#include <mutex>
#include <condition_variable>

volatile bool isTerminated = false;

epGnssMeasurementReport  msrReport;
epPVTReport              posReport;
epGnssEphemerisReport    ephemReport;


std::thread ppeThr;
std::mutex mtx_gnss;
std::condition_variable cv_gnss_r;
std::condition_variable cv_gnss_w;

int  week = 0;
ringbuffer *gnssRingBuffer = nullptr;

std::thread corsThr;
std::mutex mtx_cors;
std::condition_variable cv_cors_r;
std::condition_variable cv_cors_w;
ringbuffer *corsRingBuffer = nullptr;


volatile bool hasGotObs = false;
uint8_t  obsNum = 0;
#define OBS_NUM_MAX_LEAP   10

static const uint16_t MEASUREMENT_DATA_SIZE = sizeof(epGnssMeasurementReport);
static const uint16_t EPHEMERIS_DATA_SIZE = sizeof(epGnssEphemerisReport);
static const uint16_t PVTREPORT_DATA_SIZE = sizeof(epPVTReport);
static const int GNSS_DATA_RING_BUF_SIZE =  MEASUREMENT_DATA_SIZE*25;  
static const int CORS_DATA_RING_BUF_SIZE =  MEASUREMENT_DATA_SIZE*10;  

static const uint16_t RTCM_STATUS_DATA_SIZE = sizeof(int);

static const uint16_t DATA_HEADER_LENGTH  = 7;
static char DATA_HEADER[DATA_HEADER_LENGTH] = {0xAA,0x44,0x13,0x00,0x00,0x00,0x00};
static const uint16_t  DATA_MEASUREMENT  =  1;  
static const uint16_t  DATA_EPHEMERIS =  2;
static const uint16_t  DATA_PVTREPORT =  3;
static const uint16_t  DATA_RTCM =  4; //correction data
static const uint16_t  DATA_GGA =  5;
static const uint16_t  DATA_RTCM_STATUS =  6;//correction service status
static const uint16_t  DATA_EPOCH_BASE =  7;//base
static const uint16_t  DATA_EPOCH_ROVER =  8;//base
/**********************************************************************************************************************************************
*
*EP   Callbacks 
*
**********************************************************************************************************************************************/
#define EP_GNSS_SUB_CONSTELLATION (EP_GNSS_CONSTELLATION_GPS |\
                                   EP_GNSS_CONSTELLATION_GALILEO |\
                                   EP_GNSS_CONSTELLATION_GLONASS |\
                                   EP_GNSS_CONSTELLATION_BEIDOU)

#define EP_GNSS_SUB_SIGNAL_TYPE (EP_GNSS_SIGNAL_GPS_L1CA | EP_GNSS_SIGNAL_GPS_L1C |\
        EP_GNSS_SIGNAL_GPS_L2C_L | EP_GNSS_SIGNAL_GPS_L5_Q | EP_GNSS_SIGNAL_GLONASS_G1_CA |\
        EP_GNSS_SIGNAL_GLONASS_G2_CA | EP_GNSS_SIGNAL_GALILEO_E1_C |\
        EP_GNSS_SIGNAL_GALILEO_E5A_Q | EP_GNSS_SIGNAL_GALILIEO_E5B_Q |\
        EP_GNSS_SIGNAL_BEIDOU_B1_I | EP_GNSS_SIGNAL_BEIDOU_B1_C | EP_GNSS_SIGNAL_BEIDOU_B2_I |\
        EP_GNSS_SIGNAL_BEIDOU_B2A_I) 

const struct EPCallbacks *pEpCbs = NULL;
struct EPInterface epInterface = {};
static epSubscriptionInfo subscriptionMsg = {};

extern LOG_CONFIG log_config;
#ifdef __cplusplus
extern "C" {
#endif

void reportToQXDM(const char * data, int length)
{
    if (NULL != pEpCbs &&
	    NULL != pEpCbs->epReportLogMessageCb) {
        pEpCbs->epReportLogMessageCb(1,
                                 EP_LOG_LEVEL_ERROR,
                                 EP_LOG_TYPE_DEBUG,
                                 length,
                                 (const uint8_t*)data);
    }
}

#ifdef __cplusplus
}
#endif

void ePSendEnginePluginError (epError error)
{
    //LOG_INFO("PPE, Received EP Error:%d\n", error);
    /* Copy input data to local structures and process it from separate thread context */
}

bool ePSessionContrlCommand(const epCommandRequest *cmdParameters)
{
    /* Copy input data to local structures and process it from separate thread context */
    return 1;
}

void ePProvideGnssEphemeris(const epGnssEphemerisReport *ephemerisReport)
{
    /**/
    //LOG_DEBUG("PPE, Received EP Ephemeris data Constellation Mask:%hu, GPS week:%hu,ms:%lu\n", \
    //          ephemerisReport->gnssConstellation, ephemerisReport->gpsSystemTime.systemWeek, ephemerisReport->gpsSystemTime.systemMsec);
    std::unique_lock<std::mutex> lk(mtx_gnss);
    while(!isTerminated){
        if(rb_get_space_free(gnssRingBuffer) < (EPHEMERIS_DATA_SIZE + DATA_HEADER_LENGTH)){
            cv_gnss_w.wait(lk);
            continue;
        }
        memcpy(&DATA_HEADER[3], &EPHEMERIS_DATA_SIZE, sizeof(uint16_t));
        memcpy(&DATA_HEADER[5], &DATA_EPHEMERIS, sizeof(uint16_t));
        
        int writenSize = rb_write(gnssRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
        writenSize = rb_write(gnssRingBuffer, (char*) ephemerisReport, EPHEMERIS_DATA_SIZE);

        break;
    }
    lk.unlock();
    cv_gnss_r.notify_all();
    /**/
    /* Copy input data to local structures and process it from separate thread context */
}

void ePProvideGnssSvMeasurement(const epGnssMeasurementReport *msrReport)
{
    /**/
    //LOG_INFO("PPE,Receive Measurement week:%hu,ms:%lu,sats:%hu\n", msrReport->gpsSystemTime.systemWeek, msrReport->gpsSystemTime.systemMsec, msrReport->numMeas);
    LOG_DEBUG("PPE,Receive Measurement week:%hu,ms:%lu,obs:%hu\n", msrReport->gpsSystemTime.systemWeek, msrReport->gpsSystemTime.systemMsec, msrReport->numMeas);

    if(hasGotObs){
        if((obsNum - msrReport->numMeas) >=  OBS_NUM_MAX_LEAP){
            LOG_ERROR("PPE,Skip the Measurement week:%hu,ms:%lu,obs:%hu,last obs:%hu\n", msrReport->gpsSystemTime.systemWeek, msrReport->gpsSystemTime.systemMsec, msrReport->numMeas, obsNum );
            return;
        }
    }

    std::unique_lock<std::mutex> lk(mtx_gnss);
    while(!isTerminated){
        if(rb_get_space_free(gnssRingBuffer) < (MEASUREMENT_DATA_SIZE + DATA_HEADER_LENGTH)){
            cv_gnss_w.wait(lk);
            continue;
        }
        memcpy(&DATA_HEADER[3], &MEASUREMENT_DATA_SIZE, sizeof(uint16_t));
        memcpy(&DATA_HEADER[5], &DATA_MEASUREMENT, sizeof(uint16_t));

        int writenSize = rb_write(gnssRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
        writenSize = rb_write(gnssRingBuffer, (char*) msrReport, MEASUREMENT_DATA_SIZE);
    
        break;
    }
    obsNum =  msrReport->numMeas;
    hasGotObs = true;

    lk.unlock();
    cv_gnss_r.notify_all();
    /**/

    /* Copy input data to local structures and process it from separate thread context */
}

void ePProvidePosition(const epPVTReport *positionReport)
{
    /**/
    LOG_DEBUG("PPE, Receive EP PVTReport data:%llu\n", positionReport->utcTimestampMs);
    std::unique_lock<std::mutex> lk_cors(mtx_cors);
    while(!isTerminated){
        if(rb_get_space_free(corsRingBuffer) < (PVTREPORT_DATA_SIZE + DATA_HEADER_LENGTH)){
            cv_cors_w.wait(lk_cors);
            continue;
        }
        memcpy(&DATA_HEADER[3], &PVTREPORT_DATA_SIZE, sizeof(uint16_t));
        memcpy(&DATA_HEADER[5], &DATA_PVTREPORT, sizeof(uint16_t));

        int writenSize = rb_write(corsRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
        writenSize = rb_write(corsRingBuffer, (char*) positionReport, PVTREPORT_DATA_SIZE);
            
        break;
    }

    lk_cors.unlock();
    cv_cors_r.notify_all();
    /**/
    /* Copy input data to local structures and process it from separate thread context */
}

/*9628-none
void  ePProvideCorrectionData(const epCorrectionStreamData *streamData)
{
}

void  ePProvideKlobucharIonoModel (const epKlobucharIonoModel *ionoData)
{
}

void  ePProvideGloAdditionalParam (const epGloAdditionalParameter *additionalData)
{
}
*
*/

void ePProvideSvPolynomial(const epGnssSvPolynomial *svPolynomial)
{
    //log_msg("PPE Debug,%s::%s:%d \n", __FILE__, __func__, __LINE__);
    /* Copy input data to local structures and process it from separate thread context */
}

void epProvideConnectionStatus(epConnectionStatus connStatus)
{
    //log_msg("PPE Debug,%s::%s:%d, epConnectionStatus:%d \n", __FILE__, __func__, __LINE__, connStatus);
    //LOG_INFO("PPE, Received EP Connection Status:%d\n", connStatus);
}

void epProvideFeatureStatus(epFeatureStatus status, epLicenseType licenseType)
{
    //log_msg("PPE Debug,%s::%s:%d, epFeatureStatus:%d,epLicenseType:%d\n", __FILE__, __func__, __LINE__, status, licenseType);
    //LOG_INFO("PPE, Receive FeatureStatus:%d,epLicenseType:%d\n", status, licenseType);
}

bool ePRequestVersionInfo(uint8_t *engineVersion, size_t maxBuffSizeVer,
        uint8_t *engineId, size_t maxBuffSizeId)
{
    bool returnValue = true;
    if ((NULL != engineVersion) && (NULL != engineId)) {
	memset(engineVersion, 0x00, maxBuffSizeVer);
        *engineVersion = '1';
	memset(engineId, 0x00, maxBuffSizeId);
        *engineId = '1';
    } else {
        returnValue = false;
    }
    return returnValue;
}

/**********************************************************************************************************************************************
*
*CMCC   Correction Communications
*
**********************************************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void onReceiveCMCCRtcmData(void *data, int length) {
    /**/
    LOG_DEBUG("Task:0X%X, Receive CMCC RTCM messages len:%d\n", (uint32_t)pthread_self(), length); 
    std::unique_lock<std::mutex> lk(mtx_cors);
    while(!isTerminated){
        if(rb_get_space_free(corsRingBuffer) < (length+DATA_HEADER_LENGTH)){
            cv_cors_w.wait(lk);
            continue;
        }
        uint16_t data_length = (uint16_t)length;
        memcpy(&DATA_HEADER[3], &data_length, sizeof(uint16_t));
        memcpy(&DATA_HEADER[5], &DATA_RTCM, sizeof(uint16_t));
 
        int writenSize = rb_write(corsRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
        writenSize = rb_write(corsRingBuffer, (char*) data, length);
        
        break;
    }

    lk.unlock();
    cv_cors_r.notify_all();
 
    /**/
}

void onReceiveCMCCStatusData(int status) {
    /**/
    LOG_DEBUG("PPE, Receive Correct status:%\n", status);
    std::unique_lock<std::mutex> lk(mtx_cors);
    while(!isTerminated){
        if(rb_get_space_free(corsRingBuffer) < (RTCM_STATUS_DATA_SIZE + DATA_HEADER_LENGTH)){
            cv_cors_w.wait(lk);
            continue;
        }
        memcpy(&DATA_HEADER[3], &RTCM_STATUS_DATA_SIZE, sizeof(uint16_t));
        memcpy(&DATA_HEADER[5], &DATA_RTCM_STATUS, sizeof(uint16_t));
 
        int writenSize = rb_write(corsRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
        writenSize = rb_write(corsRingBuffer, (char*)&status, RTCM_STATUS_DATA_SIZE);

        break;
    }

    lk.unlock();
    cv_cors_r.notify_all();
    /**/
}

RtcmDataResponse   rtcm_data = {onReceiveCMCCRtcmData};
RtcmStatusResponse status_data = {onReceiveCMCCStatusData};

#include <dlfcn.h>

typedef void (*pStartSdk)(RtcmDataResponse*, RtcmStatusResponse*);
typedef void (*pStopSdk)();
typedef int (*pSendGGA)(const char*, const int);
typedef void (*pSetRtcmUserInfo)(const char* user,const char* pwd);
typedef void (*pSetSourceID)(const char* source_id);

#ifdef __cplusplus
}
#endif


#define  FILE_NAME_MAX_SIZE     256
typedef struct correct_context_{
    char    _ld_lib_name[FILE_NAME_MAX_SIZE];
    char    _server_ip[32];
    uint16_t _server_port;
    char    _mount_code[32];
    char    _user[12];
    char    _passwd[8];

    void   *_servHandler;
    pStartSdk  _startSdk;
    pStopSdk   _stopSdk;
    pSendGGA   _sendGGA;
    pSetRtcmUserInfo _setRtcmUserInfo;
    pSetSourceID _setSourceID;
}CORRECT_CONTEXT;
#define  CORRECT_CONTEXT_SIZE     sizeof(CORRECT_CONTEXT)

#define  CORRECT_SERVICE_LIB     "/usr/lib/libntripclient-linux-gnueabi.so"

static int  handler_serv_conf(void* user, const char* section, const char* name,
                 const char* value)
{
    CORRECT_CONTEXT* pconfig = (CORRECT_CONTEXT*)user;

    if(MATCH("SERVICE", "CORRECT_LIB")) {
        snprintf(pconfig->_ld_lib_name, FILE_NAME_MAX_SIZE, "%s", value);
    }else if (MATCH("SERVICE", "CORRECT_SERVER_IP")) {
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

CORRECT_CONTEXT correctServ_context;

void start_correctServ(const char *config)
{
    memset((void*)&correctServ_context, 0x00, CORRECT_CONTEXT_SIZE);
    if (ini_parse(config, handler_serv_conf, &correctServ_context) < 0) {
        LOG_ERROR("Can't load log config:%s\n", config);
        snprintf(correctServ_context._ld_lib_name, FILE_NAME_MAX_SIZE, "%s", CORRECT_SERVICE_LIB);
        //return ;
    }
    LOG_INFO("PPE, loading Correct lib:%s", correctServ_context._ld_lib_name);

    if(strlen(correctServ_context._ld_lib_name)){
        char *error; 
      
        correctServ_context._servHandler =
                    dlopen(correctServ_context._ld_lib_name, RTLD_LAZY);  
        if (!correctServ_context._servHandler) {  
            LOG_ERROR( "%s ", dlerror());  
            return ;
        }  
          
        correctServ_context._startSdk = 
            (pStartSdk)dlsym(correctServ_context._servHandler, "startSdk");  
        if ((error = dlerror()) != NULL)  {  
            //dlclose(correctServ_context._servHandler);  
            //correctServ_context._servHandler = NULL;
            LOG_ERROR( "%s ", error);  
            //return ;
        }  
        
        correctServ_context._stopSdk = 
            (pStopSdk)dlsym(correctServ_context._servHandler, "stopSdk");  
        if ((error = dlerror()) != NULL)  {  
            //dlclose(correctServ_context._servHandler);  
            //correctServ_context._servHandler = NULL;
            LOG_ERROR( "%s ", error);  
            //return ;
        }  
        
        correctServ_context._sendGGA = 
            (pSendGGA)dlsym(correctServ_context._servHandler, "sendGGA");  
        if ((error = dlerror()) != NULL)  {  
            //dlclose(correctServ_context._servHandler);  
            //correctServ_context._servHandler = NULL;
            LOG_ERROR( "%s ", error);  
            //return ;
        } 

        correctServ_context._setRtcmUserInfo = 
            (pSetRtcmUserInfo)dlsym(correctServ_context._servHandler, "setRtcmUserInfo");  
        if ((error = dlerror()) != NULL)  {  
            LOG_ERROR( "%s ", error);  
        }  
        
        correctServ_context._setSourceID = 
            (pSetSourceID)dlsym(correctServ_context._servHandler, "setSourceID");  
        if ((error = dlerror()) != NULL)  {  
            LOG_ERROR( "%s ", error);  
        }  
 
    }
    if(correctServ_context._servHandler &&
        correctServ_context._startSdk){
        correctServ_context._startSdk(&rtcm_data, &status_data);
        LOG_INFO("PPE, Starting Correct LIB:%s\n", correctServ_context._ld_lib_name);
    }

    if(correctServ_context._servHandler &&
        correctServ_context._setRtcmUserInfo){
        correctServ_context._setRtcmUserInfo(correctServ_context._user, correctServ_context._passwd);
        LOG_DEBUG("PPE, Starting Correct LIB with User:%s, Passwd:%s\n", correctServ_context._user, correctServ_context._passwd);
    }
    
    if(correctServ_context._servHandler &&
        correctServ_context._setSourceID){
        correctServ_context._setSourceID(correctServ_context._mount_code);
        LOG_DEBUG("PPE, Starting Correct LIB with MountPoint:%s\n", correctServ_context._mount_code);
    }
 
    LOG_INFO("PPE, Finish loading Correct LIB\n");
    return ;
}

int SendSevGGA(const char *ggaRawBuf, const int length)
{
    if(correctServ_context._servHandler &&
        correctServ_context._sendGGA){
        LOG_DEBUG("PPE, Send Correct Request:%s\n", ggaRawBuf);
        //LOG_INFO("PPE, Send Correct Request\n");
        return correctServ_context._sendGGA(ggaRawBuf, length);
    }

    return 0;
}

void stop_correctServ()
{
    if(correctServ_context._servHandler &&
        correctServ_context._stopSdk){
        correctServ_context._stopSdk();
        dlclose(correctServ_context._servHandler);  
    }

    return ;
}
/**/

/**********************************************************************************************************************************************
*
*PPE init and entrance
*
**********************************************************************************************************************************************/
static void ep_init()
{
    if(log_config._rtk_eph_type){
        subscriptionMsg.flags = (EP_SUBSCRIPTION_PVT_1HZ_REPORT | \
                             EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT | \
                             EP_SUBSCRIPTION_EPHEMERIS_REPORT);
        
        subscriptionMsg.subscriptionModMeasurement.constellationType = EP_GNSS_SUB_CONSTELLATION;
    }else{
        subscriptionMsg.flags = (EP_SUBSCRIPTION_PVT_1HZ_REPORT | \
                             EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT);


        subscriptionMsg.subscriptionModMeasurement.constellationType = EP_GNSS_SUB_CONSTELLATION;
    }
    
    memset(&epInterface, 0, sizeof(epInterface));
    epInterface.epInterfaceFileMajorVersion = EP_INTERFACE_FILE_MAJOR_VERSION;
    epInterface.epInterfaceFileMinorVersion = EP_INTERFACE_FILE_MINOR_VERSION;
    epInterface.epRequestVersionInfo = ePRequestVersionInfo;
    epInterface.epSessionContrlCommand = ePSessionContrlCommand;
    epInterface.epProvideGnssEphemeris = ePProvideGnssEphemeris;
    epInterface.epProvideGnssSvMeasurement = ePProvideGnssSvMeasurement;
    epInterface.epProvidePosition = ePProvidePosition;
    epInterface.epSendEnginePluginError = ePSendEnginePluginError;
    epInterface.epProvideSvPolynomial = ePProvideSvPolynomial;
    epInterface.epProvideFeatureStatus = epProvideFeatureStatus;  
    
}

//void RtkReport(const RTK_result *pRtkResult)
void RtkReport(const void *pResult)
{
    const RTK_result *pRtkResult = (const RTK_result *)pResult;
    LOG_DEBUG("PPE, Rtk Result,ResolutionTime:%.4f,Diff Time:%.4f,Status:%d,Sats:%d,x:%.4f,y:%.4f,z:%.4f\n", \
              pRtkResult->dRTK_Resolution_Time, pRtkResult->dRTK_Diff_Time, pRtkResult->bStatusFlag, pRtkResult->bSatNo,\
              pRtkResult->dRov_Pos_x, pRtkResult->dRov_Pos_y, pRtkResult->dRov_Pos_z);
 
    if(log_config._log_level >= PPE_LOG_LEVEL_DEBUG){
        char    buffer[1024] = {0};
        ppeout_gga(pRtkResult, week, buffer);
        LOG_DEBUG("PPE, %s", buffer);
        //ppeout_rmc(pRtkResult, week, buffer);
        //LOG_DEBUG("PPE, %s", buffer);
    }

    //convert RTK_result--->epPVTReport positionReport
    if (NULL != pEpCbs &&
	    NULL != pEpCbs->epReportPositionCb) {
        
        ppeout_rtk(pRtkResult, week, &posReport);
        
        pEpCbs->epReportPositionCb(&posReport);
    }
}

void parseMeasurement(const epGnssMeasurementReport *msrReport)
{
    obsqumcom(msrReport, &ppes);
 
    PushEpoch(0, &ppes.wobs);
}

void parseEphemeris(const epGnssEphemerisReport *ephemerisReport)
{
    ephcom_t ephcom = {0};
    uint16_t ret = (uint16_t)ppeephcvt(ephemerisReport, &ephcom);
    if(ret  > 0){
        if(ephcom.sys ==  EP_GNSS_CONSTELLATION_GLONASS){
            /*
            for(int i = 0; i < ret; ++i){
                PushGloEphemeris(&ephcom.ephInfo.ephglo[i]);
            }
            */
        }else{
            for(int i = 0; i < ret; ++i){
                PushEphemeris(&ephcom.ephInfo.ephggb[i]);
            }
        }
    }
}

void parsePVTReport(const epPVTReport* positionReport)
{
    int ret = epposout_gga(positionReport, ppes.gpggabuf);
    if(ret > 1){
        int len = strlen(ppes.gpggabuf);
        PushGGA(SOURCE_TYPE_ROVER, ppes.gpggabuf, len);
        if (SendSevGGA(ppes.gpggabuf, len) == -1){
            LOG_ERROR("PPE, Failed to send Correct Request:%s", ppes.gpggabuf);
        }
    }
}

#ifdef _LINK_SERIAL_PORT
extern SERIAL_CONFIG serial_config;
#endif

void CorsThreadFunc()
{

    //start_correctServ("/etc/sino.conf");

    char *readBuffer = new char[PVTREPORT_DATA_SIZE+DATA_HEADER_LENGTH];
    char GGA_BUF[256] = {0};
    while(!isTerminated){

        uint16_t data_length = 0;
        uint16_t data_type = 0;
        {
            std::unique_lock<std::mutex> lk(mtx_cors);
            if(rb_get_space_used(corsRingBuffer) <= 0){
                cv_cors_r.wait(lk);
            }

            int readSize = rb_read(corsRingBuffer, readBuffer, DATA_HEADER_LENGTH);
            memcpy((void*)&data_length, &readBuffer[3], sizeof(uint16_t));    
            memcpy((void*)&data_type, &readBuffer[5], sizeof(uint16_t));    
            
            readSize = rb_read(corsRingBuffer, &readBuffer[DATA_HEADER_LENGTH], data_length);
            if ( readSize > 0 ){
                lk.unlock();
                cv_cors_w.notify_all();
            
            }
        }
        if(data_length <= 0) continue;
        
        if(data_type == DATA_PVTREPORT){
            epPVTReport              posReport;
            memset(GGA_BUF, 0x00, sizeof(GGA_BUF));
            memcpy((void*)&posReport, &readBuffer[DATA_HEADER_LENGTH], data_length);
            LOG_DEBUG("PPE, Handling EP PVTReport data:%llu\n", posReport.utcTimestampMs);
            int ret = epposout_gga(&posReport, GGA_BUF);
            if(ret > 1){
                uint16_t len = (uint16_t)strlen(GGA_BUF);
                
                std::unique_lock<std::mutex> lk(mtx_gnss);
                while(true){
                    if(rb_get_space_free(gnssRingBuffer) <= 0 ){
                        cv_gnss_w.wait(lk);
                        continue;
                    }
                    memcpy(&DATA_HEADER[3], &len, sizeof(uint16_t));
                    memcpy(&DATA_HEADER[5], &DATA_GGA, sizeof(uint16_t));

                    int writenSize = rb_write(gnssRingBuffer, DATA_HEADER, DATA_HEADER_LENGTH);
                    writenSize = rb_write(gnssRingBuffer, (char*) GGA_BUF, len);
            
                    break;
                }

                lk.unlock();
                cv_gnss_r.notify_all();


                if (SendSevGGA(GGA_BUF, len) == -1){
                    LOG_ERROR("PPE, Failed to send Correct Request:%s", GGA_BUF);
                }
            }
        }else if(data_type == DATA_RTCM){
            write_data(RAW_DATA_OUTPUT_CORRECT, readBuffer, data_length + DATA_HEADER_LENGTH);
            LOG_DEBUG("PPE, Parse Correction data length:%d \n", data_length);
            PushRtcm(1, &readBuffer[DATA_HEADER_LENGTH], data_length);
        }else if(data_type == DATA_RTCM_STATUS){
            int status = -1;
            memcpy((void*)&status , &readBuffer[DATA_HEADER_LENGTH], data_length);
            LOG_DEBUG("PPE, Handled Correction Status:%d\n", status);
        }

    }
    delete[] readBuffer;
}


void GNSSThreadFunc(const struct EPCallbacks *callbacks)
{
#ifdef _LINK_SERIAL_PORT
    init_serialport("/etc/sino.conf");
#endif

    init_ppe(&ppes);
    rtk_init(RtkReport, RTK_GNSS_CONSTELLATION_GPS| RTK_GNSS_CONSTELLATION_GALILEO| RTK_GNSS_CONSTELLATION_BEIDOU, RTK_GNSS_SOLUTION_SINO);
    gnssRingBuffer = rb_create(GNSS_DATA_RING_BUF_SIZE);

    char *readBuffer = new char[MEASUREMENT_DATA_SIZE+DATA_HEADER_LENGTH];

    corsRingBuffer = rb_create(CORS_DATA_RING_BUF_SIZE);
    
    //dynamically load
    start_correctServ("/etc/sino.conf");

    corsThr = std::thread(CorsThreadFunc);
    corsThr.detach();


    if ((NULL != callbacks) && (NULL != callbacks->epRequestReportSubscription)) {
        LOG_INFO("PPE, subscrible data to EP Flags:%lld, Constellation:%hu.\n", subscriptionMsg.flags, subscriptionMsg.subscriptionModMeasurementNHz.constellationType);
        callbacks->epRequestReportSubscription(&subscriptionMsg);
    }
    
    LOG_INFO("PPE, Begin to handle data.\n");
    char GGA_BUF[256] = {0};
    while(!isTerminated){
        uint16_t data_length = 0;
        uint16_t data_type = 0;
        {
            std::unique_lock<std::mutex> lk(mtx_gnss);
            if(rb_get_space_used(gnssRingBuffer) <= 0){
                cv_gnss_r.wait(lk);
            }

            int readSize = rb_read(gnssRingBuffer, readBuffer, DATA_HEADER_LENGTH);
            memcpy((void*)&data_length, &readBuffer[3], sizeof(uint16_t));    
            memcpy((void*)&data_type, &readBuffer[5], sizeof(uint16_t));    
            
            readSize = rb_read(gnssRingBuffer, &readBuffer[DATA_HEADER_LENGTH], data_length);
            if ( readSize > 0 ){
                lk.unlock();
                cv_gnss_w.notify_all();
            }
        }
        if(data_length <= 0) continue;
        
        /**/
        if(data_type == DATA_MEASUREMENT){
            write_data(RAW_DATA_OUTPUT_QUALCOM, readBuffer, data_length + DATA_HEADER_LENGTH);
            memcpy((void*)&msrReport, &readBuffer[DATA_HEADER_LENGTH], data_length);
            LOG_DEBUG("PPE,Parse Measurement week:%hu,ms:%lu,sats:%hu\n", msrReport.gpsSystemTime.systemWeek, msrReport.gpsSystemTime.systemMsec, msrReport.numMeas);
            parseMeasurement(&msrReport);
        }else if(data_type == DATA_EPHEMERIS){
            write_data(RAW_DATA_OUTPUT_QUALCOM, readBuffer, data_length + DATA_HEADER_LENGTH);
            memcpy((void*)&ephemReport, &readBuffer[DATA_HEADER_LENGTH], data_length);
            parseEphemeris(&ephemReport);
        }else if(data_type == DATA_GGA){//cors thread handle PVTReport and then convert to GGA
            memset(GGA_BUF, 0x00, sizeof(GGA_BUF));
            memcpy((void*)GGA_BUF, &readBuffer[DATA_HEADER_LENGTH], data_length);
            int len = strlen(GGA_BUF);
            PushGGA(SOURCE_TYPE_ROVER, GGA_BUF, len);
        /**pv report ,another thread deal with it*/
        }/**else if(data_type == DATA_PVTREPORT){
            memset(GGA_BUF, 0x00, sizeof(GGA_BUF));
            memcpy((void*)&posReport, &readBuffer[DATA_HEADER_LENGTH], data_length);
            LOG_DEBUG("PPE, Handling EP PVTReport data:%llu\n", posReport.utcTimestampMs);
            int ret = epposout_gga(&posReport, GGA_BUF);
            if(ret > 1){
                int len = strlen(GGA_BUF);
                PushGGA(SOURCE_TYPE_ROVER, GGA_BUF, len);
            }
 
        }else if(data_type == DATA_RTCM){
            write_data(RAW_DATA_OUTPUT_CORRECT, readBuffer, data_length + DATA_HEADER_LENGTH);
            LOG_DEBUG("PPE, Parse Correction data length:%d \n", data_length);
            PushRtcm(1, &readBuffer[DATA_HEADER_LENGTH], data_length);
        }else if(data_type == DATA_RTCM_STATUS){
            int status = -1;
            memcpy((void*)&status , &readBuffer[DATA_HEADER_LENGTH], data_length);
            LOG_DEBUG("PPE, Handled Correction Status:%d\n", status);
        }*/
    }
   
}

const struct EPInterface* get_ep_interface(const struct EPCallbacks* callbacks)
{
    init_logger("/etc/sino.conf", reportToQXDM, NULL);

    ep_init();
    
    pEpCbs = callbacks;

    ppeThr = std::thread(GNSSThreadFunc, callbacks);
    ppeThr.detach();

    return &epInterface;
}

