//
// Created by paulo on 21/11/2022.
//

#ifndef EQMOUNT_CUSTOM_CONTROLLER_NRF24L01P_H
#define EQMOUNT_CUSTOM_CONTROLLER_NRF24L01P_H

#include "main.h"

// Retransmit delay
typedef enum {
	nRF24_ARD_NONE = (uint8_t) 0x00, // Dummy value for case when retransmission is not used
	nRF24_ARD_250us = (uint8_t) 0x00,
	nRF24_ARD_500us = (uint8_t) 0x01,
	nRF24_ARD_750us = (uint8_t) 0x02,
	nRF24_ARD_1000us = (uint8_t) 0x03,
	nRF24_ARD_1250us = (uint8_t) 0x04,
	nRF24_ARD_1500us = (uint8_t) 0x05,
	nRF24_ARD_1750us = (uint8_t) 0x06,
	nRF24_ARD_2000us = (uint8_t) 0x07,
	nRF24_ARD_2250us = (uint8_t) 0x08,
	nRF24_ARD_2500us = (uint8_t) 0x09,
	nRF24_ARD_2750us = (uint8_t) 0x0A,
	nRF24_ARD_3000us = (uint8_t) 0x0B,
	nRF24_ARD_3250us = (uint8_t) 0x0C,
	nRF24_ARD_3500us = (uint8_t) 0x0D,
	nRF24_ARD_3750us = (uint8_t) 0x0E,
	nRF24_ARD_4000us = (uint8_t) 0x0F
} retransmit_delay_t;

// Data rate
typedef enum {
	nRF24_DR_250kbps = (uint8_t) 0x20, // 250kbps data rate
	nRF24_DR_1Mbps = (uint8_t) 0x00, // 1Mbps data rate
	nRF24_DR_2Mbps = (uint8_t) 0x08  // 2Mbps data rate
} data_rate_t;

// RF output power in TX mode
typedef enum {
	nRF24_TXPWR_18dBm = (uint8_t) 0x00, // -18dBm
	nRF24_TXPWR_12dBm = (uint8_t) 0x02, // -12dBm
	nRF24_TXPWR_6dBm = (uint8_t) 0x04, //  -6dBm
	nRF24_TXPWR_0dBm = (uint8_t) 0x06  //   0dBm
} output_power_t;

// CRC encoding scheme
typedef enum {
	nRF24_CRC_off = (uint8_t) 0x00, // CRC disabled
	nRF24_CRC_1byte = (uint8_t) 0x08, // 1-byte CRC
	nRF24_CRC_2byte = (uint8_t) 0x0c  // 2-byte CRC
} crc_scheme_t;

// nRF24L01 power control
typedef enum {
	nRF24_PWR_UP = (uint8_t) 0x02, // Power up
	nRF24_PWR_DOWN = (uint8_t) 0x00  // Power down
} power_state_t;

// Transceiver mode
typedef enum {
	nRF24_MODE_RX = (uint8_t) 0x01, // PRX
	nRF24_MODE_TX = (uint8_t) 0x00  // PTX
} transceiver_mode_t;

typedef enum {
	nRF24_DPL_ON = (uint8_t) 0x01, // PRX
	nRF24_DPL_OFF = (uint8_t) 0x00  // PTX
} dpl_state_t;

// Enumeration of RX pipe addresses and TX address
typedef enum {
	nRF24_PIPE0 = (uint8_t) 0x00, // pipe0
	nRF24_PIPE1 = (uint8_t) 0x01, // pipe1
	nRF24_PIPE2 = (uint8_t) 0x02, // pipe2
	nRF24_PIPE3 = (uint8_t) 0x03, // pipe3
	nRF24_PIPE4 = (uint8_t) 0x04, // pipe4
	nRF24_PIPE5 = (uint8_t) 0x05, // pipe5
	nRF24_PIPETX = (uint8_t) 0x06  // TX address (not a pipe in fact)
} pipe_addr_t;

// State of auto acknowledgment for specified pipe
typedef enum {
	nRF24_AA_OFF = (uint8_t) 0x00,
	nRF24_AA_ON = (uint8_t) 0x01
} auto_ack_state_t;

// Status of the RX FIFO
typedef enum {
	nRF24_STATUS_RXFIFO_DATA = (uint8_t) 0x00, // The RX FIFO contains data and available locations
	nRF24_STATUS_RXFIFO_EMPTY = (uint8_t) 0x01, // The RX FIFO is empty
	nRF24_STATUS_RXFIFO_FULL = (uint8_t) 0x02, // The RX FIFO is full
	nRF24_STATUS_RXFIFO_ERROR = (uint8_t) 0x03  // Impossible state: RX FIFO cannot be empty and full at the same time
} rx_fifo_status_t;

// Status of the TX FIFO
typedef enum {
	nRF24_STATUS_TXFIFO_DATA = (uint8_t) 0x00, // The TX FIFO contains data and available locations
	nRF24_STATUS_TXFIFO_EMPTY = (uint8_t) 0x01, // The TX FIFO is empty
	nRF24_STATUS_TXFIFO_FULL = (uint8_t) 0x02, // The TX FIFO is full
	nRF24_STATUS_TXFIFO_ERROR = (uint8_t) 0x03  // Impossible state: TX FIFO cannot be empty and full at the same time
} tx_fifo_status_t;

// Result of RX FIFO reading
typedef enum {
	nRF24_RX_PIPE0 = (uint8_t) 0x00, // Packet received from the PIPE#0
	nRF24_RX_PIPE1 = (uint8_t) 0x01, // Packet received from the PIPE#1
	nRF24_RX_PIPE2 = (uint8_t) 0x02, // Packet received from the PIPE#2
	nRF24_RX_PIPE3 = (uint8_t) 0x03, // Packet received from the PIPE#3
	nRF24_RX_PIPE4 = (uint8_t) 0x04, // Packet received from the PIPE#4
	nRF24_RX_PIPE5 = (uint8_t) 0x05, // Packet received from the PIPE#5
	nRF24_RX_EMPTY = (uint8_t) 0xff  // The RX FIFO is empty
} nRF24_RXResult;

// Function prototypes
void nRF24_Init(void);

uint8_t nRF24_Check(void);

void nRF24_SetPowerMode(uint8_t mode);

void nRF24_SetOperationalMode(uint8_t mode);

void nRF24_SetRFChannel(uint8_t channel);

void nRF24_SetAutoRetr(uint8_t ard, uint8_t arc);

void nRF24_SetAddrWidth(uint8_t addr_width);

void nRF24_SetAddr(uint8_t pipe, const uint8_t* addr);

void nRF24_SetTXPower(uint8_t tx_pwr);

void nRF24_SetDataRate(uint8_t data_rate);

void nRF24_SetCRCScheme(uint8_t scheme);

void nRF24_SetRXPipe(uint8_t pipe, uint8_t aa_state, uint8_t payload_len);

void nRF24_ClosePipe(uint8_t pipe);

void nRF24_EnableAA(uint8_t pipe);

void nRF24_DisableAA(uint8_t pipe);

void nRF24_SetDynamicPayloadLength(uint8_t mode);

void nRF24_SetPayloadWithAck(uint8_t mode);

uint8_t nRF24_GetStatus(void);

uint8_t nRF24_GetIRQFlags(void);

uint8_t nRF24_GetStatus_RXFIFO(void);

uint8_t nRF24_GetStatus_TXFIFO(void);

uint8_t nRF24_GetRXSource(void);

uint8_t nRF24_GetRetransmitCounters(void);

uint8_t nRF24_GetFeatures(void);

void nRF24_ResetPLOS(void);

void nRF24_FlushTX(void);

void nRF24_FlushRX(void);

void nRF24_ClearIRQFlags(void);

void nRF24_ActivateFeatures(void);

void nRF24_WritePayload(uint8_t* pBuf, uint8_t length);

void nRF24_WriteAckPayload(nRF24_RXResult pipe, char* payload, uint8_t length);

nRF24_RXResult nRF24_ReadPayload(uint8_t* pBuf, uint8_t* length);

nRF24_RXResult nRF24_ReadPayloadDpl(uint8_t* pBuf, uint8_t* length);

#endif //EQMOUNT_CUSTOM_CONTROLLER_NRF24L01P_H