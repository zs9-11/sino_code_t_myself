#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEV_NAME_MAX_SIZE   128

#define DEV_DEFAULT_USED        1
#define DEV_DEFAULT_NAME        "/dev/ttyHS0"
#define DEV_DEFAULT_BAUD_RATE   115200
#define DEV_DEFAULT_DATA_BITS   8
#define DEV_DEFAULT_STOP_BITS   1
#define DEV_DEFAULT_FLOW_CTRL   0
#define DEV_DEFAULT_PARITY      0

typedef struct serial_config_{
    char         _dev_name[DEV_NAME_MAX_SIZE];
    uint8_t      _allowTouse;
    uint8_t      _dataType;
    uint32_t     _baud_rates;  //not used
    uint8_t      _data_bits;
    uint8_t      _stop_bits;
    uint8_t      _parity;
    uint8_t      _flow_control;
}SERIAL_CONFIG;
#define    SERIAL_CONFIG_SIZE    sizeof(SERIAL_CONFIG)

void init_serialport(const char* config);

void write_serialport(const char* data, const uint16_t length);

void dinit_serialport();

#ifdef __cplusplus
}
#endif

