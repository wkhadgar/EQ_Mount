//
// Created by paulo on 26/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H
#define EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H

#include "main.h"
#include "usart.h"

#define MAX_NMEA_SENTENCE_SIZE 75
#define GNSSBUFFSIZE           128
#define GNSS_USART             &huart1

#define TARGET_NMEA_SENTENCE "$GPRMC"
#define RMC_ONLY_OUTPUT_CMD  "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
#define MINIMUN_DATE         "010122"
#define UTC_ZONE_OFFSET       -3

typedef struct GNSS_data {
    float nmea_utc;
    float nmea_latitude;
    float nmea_longitude;
    int nmea_date;
    char hemisphere;
    char lat_side;
    char valid_data;

} GNSS_data_t;

static GNSS_data_t GNSS_info = {0};

void GNSS_init(void);

bool is_NMEA_str_valid(char *nmea_str);

void GNSS_get_data(char *string);

void GNSS_UART_CallBack();

#endif //EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H
