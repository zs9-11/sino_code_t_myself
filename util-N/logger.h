#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ini.h"

#define  LOG_FILE_NAME_MAX_SIZE     256
//#define  LOG_BUF_MAX_SIZE           9216  
#define  LOG_BUF_MAX_SIZE           2048  
//#define  LOG_DEFAULT_FILE_NAME      "/usrdata/ppe.log"

#define LOG_FILE_NAME			"ppe.log"
#define DAT_FILE_NAME			"ppe_data.bin"
#define DIR_NAME			    "LOGS"

#define  LOG_DEFAULT_FILE_SIZE      10

#ifdef _EC100Y_PLATFORM

#define D_DISK				    "D:"
#define D_ROOT_PATH			    D_DISK "/"
#define D_LOG_FILE_UNDER_ROOT 	D_DISK "/" LOG_FILE_NAME
#define D_DAT_FILE_UNDER_ROOT 	D_DISK "/" DAT_FILE_NAME
#define D_DIR_PATH 			    D_DISK "/" DIR_NAME
#define D_LOG_FILE_UNDER_DIR	D_DIR_PATH "/" LOG_FILE_NAME
#define D_DAT_FILE_UNDER_DIR	D_DIR_PATH "/" DAT_FILE_NAME
#else

#define D_DISK				    "/tmp"
#define D_ROOT_PATH			    D_DISK "/"
#define D_LOG_FILE_UNDER_ROOT 	D_DISK "/" LOG_FILE_NAME
#define D_DAT_FILE_UNDER_ROOT 	D_DISK "/" DAT_FILE_NAME
#define D_DIR_PATH 			    D_DISK "/" DIR_NAME
#define D_LOG_FILE_UNDER_DIR	D_DIR_PATH "/" LOG_FILE_NAME
#define D_DAT_FILE_UNDER_DIR	D_DIR_PATH "/" DAT_FILE_NAME
#endif

/** Type of Log Level */
typedef uint16_t ppeLogLevelType;
/** Log Level is None */
#define PPE_LOG_LEVEL_NONE    ((ppeLogLevelType)0x0000)
/** Log Level is Error */
#define PPE_LOG_LEVEL_ERROR    ((ppeLogLevelType)0x0001)
#define PPE_LOG_LEVEL_ERROR_STR    "ERROR"
/** Log Level is Warning */
#define PPE_LOG_LEVEL_WARNING  ((ppeLogLevelType)0x0003)
#define PPE_LOG_LEVEL_WARNING_STR  "WARNING"
/** Log Level is Info */
#define PPE_LOG_LEVEL_INFO     ((ppeLogLevelType)0x0007)
#define PPE_LOG_LEVEL_INFO_STR     "INFO"
/** Log Level is Debug */
#define PPE_LOG_LEVEL_DEBUG    ((ppeLogLevelType)0x000F)
#define PPE_LOG_LEVEL_DEBUG_STR    "DEBUG"
/** Log Level is Verbose */
#define PPE_LOG_LEVEL_VERBOSE  ((ppeLogLevelType)0x001F)
#define PPE_LOG_LEVEL_VERBOSE_STR  "VERBOSE"

/** Type of Log type */
typedef uint8_t ppeLogOutType;
/** Log type is stdout*/
#define PPE_LOG_TYPE_NONE    ((ppeLogOutType)0x00)
/** Log type is stdout*/
#define PPE_LOG_TYPE_STDOUT    ((ppeLogOutType)0x01)
/** Log type is QXDM */
#define PPE_LOG_TYPE_QXDM      ((ppeLogOutType)0x02)
/** Log type is file */
#define PPE_LOG_TYPE_FILE      ((ppeLogOutType)0x04)
/** Log type is file */
#define PPE_LOG_TYPE_ALL       ((ppeLogOutType)0xFF)

/** data of output type */
typedef uint8_t ppeDatOutType;
/** data not out*/
#define PPE_DATA_OUTPUT_NONE   ((ppeDatOutType)0x00)
/** data out to serial*/
#define PPE_DATA_OUTPUT_SERIAL ((ppeDatOutType)0x01)
/** data out to File*/
#define PPE_DATA_OUTPUT_FILE   ((ppeDatOutType)0x02)
/** data out to File and serial*/
#define PPE_DATA_OUTPUT_BOTH   ((ppeDatOutType)0x0F)

/** Data of output type */
typedef uint8_t rawDataOutType;
#define RAW_DATA_OUTPUT_NONE   ((rawDataOutType)0x00)
/** qualcom  org data*/
#define RAW_DATA_OUTPUT_QUALCOM   ((rawDataOutType)0x01)
/** CORS correct  data */
#define RAW_DATA_OUTPUT_CORRECT   ((rawDataOutType)0x02)
/** sino internal data */
#define RAW_DATA_OUTPUT_SINOINTER ((rawDataOutType)0x04)
/** sino debug data */
#define RAW_DATA_OUTPUT_DEBUG ((rawDataOutType)0x08)

#define RAW_DATA_OUTPUT_ALL      ((rawDataOutType)0x0F)

/** rtk data type */
typedef uint8_t rtkDataType;
/** data M919*/
#define RTK_DATA_M919           ((rtkDataType)0x01)
/** data M916*/
#define RTK_DATA_M916           ((rtkDataType)0x02)
/** data M919 && M916*/
#define RTK_DATA_BOTH           ((rtkDataType)0x0F)


typedef void (*pExtLogReport)(const char *data, int length);

typedef struct log_config_{
    uint16_t     _log_level;
    uint8_t      _log_type;
    uint16_t     _log_buf_size;
    //char         *_log_buf;
    char         _log_buf[LOG_BUF_MAX_SIZE];
    uint16_t     _log_file_size;
    char         _log_file_name[LOG_FILE_NAME_MAX_SIZE];
    FILE        *_logger;
    uint8_t      _dat_output_type;
    uint8_t      _dat_type;
    char         _dat_file_name[LOG_FILE_NAME_MAX_SIZE];
    FILE        *_datlogger;
    uint8_t      _rtk_dat_type;
    uint16_t     _rtk_smooth_cnt;
    uint8_t      _rtk_psr_type;
    uint8_t      _rtk_eph_type;
    pExtLogReport _pExtLogReport;
}LOG_CONFIG;
#define LOG_CONFIG_SIZE  sizeof(LOG_CONFIG)

//now,mulithread need sync with system ...
void log_msg(uint16_t loglevel, const char * const format, ...);

#define  LOG_INFO(FMT, ...) \
                log_msg(PPE_LOG_LEVEL_INFO, FMT, ##__VA_ARGS__)

#define  LOG_WARNING(FMT, ...) \
                log_msg(PPE_LOG_LEVEL_WARNING, FMT, ##__VA_ARGS__)

#define  LOG_ERROR(FMT, ...) \
                log_msg(PPE_LOG_LEVEL_ERROR, FMT, ##__VA_ARGS__)

#define  LOG_DEBUG(FMT, ...) \
                log_msg(PPE_LOG_LEVEL_DEBUG, FMT, ##__VA_ARGS__)


void write_data(uint8_t dat_type, const char * data, const int length);
//void log_bin(const char * data, const int length);

void init_logger(const char *config, pExtLogReport extLog , LOG_CONFIG* pconfig );
 
void dinit_logger();

#ifdef __cplusplus
}
#endif

