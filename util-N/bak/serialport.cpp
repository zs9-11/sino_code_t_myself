#include <stdarg.h>
#include <stdlib.h>

#include <SerialPort.h>

#include "ini.h"
#include "logger.h"
#include "serialport.h"

SERIAL_CONFIG serial_config;

itas109::CSerialPort sp;

#ifdef __cplusplus
extern "C" {
#endif

static int  handler_serial_conf(void* user, const char* section, const char* name,
                 const char* value)
{
    SERIAL_CONFIG* pconfig = (SERIAL_CONFIG*)user;

    if(MATCH("DATA", "BAUD_RATES")) {
        pconfig->_baud_rates= atoi(value);
    }else if (MATCH("DATA", "DATA_BITS")) {
        pconfig->_data_bits= atoi(value);
    }else if (MATCH("DATA", "DATA_OUTPUT")) {
        pconfig->_allowTouse = atoi(value);
    }else if (MATCH("DATA", "DATA_TYPE")) {
        pconfig->_dataType = atoi(value);
    }else if (MATCH("DATA", "DATA_COM")) {
        snprintf(pconfig->_dev_name, DEV_NAME_MAX_SIZE, "%s", value);
    }else if (MATCH("DATA", "STOP_BITS")) {
        pconfig->_stop_bits = atoi(value);
    }else if (MATCH("DATA", "PARITY")) {
        pconfig->_parity = atoi(value);
    }else if (MATCH("DATA", "FLOW_CONTROL")) {
        pconfig->_flow_control = atoi(value);
    }else{
        return 0;  /* unknown section/name, error */
                 
    }
    return 1;
}

#ifdef __cplusplus
}
#endif

static void private_init_serialport()
{
    if((serial_config._allowTouse&PPE_DATA_OUTPUT_SERIAL) && 
        strlen(serial_config._dev_name) &&
       !sp.isOpened()){
	    sp.init(serial_config._dev_name, itas109::BaudRate115200);
	    sp.open();
        int open_error = sp.getLastError();
        if(!sp.isOpened()){
            LOG_ERROR("Fail to open serial:%s,error:%d\n", serial_config._dev_name, open_error);
        }else{
            LOG_ERROR("Open serial:%s\n", serial_config._dev_name);
        }
    }
    return;
}

static void private_serialport_default_conf()
{
    serial_config._baud_rates = DEV_DEFAULT_BAUD_RATE;
    serial_config._data_bits = DEV_DEFAULT_DATA_BITS;
    serial_config._allowTouse = DEV_DEFAULT_USED;
    snprintf(serial_config._dev_name, DEV_NAME_MAX_SIZE, "%s", DEV_DEFAULT_NAME);
    serial_config._stop_bits = DEV_DEFAULT_STOP_BITS;
    serial_config._parity = DEV_DEFAULT_PARITY;
    serial_config._flow_control = DEV_DEFAULT_FLOW_CTRL;

    return;
}

void init_serialport(const char *config)
{
    memset((void*)&serial_config, 0x00, SERIAL_CONFIG_SIZE);
    
    if (ini_parse(config, handler_serial_conf, &serial_config) < 0) {
        private_serialport_default_conf();
        LOG_ERROR("Can't load serial config:%s\n", config);
        //return ;
    }
    LOG_INFO("load serial config,dev:%s,Baud:115200,...\n", serial_config._dev_name);

    private_init_serialport();
    return;
}

void write_serialport(const char* data, const uint16_t length)
{

    if(!sp.isOpened()){
        private_init_serialport();
    }

    if(!sp.isOpened()){
        return;
    }

    uint16_t output = 0;
    while((length- output) > 0 ){
        int ret = - 1;
        if((length- output) > 512){
            ret = sp.writeData(&data[output], 512);
        }else{
            ret = sp.writeData(&data[output], length - output);
        }
        
        if(ret == -1){
            continue;
        }else{
            output += ret;
        }
    }
}

void dinit_serialport()
{
    if(sp.isOpened()){
        sp.close();
    }
}
