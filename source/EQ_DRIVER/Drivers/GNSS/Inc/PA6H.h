//
// Created by paulo on 26/09/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H
#define EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H

#include "main.h"
#include "usart.h"

#define MAX_NMEA_SENTENCE_SIZE 75
#define GNSS_BUFFER_SIZE           128
#define GNSS_USART             &huart1

#define TARGET_NMEA_SENTENCE "$GPRMC"
#define RMC_ONLY_OUTPUT_CMD  "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"


typedef enum {
	INVALID_DATA = 0,
	VALID_DATA,
} data_validity_t;

typedef struct GNSS_data {
	uint32_t nmea_utc;
	uint32_t nmea_date;
	float nmea_latitude;
	float nmea_longitude;
	char hemisphere;
	char longitude_side;
	data_validity_t is_valid;
	
} GNSS_data_t;

/**
 * @brief Initializes the GNSS module in IT mode.
 */
void GNSS_init(void);

/**
 * @brief Checks if given str is a valid NMEA sentence
 *
 * @param nmea_str str to be checked
 * @return bool true if nmea_str is a valid NMEA sentence, false otherwise.
 */
bool is_NMEA_str_valid(const char* nmea_str);

/**
 * @brief Stores the data from the given valid NMEA GPRMC sentence into the proper struct.
 * @param GNSS_data [out] Reference to the struct for the data to be saved on.
 * @param string valid NMEA GPRMC sentence.
 * @return bool true if the data was successfully saved, false otherwise.
 */
bool GNSS_get_data(GNSS_data_t* GNSS_data, char* string);

/**
 * @brief Callback for the IT on the UART RX pin.
 *
 * @param GNSS_data [out] Target struct for the data received to be saved on.
 * @return bool true if the data was successfully saved, false otherwise.
 */
bool GNSS_UART_CallBack(GNSS_data_t* GNSS_data);

#endif //EQMOUNT_CUSTOM_CONTROLLER_PA6H_GNSS_H