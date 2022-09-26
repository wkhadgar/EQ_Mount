//
// Created by paulo on 26/09/2022.
//

#include "PA6H.h"

uint8_t rx_data = 0;
uint8_t rx_buffer[GNSSBUFFSIZE];
uint8_t rx_index = 0;

void GNSS_init(void) {
    HAL_UART_Transmit(GNSS_USART, RMC_ONLY_OUTPUT_CMD, sizeof(RMC_ONLY_OUTPUT_CMD), 100);
    HAL_UART_Receive_IT(GNSS_USART, &rx_data, 1);
}

bool is_NMEA_str_valid(char *nmea_str) {
    char check[2];
    char nmea_str_checksum[3];
    uint8_t calculated_check = 0;
    uint8_t i = 0;

    if (nmea_str[i] == '$') {
        i++;
    } else {
        return false;
    }

    /** checksum calculation */
    while ((nmea_str[i] != '\0') && (nmea_str[i] != '*') && (i < MAX_NMEA_SENTENCE_SIZE)) {
        calculated_check ^= nmea_str[i++];
    }

    if (i >= MAX_NMEA_SENTENCE_SIZE) {
        return false;
    }

    /** checksum catch */
    if (nmea_str[i] == '*') {
        check[0] = nmea_str[i + 1];
        check[1] = nmea_str[i + 2];
    } else {
        return false;
    }

    sprintf(nmea_str_checksum, "%02X", calculated_check);
    return ((nmea_str_checksum[0] == check[0])
            && (nmea_str_checksum[1] == check[1])) ? true : false;
}

void GNSS_get_data(char *nmea_str) {
    float trash;
    char trash_;
    if (!strncmp(nmea_str, TARGET_NMEA_SENTENCE, 6)) {
        if (sscanf(nmea_str, "$GPRMC,%f,%c,%f,%c,%f,%c,%f,%f,%d,,,%c", &GNSS_info.nmea_utc, &GNSS_info.valid_data,
                   &GNSS_info.nmea_latitude,
                   &GNSS_info.hemisphere, &GNSS_info.nmea_longitude, &GNSS_info.lat_side, &trash, &trash,
                   &GNSS_info.nmea_date, &trash_) >= 1)
            return;
    }
}

void GNSS_UART_CallBack() {
    if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
        rx_buffer[rx_index++] = rx_data;
    } else {
        if (is_NMEA_str_valid((char *) rx_buffer)) {
            GNSS_get_data((char *) rx_buffer);
        }
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    HAL_UART_Receive_IT(GNSS_USART, &rx_data, 1);
}

