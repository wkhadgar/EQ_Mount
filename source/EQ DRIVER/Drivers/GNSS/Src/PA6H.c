//
// Created by paulo on 26/09/2022.
//

#include "PA6H.h"

uint8_t rx_data = 0;
uint8_t rx_index = 0;
uint8_t rx_buffer[GNSSBUFFSIZE];

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

bool GNSS_get_data(GNSS_data_t *GNSS_data, char *nmea_str) {

    if (!strncmp(nmea_str, TARGET_NMEA_SENTENCE, 6)) {
        sscanf(nmea_str, "$GPRMC,%d.%*d,%c", &GNSS_data->nmea_utc, &GNSS_data->valid_data);

        if (GNSS_data->valid_data == 'V') {
            sscanf(nmea_str, "$GPRMC,%*f,%*c,,,,,%*f,%*f,%d,,,%*c", &GNSS_data->nmea_date);
        } else if (GNSS_data->valid_data == 'A') {
            sscanf(nmea_str, "$GPRMC,%*f,%*c,%f,%c,%f,%c,%*f,%*f,%d,,,%*c", &GNSS_data->nmea_latitude,
                   &GNSS_data->hemisphere, &GNSS_data->nmea_longitude, &GNSS_data->longitude_side,
                   &GNSS_data->nmea_date);
        }

        return true;
    }

    return false;
}

bool GNSS_UART_CallBack(GNSS_data_t *GNSS_data) {
    static bool success = false;
    if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
        rx_buffer[rx_index++] = rx_data;
    } else {
        if (is_NMEA_str_valid((char *) rx_buffer)) {
            success = GNSS_get_data(GNSS_data, (char *) rx_buffer);
        }
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    HAL_UART_Receive_IT(GNSS_USART, &rx_data, 1);

    return success;
}

