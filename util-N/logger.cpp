#include <stdarg.h>
#include <stdlib.h>

 
#include "logger.h"

#ifdef _LINK_SERIAL_PORT
#include "serialport.h"
#endif

#ifdef _EC100Y_PLATFORM
extern "C" {
#include <ql_uart.h>
}
#endif
//multi thread
#ifdef _DAT_LOG_MT
#include "RingBuffer.h"
static volatile bool isTerminated = false;

#ifdef _EC100Y_PLATFORM

ringbuffer datLogRingBuffer; 
#define DAT_LOG_READ_BUFFER_MAX_SIZE    2048
static char readBuffer[DAT_LOG_READ_BUFFER_MAX_SIZE];


extern "C" {
#include <ql_rtos.h>
//#include <ql_uart.h>
}
ql_task_t datLogThr= NULL;
ql_sem_t  mtx_datLog = NULL;

#else
#include <thread>
#include <mutex>
#include <condition_variable>

#define DAT_LOG_RING_BUFFER_MAX_SIZE    1024*1024*15
#define DAT_LOG_READ_BUFFER_MAX_SIZE    1024*1024
ringbuffer *datLogRingBuffer = nullptr;
std::thread datLogThr;
std::mutex mtx_datLog;
std::condition_variable cv_datLog_r;
std::condition_variable cv_datLog_w;
#endif
#endif

LOG_CONFIG log_config;


#ifdef __cplusplus
extern "C" {
#endif

static int  handler_log_conf(void* user, const char* section, const char* name,
                 const char* value)
{
    LOG_CONFIG* pconfig = (LOG_CONFIG*)user;
    pconfig->_rtk_smooth_cnt = 1000;
    if(MATCH("LOG", "LOG_LEVEL")) {
        pconfig->_log_level = atoi(value);
    }else if (MATCH("LOG", "LOG_DEST")) {
        pconfig->_log_type = atoi(value);
    }else if (MATCH("LOG", "LOG_FILE")) {
        snprintf(pconfig->_log_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", value);
    }else if (MATCH("LOG", "LOG_BUFF_SIZE")) {
        pconfig->_log_buf_size = atoi(value);
    }else if (MATCH("LOG", "LOG_FILE_SIZE")) {
        pconfig->_log_file_size = atoi(value);
    }else if (MATCH("DATA", "DATA_FILE")) {
        snprintf(pconfig->_dat_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", value);
    }else if (MATCH("DATA", "DATA_OUTPUT")) {
        pconfig->_dat_output_type = atoi(value);
    }else if (MATCH("DATA", "DATA_TYPE")) {
        pconfig->_dat_type = atoi(value);
    }else if (MATCH("DATA", "RTK_DATA_TYPE")) {
        pconfig->_rtk_dat_type = atoi(value);
    }else if (MATCH("DATA", "RTK_PSR_SMOOTH")) {
        pconfig->_rtk_psr_type = atoi(value);
    }else if (MATCH("DATA", "RTK_SMOOTH_COUNT")) {
        pconfig->_rtk_smooth_cnt = atoi(value);
    }else if (MATCH("DATA", "RTK_EPH_TYPE")) {
        pconfig->_rtk_eph_type = atoi(value);
    }else{
        return 0;  /* unknown section/name, error */
                 
    }
    return 1;
}
#ifdef __cplusplus
}
#endif


static void private_init_logger()
{
    if((log_config._log_type&PPE_LOG_TYPE_FILE) && 
       strlen(log_config._log_file_name)){
        if(!log_config._logger){
            log_config._logger = fopen(log_config._log_file_name, "a+");
            //fclose(log_config._logger);
        }
    }
}

static void private_init_dat_logger()
{
    if((log_config._dat_output_type&PPE_DATA_OUTPUT_FILE) && 
        strlen(log_config._dat_file_name)){
        if(!log_config._datlogger){
            log_config._datlogger = fopen(log_config._dat_file_name, "ab+");
            //fclose(log_config._logger);
        }
    }
}


static void log_bin(const char * data, const int length)
{
#ifdef _EC100Y_PLATFORM //EC100Y only supports serial port
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_SERIAL){
        ql_uart_write(QL_DEBUG_UART_PORT, (unsigned char *)data, length);
    }
#else
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_FILE){
        if (!log_config._datlogger) {
            private_init_dat_logger();
        }
        if (NULL != log_config._datlogger) {
            fwrite(data, sizeof(char), length, log_config._datlogger);
            fflush(log_config._datlogger);
        }
    }
#endif
}

#ifdef _DAT_LOG_MT
#ifdef _EC100Y_PLATFORM

static void DatLogThreadFunc(void*)
{

    while(!isTerminated){
        int readSize = 0;
                    
        ql_rtos_semaphore_wait(mtx_datLog, QL_WAIT_FOREVER);
 
        if(rb_get_space_used(&datLogRingBuffer) <= 0){
            ql_rtos_semaphore_release(mtx_datLog);
            ql_rtos_task_sleep_ms(100);
            continue;
        }

        readSize = rb_read(&datLogRingBuffer, readBuffer, DAT_LOG_READ_BUFFER_MAX_SIZE);
            
        if( readSize <= 0 ){
            ql_rtos_semaphore_release(mtx_datLog);
            ql_rtos_task_sleep_ms(100);
            continue;
        } 
        
        if (log_config._dat_output_type&PPE_DATA_OUTPUT_BOTH){
            log_bin(readBuffer, readSize);
        }
        ql_rtos_semaphore_release(mtx_datLog);
    }
}

static void private_init_dat_logger_thread()
{
    if(log_config._dat_output_type&PPE_DATA_OUTPUT_BOTH){
        rb_init(&datLogRingBuffer);
        ql_rtos_semaphore_create(&mtx_datLog, 1);
        ql_rtos_task_create(&datLogThr, 20480, 10, (char*)"dat Log Task", DatLogThreadFunc, NULL);
    }
}
#else
static void DatLogThreadFunc()
{

    char *readBuffer = new char[DAT_LOG_READ_BUFFER_MAX_SIZE];

    while(!isTerminated){
        int readSize = 0;
        {
            std::unique_lock<std::mutex> lk(mtx_datLog);
            if(rb_get_space_used(datLogRingBuffer) <= 0){
                cv_datLog_r.wait(lk);
            }

            readSize = rb_read(datLogRingBuffer, readBuffer, DAT_LOG_READ_BUFFER_MAX_SIZE);
            
            if ( readSize > 0 ){
                lk.unlock();
                //cv_datLog.notify_one();
                cv_datLog_w.notify_all();
            }
        }

        if( readSize <= 0 ) continue;
        
        if (log_config._dat_output_type&PPE_DATA_OUTPUT_FILE){
            log_bin(readBuffer, readSize);
        }
        /*       
        #ifdef _LINK_SERIAL_PORT
        if (log_config._dat_output_type&PPE_DATA_OUTPUT_SERIAL){
            write_serialport(data, (uint16_t)length);
        }
        #endif
        */
    }
}

static void private_init_dat_logger_thread()
{
    if(log_config._dat_output_type&PPE_DATA_OUTPUT_BOTH){

        datLogRingBuffer = rb_create(DAT_LOG_RING_BUFFER_MAX_SIZE);

        datLogThr = std::thread(DatLogThreadFunc);
        datLogThr.detach();

    }
}
#endif
#endif

static void private_logger_default_conf()
{
    log_config._log_level = PPE_LOG_LEVEL_ERROR;
    log_config._log_type = PPE_LOG_TYPE_FILE;
    //snprintf(log_config._log_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", LOG_DEFAULT_FILE_NAME);
    snprintf(log_config._log_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", D_LOG_FILE_UNDER_ROOT);
    log_config._log_file_size = LOG_DEFAULT_FILE_SIZE;
    log_config._log_buf_size = LOG_BUF_MAX_SIZE;
}

void init_logger(const char *config, pExtLogReport extLog, LOG_CONFIG* pconfig)
{
    memset((void*)&log_config, 0x00, LOG_CONFIG_SIZE);
    if(extLog){
        log_config._pExtLogReport = extLog;
        log_config._log_level = PPE_LOG_LEVEL_ERROR;
        log_config._log_type = PPE_LOG_TYPE_QXDM;
    }

    if(pconfig){
        log_config._log_level = pconfig->_log_level;
        log_config._log_type = pconfig->_log_type;
        snprintf(log_config._log_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", pconfig->_log_file_name);
        //log_config._log_buf_size = pconfig->_log_buf_size;
        log_config._log_buf_size = LOG_BUF_MAX_SIZE;
        log_config._log_file_size = pconfig->_log_file_size;
        snprintf(log_config._dat_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", pconfig->_dat_file_name);
        log_config._dat_output_type = pconfig->_dat_output_type;
        log_config._dat_type = pconfig->_dat_type;
        log_config._rtk_dat_type = pconfig->_rtk_dat_type;
    }else if (ini_parse(config, handler_log_conf, &log_config) < 0) {
        private_logger_default_conf();
        //LOG_ERROR("Can't load log config:%s\n", config);
        //return ;
    }

    /*
    uint16_t  bufSize =  log_config._log_buf_size >  LOG_BUF_MAX_SIZE? LOG_BUF_MAX_SIZE: log_config._log_buf_size;
    if(!log_config._log_buf){
        //delete[] log_config._log_buf;
        //log_config._log_buf = new char[bufSize];
        log_config._log_buf = calloc(1, bufSize);
    }
    */

    private_init_logger();
#ifdef _DAT_LOG_MT
    private_init_dat_logger_thread();
#endif
    return ;
}

#include  <time.h>
static int log_time(char *buff, int length) {
//    static char cur_system_time[24] = { 0 };
/**/
    time_t timep;

    time(&timep);
#ifdef _EC100Y_PLATFORM
    struct tm *p = gmtime(&timep);//多线程有问题
#else
    struct tm tmbuf;
    struct tm *p = localtime_r(&timep, &tmbuf);
#endif
    if (p == NULL) {
        return 0;
    }
    return snprintf(buff, length, "[%02d-%02d %02d:%02d:%02d]", p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
/**/
    /*
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    size_t pos = strftime(buff, length, "%D %T", gmtime(&ts.tv_sec));
    return pos + snprintf(&buff[pos], length - pos, ".%03d UTC ", buff, (int)(ts.tv_nsec/1000000));
    */
    //return pos + snprintf(&buff[pos], length - pos, ".%09llu UTC ", buff, ts.tv_nsec);
}


static int log_level(uint16_t loglevel, char *buff, int length) {
    switch(loglevel){
    case PPE_LOG_LEVEL_ERROR:
        return snprintf(buff, length, "[%s]", PPE_LOG_LEVEL_ERROR_STR);
    case PPE_LOG_LEVEL_WARNING:
        return snprintf(buff, length, "[%s]", PPE_LOG_LEVEL_WARNING_STR);
    case PPE_LOG_LEVEL_INFO:
        return snprintf(buff, length, "[%s]", PPE_LOG_LEVEL_INFO_STR);
    case PPE_LOG_LEVEL_DEBUG:
        return snprintf(buff, length, "[%s]", PPE_LOG_LEVEL_DEBUG_STR);
    case PPE_LOG_LEVEL_VERBOSE:
        return snprintf(buff, length, "[%s]", PPE_LOG_LEVEL_VERBOSE_STR);
    }
    return 0;
}

//now,mulithread need sync with system ...
void log_msg(uint16_t loglevel, const char * const format, ...)
{
    //if(!(loglevel&log_config._log_level)){
    if(loglevel > log_config._log_level){
        return;
    }

    va_list args;
    va_start(args, format);
    int formatResult = -1;
    int pos = 0;
    formatResult = log_time(log_config._log_buf, log_config._log_buf_size);
    if(formatResult < 0){
        formatResult = 0;
    }
    pos += formatResult;

    formatResult = log_level(loglevel, &log_config._log_buf[pos], log_config._log_buf_size - formatResult);
    if(formatResult < 0){
        formatResult = 0;
    }
    pos += formatResult;

    formatResult = vsnprintf(&log_config._log_buf[pos], log_config._log_buf_size, format, args);
    
    va_end(args);

    if(formatResult < 0){
        return;
    }
    pos += formatResult;

    /**2020-12-23 ,not report to EP*/
    /*
    if ((log_config._log_type&PPE_LOG_TYPE_QXDM) && (NULL != log_config._pExtLogReport)) {
        log_config._pExtLogReport(log_config._log_buf, pos + 1);
    }
    */
#ifdef _EC100Y_PLATFORM //EC100Y only supports serial port
    /*
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_SERIAL){
        ql_uart_write(QL_DEBUG_UART_PORT, (unsigned char *)log_config._log_buf, strlen(log_config._log_buf));
    }
    */
#else
 
    if (log_config._log_type&PPE_LOG_TYPE_FILE){
        if (!log_config._logger) {
            private_init_logger();
        }
        if (NULL != log_config._logger) {
            //fwrite(log_config._log_buf, sizeof(char), pos + 1, log_config._logger);
            fprintf(log_config._logger, "%s", log_config._log_buf);
            fflush(log_config._logger);
        }
    }
    
//    if (log_config._log_type&PPE_LOG_TYPE_STDOUT) {
//        printf("%s\n", log_config._log_buf);
//    }
#endif
    if (log_config._log_type&PPE_LOG_TYPE_STDOUT) {
        printf("%s\n", log_config._log_buf);
    }

}

void write_data(uint8_t dat_type, const char * data, const int length)
{
    if(!(dat_type&log_config._dat_type) || !(log_config._dat_output_type&PPE_DATA_OUTPUT_BOTH)){
        return;
    }

#ifdef _DAT_LOG_MT
#ifdef _EC100Y_PLATFORM
   ql_rtos_semaphore_wait(mtx_datLog, QL_WAIT_FOREVER);
#else
    std::unique_lock<std::mutex> lk(mtx_datLog);
#endif
    while(!isTerminated){
#ifdef _EC100Y_PLATFORM
        if(rb_get_space_free(&datLogRingBuffer) < length){
#else
        if(rb_get_space_free(datLogRingBuffer) < length){
#endif
#ifdef _EC100Y_PLATFORM
            ql_rtos_semaphore_release(mtx_datLog);
            ql_rtos_task_sleep_ms(100);

#else
            cv_datLog_w.wait(lk);
#endif
            continue;
        }
 
#ifdef _EC100Y_PLATFORM
        int writenSize = rb_write(&datLogRingBuffer, (char*)data, length);
#else
        int writenSize = rb_write(datLogRingBuffer, (char*)data, length);
#endif
        break;
    }

#ifdef _EC100Y_PLATFORM
   ql_rtos_semaphore_release(mtx_datLog);
#else
    lk.unlock();
    cv_datLog_r.notify_all();
#endif 
#else
    //if (log_config._dat_output_type&PPE_DATA_OUTPUT_FILE){
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_BOTH){
        log_bin(data, length);
    }
    #ifdef _LINK_SERIAL_PORT
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_SERIAL){
        write_serialport(data, (uint16_t)length);
    }
    #endif
#endif
}

void dinit_logger()
{
    if(log_config._logger){
        fclose(log_config._logger);
    }
    /*
    if(log_config._log_buf){
        //delete[] log_config._log_buf;
        free(log_config._log_buf);
    }
    */
    if(log_config._datlogger){
        fclose(log_config._datlogger);
    }
}
