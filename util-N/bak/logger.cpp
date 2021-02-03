#include <stdarg.h>
#include <stdlib.h>

#include "logger.h"
#ifdef _LINK_SERIAL_PORT
#include "serialport.h"
#endif

LOG_CONFIG log_config;

#ifdef __cplusplus
extern "C" {
#endif

static int  handler_log_conf(void* user, const char* section, const char* name,
                 const char* value)
{
    LOG_CONFIG* pconfig = (LOG_CONFIG*)user;

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
            log_config._logger = fopen(log_config._log_file_name, "ab+");
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
static void private_logger_default_conf()
{
    log_config._log_level = PPE_LOG_LEVEL_ERROR;
    log_config._log_type = PPE_LOG_TYPE_FILE;
    snprintf(log_config._log_file_name, LOG_FILE_NAME_MAX_SIZE, "%s", LOG_DEFAULT_FILE_NAME);
    log_config._log_file_size = LOG_DEFAULT_FILE_SIZE;
    log_config._log_buf_size = LOG_BUF_MAX_SIZE;
}

void init_logger(const char *config, pExtLogReport extLog)
{
    memset((void*)&log_config, 0x00, LOG_CONFIG_SIZE);
    if(extLog){
        log_config._pExtLogReport = extLog;
        log_config._log_level = PPE_LOG_LEVEL_ERROR;
        log_config._log_type = PPE_LOG_TYPE_QXDM;
    }

    if (ini_parse(config, handler_log_conf, &log_config) < 0) {
        private_logger_default_conf();
        //LOG_ERROR("Can't load log config:%s\n", config);
        //return ;
    }

    uint16_t  bufSize =  log_config._log_buf_size >  LOG_BUF_MAX_SIZE? LOG_BUF_MAX_SIZE: log_config._log_buf_size;
    if(!log_config._log_buf){
        //delete[] log_config._log_buf;
        log_config._log_buf = new char[bufSize];
    }


    private_init_logger();
    return ;
}

#include  <time.h>
static int log_time(char *buff, int length) {
//    static char cur_system_time[24] = { 0 };
    time_t timep;
    //struct tm *p;
    struct tm tmbuf;

    time(&timep);
    struct tm *p = localtime_r(&timep, &tmbuf);
    if (p == NULL) {
        return 0;
    }
    return snprintf(buff, length, "[%02d-%02d %02d:%02d:%02d]", p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);

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

    if ((log_config._log_type&PPE_LOG_TYPE_QXDM) && (NULL != log_config._pExtLogReport)) {
        log_config._pExtLogReport(log_config._log_buf, pos + 1);
    }
    
    if (log_config._log_type&PPE_LOG_TYPE_FILE){
        if (!log_config._logger) {
            private_init_logger();
        }
        if (NULL != log_config._logger) {
            fwrite(log_config._log_buf, sizeof(char), pos + 1, log_config._logger);
            fflush(log_config._logger);
        }
    }
    
    if (log_config._log_type&PPE_LOG_TYPE_STDOUT) {
        printf("%s\n", log_config._log_buf);
    }
}

//void log_bin(uint16_t loglevel, const char * data, const int length)
static void log_bin(const char * data, const int length)
{
    
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_FILE){
        if (!log_config._datlogger) {
            private_init_dat_logger();
        }
        if (NULL != log_config._datlogger) {
            fwrite(data, sizeof(char), length, log_config._datlogger);
            fflush(log_config._datlogger);
        }
    }
}

void write_data(uint8_t dat_type, const char * data, const int length)
{
    if(!(dat_type&log_config._dat_type)){
        return;
    }
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_FILE){
        log_bin(data, length);
    }
    #ifdef _LINK_SERIAL_PORT
    if (log_config._dat_output_type&PPE_DATA_OUTPUT_SERIAL){
        write_serialport(data, (uint16_t)length);
    }
    #endif
}

void dinit_logger()
{
    if(log_config._logger){
        fclose(log_config._logger);
    }
    if(log_config._log_buf){
        delete[] log_config._log_buf;
    }
    if(log_config._datlogger){
        fclose(log_config._datlogger);
    }
}
