//
// Created by paulo on 21/11/2022.
//

#include "nrf24l01p.h"
#include "spi.h"
#include "variables.h"

// Fake address to test transceiver presence (5 bytes long)
#define NRF_CS_GPIO NRF_CSN_GPIO_Port
#define NRF_CS_PIN NRF_CSN_Pin
#define nRF24_TEST_ADDR            "abcde"

// Addresses of the RX_PW_P# registers
static const uint8_t nRF24_RX_PW_PIPE[] = {
		nRF24_REG_RX_PW_P0,
		nRF24_REG_RX_PW_P1,
		nRF24_REG_RX_PW_P2,
		nRF24_REG_RX_PW_P3,
		nRF24_REG_RX_PW_P4,
		nRF24_REG_RX_PW_P5
};

// Addresses of the address registers
static const uint8_t nRF24_ADDR_REGS[] = {
		nRF24_REG_RX_ADDR_P0,
		nRF24_REG_RX_ADDR_P1,
		nRF24_REG_RX_ADDR_P2,
		nRF24_REG_RX_ADDR_P3,
		nRF24_REG_RX_ADDR_P4,
		nRF24_REG_RX_ADDR_P5,
		nRF24_REG_TX_ADDR
};

static inline uint8_t nRF24_SPI_TR(uint8_t data) {
	uint8_t result;
	if (HAL_SPI_TransmitReceive(&hspi1, &data, &result, 1, 10) != HAL_OK) {
		return 0;
	}
	return result;
}

// Read a register
// input:
//   reg - number of register to read
// return: value of register
static uint8_t nRF24_read_register(uint8_t reg) {
	uint8_t value;
	
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	
	nRF24_SPI_TR(reg & nRF24_MASK_REG_MAP);
	value = nRF24_SPI_TR(nRF24_CMD_NOP);
	
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
	
	return value;
}

// Write a new value to register
// input:
//   reg - number of register to write
//   value - value to write
static void nRF24_write_register(uint8_t reg, uint8_t value) {
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	
	if (reg < nRF24_CMD_W_REGISTER) {
		// This is a register access
		nRF24_SPI_TR(nRF24_CMD_W_REGISTER | (reg & nRF24_MASK_REG_MAP));
		nRF24_SPI_TR(value);
	} else {
		// This is a single byte command or future command/register
		nRF24_SPI_TR(reg);
		if ((reg != nRF24_CMD_FLUSH_TX) && (reg != nRF24_CMD_FLUSH_RX) && (reg != nRF24_CMD_REUSE_TX_PL) &&
			(reg != nRF24_CMD_NOP)) {
			// Send register value
			nRF24_SPI_TR(value);
		}
	}
	
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Read a multiple byte register.
 *
 * @param reg number of the register to read.
 * @param pBuf [out] pointer to the buffer for the read data.
 * @param count number of bytes to read.
 */
static void nRF24_read_multi_register(uint8_t reg, uint8_t* pBuf, uint8_t count) {
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	
	nRF24_SPI_TR(reg);
	while (count--) {
		*pBuf++ = nRF24_SPI_TR(nRF24_CMD_NOP);
	}
	
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Write a multiple byte register.
 *
 * @param reg number of the register to write.
 * @param pBuf [out] pointer to the buffer for the write data.
 * @param count number of bytes to write.
 */
static void nRF24_write_multi_register(uint8_t reg, const uint8_t* pBuf, uint8_t count) {
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	nRF24_SPI_TR(reg);
	while (count--) {
		nRF24_SPI_TR(*pBuf++);
	}
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

// Set transceiver to it's initial state
// note: RX/TX pipe addresses remains untouched
void nRF24_PRX_init(const uint8_t* addr, uint8_t channel) {
	// Write to registers their initial values
	nRF24_write_register(nRF24_REG_CONFIG, 0x01);
	nRF24_write_register(nRF24_REG_EN_AA, 0x00);
	nRF24_write_register(nRF24_REG_EN_RXADDR, 0x03);
	nRF24_write_register(nRF24_REG_SETUP_AW, 0x03);
	nRF24_write_register(nRF24_REG_SETUP_RETR, 0x03);
	nRF24_write_register(nRF24_REG_RF_CH, channel);
	nRF24_write_multi_register(nRF24_REG_TX_ADDR, addr, 5);
	nRF24_write_register(nRF24_REG_RF_SETUP, 0x03);
	nRF24_write_register(nRF24_REG_STATUS, 0x00);
	nRF24_write_register(nRF24_REG_RX_PW_P0, 0x20);
	nRF24_write_register(nRF24_REG_RX_PW_P1, 0x20);
	nRF24_write_register(nRF24_REG_RX_PW_P2, 0x20);
	nRF24_write_register(nRF24_REG_RX_PW_P3, 0x20);
	nRF24_write_register(nRF24_REG_RX_PW_P4, 0x20);
	nRF24_write_register(nRF24_REG_RX_PW_P5, 0x20);
	nRF24_write_register(nRF24_REG_DYNPD, 0x00);
	nRF24_write_register(nRF24_REG_FEATURE, 0x03);
	
	// Clear the FIFO's
	nRF24_FlushRX();
	nRF24_FlushTX();
	
	// Clear any pending interrupt flags
	nRF24_ClearIRQFlags();
}

// Check if the nRF24L01 present
// return:
//   1 - nRF24L01 is online and responding
//   0 - received sequence differs from original
bool nRF24_check(void) {
	uint8_t rxbuf[5];
	uint8_t i;
	uint8_t* ptr = (uint8_t*) nRF24_TEST_ADDR;
	
	// Write test TX address and read TX_ADDR register
	nRF24_SetPowerMode(nRF24_PWR_DOWN);
	nRF24_read_multi_register(nRF24_CMD_R_REGISTER | nRF24_REG_TX_ADDR, rxbuf, 5);
	nRF24_write_multi_register(nRF24_CMD_W_REGISTER | nRF24_REG_TX_ADDR, ptr, 5);
	nRF24_read_multi_register(nRF24_CMD_R_REGISTER | nRF24_REG_TX_ADDR, rxbuf, 5);
	
	// Compare buffers, return error on first mismatch
	for (i = 0; i < 5; i++) {
		if (rxbuf[i] != *ptr++) return false;
	}
	
	return true;
}

// Control transceiver power mode
// input:
//   mode - new state of power mode, one of nRF24_PWR_xx values
void nRF24_SetPowerMode(power_state_t mode) {
	uint8_t reg;
	
	reg = nRF24_read_register(nRF24_REG_CONFIG);
	reg = (mode == nRF24_PWR_UP) ? (reg | nRF24_CONFIG_PWR_UP) : (reg & ~nRF24_CONFIG_PWR_UP);
	nRF24_write_register(nRF24_REG_CONFIG, reg);
}

// Set transceiver operational mode
// input:
//   mode - operational mode, one of nRF24_MODE_xx values
void nRF24_SetOperationalMode(transceiver_mode_t mode) {
	uint8_t reg;
	
	// Configure PRIM_RX bit of the CONFIG register
	reg = nRF24_read_register(nRF24_REG_CONFIG);
	reg &= ~nRF24_CONFIG_PRIM_RX;
	reg |= (mode & nRF24_CONFIG_PRIM_RX);
	nRF24_write_register(nRF24_REG_CONFIG, reg);
}

// Set transceiver DynamicPayloadLength feature for all the pipes
// input:
//   mode - status, one of nRF24_DPL_xx values
void nRF24_SetDynamicPayloadLength(dpl_state_t mode) {
	uint8_t reg;
	reg = nRF24_read_register(nRF24_REG_FEATURE);
	if (mode) {
		nRF24_write_register(nRF24_REG_FEATURE, reg | nRF24_FEATURE_EN_DPL);
		nRF24_write_register(nRF24_REG_DYNPD, 0x1F);
	} else {
		nRF24_write_register(nRF24_REG_FEATURE, reg & ~nRF24_FEATURE_EN_DPL);
		nRF24_write_register(nRF24_REG_DYNPD, 0x0);
	}
}

// Enables Payload With Ack. NB Refer to the datasheet for proper retransmit timing.
// input:
//   mode - status, 1 or 0
void nRF24_SetPayloadWithAck(auto_ack_state_t mode) {
	uint8_t reg;
	reg = nRF24_read_register(nRF24_REG_FEATURE);
	if (mode) {
		nRF24_write_register(nRF24_REG_FEATURE, reg | nRF24_FEATURE_EN_ACK_PAY);
	} else {
		nRF24_write_register(nRF24_REG_FEATURE, reg & ~nRF24_FEATURE_EN_ACK_PAY);
	}
}

// Configure transceiver CRC scheme
// input:
//   scheme - CRC scheme, one of nRF24_CRC_xx values
// note: transceiver will forcibly turn on the CRC in case if auto acknowledgment
//       enabled for at least one RX pipe
void nRF24_SetCRCScheme(crc_scheme_t scheme) {
	uint8_t reg;
	
	// Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register
	reg = nRF24_read_register(nRF24_REG_CONFIG);
	reg &= ~nRF24_MASK_CRC;
	reg |= (scheme & nRF24_MASK_CRC);
	nRF24_write_register(nRF24_REG_CONFIG, reg);
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
inline void nRF24_SetRFChannel(uint8_t channel) {
	nRF24_write_register(nRF24_REG_RF_CH, channel);
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay, one of nRF24_ARD_xx values
//   arc - count of auto retransmits, value form 0 to 15
// note: zero arc value means that the automatic retransmission disabled
inline void nRF24_SetAutoRetr(retransmit_delay_t ard, uint8_t arc) {
	nRF24_write_register(nRF24_REG_SETUP_RETR, (uint8_t) ((ard << 4) | (arc & nRF24_MASK_RETR_ARC)));
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
inline void nRF24_SetAddrWidth(address_width_t addr_width) {
	nRF24_write_register(nRF24_REG_SETUP_AW, addr_width);
}

// Set static RX address for a specified pipe
// input:
//   pipe - pipe to configure address, one of nRF24_PIPEx values
//   addr - pointer to the buffer with address
// note: pipe can be a number from 0 to 5 (RX pipes) and 6 (TX pipe)
// note: buffer length must be equal to current address width of transceiver
// note: for pipes[2..5] only first byte of address will be written because
//       other bytes of address equals to pipe1
// note: for pipes[2..5] only first byte of address will be written because
//       pipes 1-5 share the four most significant address bytes
void nRF24_SetAddr(pipe_addr_t pipe, const uint8_t* addr) {
	uint8_t addr_width;
	
	// RX_ADDR_Px register
	switch (pipe) {
		case nRF24_PIPETX:
		case nRF24_PIPE0:
		case nRF24_PIPE1:
			// Get address width
			addr_width = nRF24_read_register(nRF24_REG_SETUP_AW) + 1;
			// Write address in reverse order (LSByte first)
			addr += addr_width;
			HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
			nRF24_SPI_TR(nRF24_CMD_W_REGISTER | nRF24_ADDR_REGS[pipe]);
			do {
				nRF24_SPI_TR(*addr--);
			} while (addr_width--);
			HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
			break;
		case nRF24_PIPE2:
		case nRF24_PIPE3:
		case nRF24_PIPE4:
		case nRF24_PIPE5:
			// Write address LSBbyte (only first byte from the addr buffer)
			nRF24_write_register(nRF24_ADDR_REGS[pipe], *addr);
			break;
		default:
			// Incorrect pipe number -> do nothing
			break;
	}
}

// Configure RF output power in TX mode
// input:
//   tx_pwr - RF output power, one of nRF24_TXPWR_xx values
void nRF24_SetTXPower(output_power_t tx_pwr) {
	uint8_t reg;
	
	// Configure RF_PWR[2:1] bits of the RF_SETUP register
	reg = nRF24_read_register(nRF24_REG_RF_SETUP);
	reg &= ~nRF24_MASK_RF_PWR;
	reg |= tx_pwr;
	nRF24_write_register(nRF24_REG_RF_SETUP, reg);
}

// Configure transceiver data rate
// input:
//   data_rate - data rate, one of nRF24_DR_xx values
void nRF24_SetDataRate(data_rate_t data_rate) {
	uint8_t reg;
	
	// Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
	reg = nRF24_read_register(nRF24_REG_RF_SETUP);
	reg &= ~nRF24_MASK_DATARATE;
	reg |= data_rate;
	nRF24_write_register(nRF24_REG_RF_SETUP, reg);
}

// Configure a specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
//   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
//   payload_len - payload length in bytes
void nRF24_SetRXPipe(pipe_addr_t pipe, auto_ack_state_t aa_state, uint8_t payload_len) {
	uint8_t reg;
	
	// Enable the specified pipe (EN_RXADDR register)
	reg = (nRF24_read_register(nRF24_REG_EN_RXADDR) | (1 << pipe)) & nRF24_MASK_EN_RX;
	nRF24_write_register(nRF24_REG_EN_RXADDR, reg);
	
	// Set RX payload length (RX_PW_Px register)
	nRF24_write_register(nRF24_RX_PW_PIPE[pipe], payload_len & nRF24_MASK_RX_PW);
	
	// Set auto acknowledgment for a specified pipe (EN_AA register)
	reg = nRF24_read_register(nRF24_REG_EN_AA);
	if (aa_state == nRF24_AA_ON) {
		reg |= (1 << pipe);
	} else {
		reg &= ~(1 << pipe);
	}
	nRF24_write_register(nRF24_REG_EN_AA, reg);
}

// Disable specified RX pipe
// input:
//   PIPE - number of RX pipe, value from 0 to 5
void nRF24_ClosePipe(pipe_addr_t pipe) {
	uint8_t reg;
	
	reg = nRF24_read_register(nRF24_REG_EN_RXADDR);
	reg &= ~(1 << pipe);
	reg &= nRF24_MASK_EN_RX;
	nRF24_write_register(nRF24_REG_EN_RXADDR, reg);
}

// Enable the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
void nRF24_EnableAA(pipe_addr_t pipe) {
	uint8_t reg;
	
	// Set bit in EN_AA register
	reg = nRF24_read_register(nRF24_REG_EN_AA);
	reg |= (1 << pipe);
	nRF24_write_register(nRF24_REG_EN_AA, reg);
}

// Disable the auto retransmit (a.k.a. enhanced ShockBurst) for one or all RX pipes
// input:
//   pipe - number of the RX pipe, value from 0 to 5, any other value will disable AA for all RX pipes
void nRF24_DisableAA(pipe_addr_t pipe) {
	uint8_t reg;
	
	if (pipe > 5) {
		// Disable Auto-ACK for ALL pipes
		nRF24_write_register(nRF24_REG_EN_AA, 0x00);
	} else {
		// Clear bit in the EN_AA register
		reg = nRF24_read_register(nRF24_REG_EN_AA);
		reg &= ~(1 << pipe);
		nRF24_write_register(nRF24_REG_EN_AA, reg);
	}
}

// Get value of the STATUS register
// return: value of STATUS register
uint8_t nRF24_GetStatus(void) {
	return nRF24_read_register(nRF24_REG_STATUS);
}

// Get pending IRQ flags
// return: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
uint8_t nRF24_GetIRQFlags(void) {
	return (nRF24_read_register(nRF24_REG_STATUS) & nRF24_MASK_STATUS_IRQ);
}

// Get status of the RX FIFO
// return: one of the nRF24_STATUS_RXFIFO_xx values
uint8_t nRF24_GetStatus_RXFIFO(void) {
	return (nRF24_read_register(nRF24_REG_FIFO_STATUS) & nRF24_MASK_RXFIFO);
}

// Get status of the TX FIFO
// return: one of the nRF24_STATUS_TXFIFO_xx values
// note: the TX_REUSE bit ignored
uint8_t nRF24_GetStatus_TXFIFO(void) {
	return ((nRF24_read_register(nRF24_REG_FIFO_STATUS) & nRF24_MASK_TXFIFO) >> 4);
}

// Get pipe number for the payload available for reading from RX FIFO
// return: pipe number or 0x07 if the RX FIFO is empty
uint8_t nRF24_GetRXSource(void) {
	return ((nRF24_read_register(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1);
}

// Get auto retransmit statistic
// return: value of OBSERVE_TX register which contains two counters encoded in nibbles:
//   high - lost packets count (max value 15, can be reseted by write to RF_CH register)
//   low  - retransmitted packets count (max value 15, reseted when new transmission starts)
uint8_t nRF24_GetRetransmitCounters(void) {
	return (nRF24_read_register(nRF24_REG_OBSERVE_TX));
}

// Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
void nRF24_ResetPLOS(void) {
	uint8_t reg;
	
	// The PLOS counter is reset after write to RF_CH register
	reg = nRF24_read_register(nRF24_REG_RF_CH);
	nRF24_write_register(nRF24_REG_RF_CH, reg);
}

// Flush the TX FIFO
void nRF24_FlushTX(void) {
	nRF24_write_register(nRF24_CMD_FLUSH_TX, nRF24_CMD_NOP);
}

// Flush the RX FIFO
void nRF24_FlushRX(void) {
	nRF24_write_register(nRF24_CMD_FLUSH_RX, nRF24_CMD_NOP);
}

// Clear any pending IRQ flags
void nRF24_ClearIRQFlags(void) {
	uint8_t reg;
	
	// Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
	reg = nRF24_read_register(nRF24_REG_STATUS);
	reg |= nRF24_MASK_STATUS_IRQ;
	nRF24_write_register(nRF24_REG_STATUS, reg);
}

// Write TX payload
// input:
//   pBuf - pointer to the buffer with payload data
//   length - payload length in bytes
void nRF24_WritePayload(const uint8_t* pBuf, uint8_t length) {
	nRF24_write_multi_register(nRF24_CMD_W_TX_PAYLOAD, pBuf, length);
}

static uint8_t nRF24_GetRxDplPayloadWidth() {
	uint8_t value;
	
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	nRF24_SPI_TR(nRF24_CMD_R_RX_PL_WID);
	value = nRF24_SPI_TR(nRF24_CMD_NOP);
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
	
	return value;
	
}

static nRF24_RXResult nRF24_ReadPayloadGeneric(uint8_t* pBuf, uint8_t* length, uint8_t dpl) {
	uint8_t pipe;
	
	// Extract a payload pipe number from the STATUS register
	pipe = (nRF24_read_register(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1;
	
	// RX FIFO empty?
	if (pipe < 6) {
		// Get payload length
		if (dpl) {
			*length = nRF24_GetRxDplPayloadWidth();
			if (*length > 32) { //broken packet
				*length = 0;
				nRF24_FlushRX();
			}
		} else {
			*length = nRF24_read_register(nRF24_RX_PW_PIPE[pipe]);
		}
		
		// Read a payload from the RX FIFO
		if (*length) {
			nRF24_read_multi_register(nRF24_CMD_R_RX_PAYLOAD, pBuf, *length);
		}
		
		return ((nRF24_RXResult) pipe);
	}
	
	// The RX FIFO is empty
	*length = 0;
	
	return nRF24_RX_EMPTY;
}

// Read top level payload available in the RX FIFO
// input:
//   pBuf - pointer to the buffer to store a payload data
//   length - pointer to variable to store a payload length
// return: one of nRF24_RX_xx values
//   nRF24_RX_PIPEX - packet has been received from the pipe number X
//   nRF24_RX_EMPTY - the RX FIFO is empty
nRF24_RXResult nRF24_ReadPayload(uint8_t* pBuf, uint8_t* length) {
	return nRF24_ReadPayloadGeneric(pBuf, length, 0);
}

nRF24_RXResult nRF24_ReadPayloadDpl(uint8_t* pBuf, uint8_t* length) {
	return nRF24_ReadPayloadGeneric(pBuf, length, 1);
}

nRF24_TXResult nRF24_TransmitPacket(const uint8_t* pBuf, uint8_t length) {
	volatile uint32_t wait = nRF24_WAIT_TIMEOUT;
	
	// Transfer a data from the specified buffer to the TX FIFO
	nRF24_WritePayload(pBuf, length);
	
	HAL_Delay(1);
	
	while ((!get_flag(NRF_SENT)) && (!get_flag(NRF_MAX_RT)) && wait) {
		HAL_Delay(1);
		wait--;
	}
	
	if (wait == 0) {
		// Timeout
		return nRF24_TX_TIMEOUT;
	}
	
	// Clear pending IRQ flags
	nRF24_ClearIRQFlags();
	
	if (get_flag(NRF_MAX_RT)) {
		// Auto retransmit counter exceeds the programmed maximum limit (FIFO is not removed)
		clear_flag(NRF_MAX_RT);
		return nRF24_TX_MAXRT;
	}
	
	if (get_flag(NRF_SENT)) {
		// Successful transmission
		clear_flag(NRF_SENT);
		return nRF24_TX_SUCCESS;
	}
	
	// Some banana happens, a payload remains in the TX FIFO, flush it
	nRF24_FlushTX();
	
	return nRF24_TX_ERROR;
}

uint8_t nRF24_GetFeatures() {
	return nRF24_read_register(nRF24_REG_FEATURE);
}

void nRF24_ActivateFeatures() {
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	nRF24_SPI_TR(nRF24_CMD_ACTIVATE);
	nRF24_SPI_TR(0x73);
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

void nRF24_WriteAckPayload(nRF24_RXResult pipe, char* payload, uint8_t length) {
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
	nRF24_SPI_TR(nRF24_CMD_W_ACK_PAYLOAD | pipe);
	while (length--) {
		nRF24_SPI_TR((uint8_t) *payload++);
	}
	HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
	
}