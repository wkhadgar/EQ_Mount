//
// Created by paulo on 21/11/2022.
//

#include "nrf24l01p.h"
#include "spi.h"

#define CS NRF_SCN_GPIO_Port
#define CS_pin NRF_SCN_Pin

#define nRF24_CMD_R_REGISTER       (uint8_t)0x00      /**<  Register read */
#define nRF24_CMD_W_REGISTER       (uint8_t)0x20      /**<  Register write */
#define nRF24_CMD_ACTIVATE         (uint8_t)0x50      /**<  (De)Activates R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK features */
#define nRF24_CMD_R_RX_PL_WID       (uint8_t)0x60     /**<  Read RX-payload width for the top R_RX_PAYLOAD in the RX FIFO. */
#define nRF24_CMD_R_RX_PAYLOAD     (uint8_t)0x61      /**<  Read RX payload */
#define nRF24_CMD_W_TX_PAYLOAD     (uint8_t)0xA0      /**<  Write TX payload */
#define nRF24_CMD_W_ACK_PAYLOAD    (uint8_t)0xA8      /**<  Write ACK payload */
#define nRF24_CMD_W_TX_PAYLOAD_NO_ACK (uint8_t) 0xB0  /**<  Write TX payload and disable AUTOACK */
#define nRF24_CMD_FLUSH_TX         (uint8_t)0xE1      /**<  Flush TX FIFO */
#define nRF24_CMD_FLUSH_RX         (uint8_t)0xE2      /**<  Flush RX FIFO */
#define nRF24_CMD_REUSE_TX_PL      (uint8_t)0xE3      /**<  Reuse TX payload */
#define nRF24_CMD_LOCK_UNLOCK      (uint8_t)0x50      /**<  Lock/unlock exclusive features */
#define nRF24_CMD_NOP              (uint8_t)0xFF      /**<  No operation (used for reading status register) */

// nRF24L0 register definitions
#define nRF24_REG_CONFIG           (uint8_t)0x00 /**< Configuration register */
#define nRF24_REG_EN_AA            (uint8_t)0x01 /**< Enable "Auto acknowledgment" */
#define nRF24_REG_EN_RXADDR        (uint8_t)0x02 /**< Enable RX addresses */
#define nRF24_REG_SETUP_AW         (uint8_t)0x03 /**< Setup of address widths */
#define nRF24_REG_SETUP_RETR       (uint8_t)0x04 /**< Setup of automatic retransmit */
#define nRF24_REG_RF_CH            (uint8_t)0x05 /**< RF channel */
#define nRF24_REG_RF_SETUP         (uint8_t)0x06 /**< RF setup register */
#define nRF24_REG_STATUS           (uint8_t)0x07 /**< Status register */
#define nRF24_REG_OBSERVE_TX       (uint8_t)0x08 /**< Transmit observe register */
#define nRF24_REG_RPD              (uint8_t)0x09 /**< Received power detector */
#define nRF24_REG_RX_ADDR_P0       (uint8_t)0x0A /**< Receive address data pipe 0 */
#define nRF24_REG_RX_ADDR_P1       (uint8_t)0x0B /**< Receive address data pipe 1 */
#define nRF24_REG_RX_ADDR_P2       (uint8_t)0x0C /**< Receive address data pipe 2 */
#define nRF24_REG_RX_ADDR_P3       (uint8_t)0x0D /**< Receive address data pipe 3 */
#define nRF24_REG_RX_ADDR_P4       (uint8_t)0x0E /**< Receive address data pipe 4 */
#define nRF24_REG_RX_ADDR_P5       (uint8_t)0x0F /**< Receive address data pipe 5 */
#define nRF24_REG_TX_ADDR          (uint8_t)0x10 /**< Transmit address */
#define nRF24_REG_RX_PW_P0         (uint8_t)0x11 /**< Number of bytes in RX payload in data pipe 0 */
#define nRF24_REG_RX_PW_P1         (uint8_t)0x12 /**< Number of bytes in RX payload in data pipe 1 */
#define nRF24_REG_RX_PW_P2         (uint8_t)0x13 /**< Number of bytes in RX payload in data pipe 2 */
#define nRF24_REG_RX_PW_P3         (uint8_t)0x14 /**< Number of bytes in RX payload in data pipe 3 */
#define nRF24_REG_RX_PW_P4         (uint8_t)0x15 /**< Number of bytes in RX payload in data pipe 4 */
#define nRF24_REG_RX_PW_P5         (uint8_t)0x16 /**< Number of bytes in RX payload in data pipe 5 */
#define nRF24_REG_FIFO_STATUS      (uint8_t)0x17 /**< FIFO status register */
#define nRF24_REG_DYNPD            (uint8_t)0x1C /**< Enable dynamic payload length */
#define nRF24_REG_FEATURE          (uint8_t)0x1D /**< Feature register */

// Register bits definitions
#define nRF24_CONFIG_PRIM_RX       (uint8_t)0x01 // PRIM_RX bit in CONFIG register
#define nRF24_CONFIG_PWR_UP        (uint8_t)0x02 // PWR_UP bit in CONFIG register
#define nRF24_FEATURE_EN_DYN_ACK   (uint8_t)0x01 // EN_DYN_ACK bit in FEATURE register
#define nRF24_FEATURE_EN_ACK_PAY   (uint8_t)0x02 // EN_ACK_PAY bit in FEATURE register
#define nRF24_FEATURE_EN_DPL       (uint8_t)0x04 // EN_DPL bit in FEATURE register
#define nRF24_FLAG_RX_DR           (uint8_t)0x40 // RX_DR bit (data ready RX FIFO interrupt)
#define nRF24_FLAG_TX_DS           (uint8_t)0x20 // TX_DS bit (data sent TX FIFO interrupt)
#define nRF24_FLAG_MAX_RT          (uint8_t)0x10 // MAX_RT bit (maximum number of TX retransmits interrupt)

// Register masks definitions
#define nRF24_MASK_REG_MAP         (uint8_t)0x1F // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
#define nRF24_MASK_CRC             (uint8_t)0x0C // Mask for CRC bits [3:2] in CONFIG register
#define nRF24_MASK_STATUS_IRQ      (uint8_t)0x70 // Mask for all IRQ bits in STATUS register
#define nRF24_MASK_RF_PWR          (uint8_t)0x06 // Mask RF_PWR[2:1] bits in RF_SETUP register
#define nRF24_MASK_RX_P_NO         (uint8_t)0x0E // Mask RX_P_NO[3:1] bits in STATUS register
#define nRF24_MASK_DATARATE        (uint8_t)0x28 // Mask RD_DR_[5,3] bits in RF_SETUP register
#define nRF24_MASK_EN_RX           (uint8_t)0x3F // Mask ERX_P[5:0] bits in EN_RXADDR register
#define nRF24_MASK_RX_PW           (uint8_t)0x3F // Mask [5:0] bits in RX_PW_Px register
#define nRF24_MASK_RETR_ARD        (uint8_t)0xF0 // Mask for ARD[7:4] bits in SETUP_RETR register
#define nRF24_MASK_RETR_ARC        (uint8_t)0x0F // Mask for ARC[3:0] bits in SETUP_RETR register
#define nRF24_MASK_RXFIFO          (uint8_t)0x03 // Mask for RX FIFO status bits [1:0] in FIFO_STATUS register
#define nRF24_MASK_TXFIFO          (uint8_t)0x30 // Mask for TX FIFO status bits [5:4] in FIFO_STATUS register
#define nRF24_MASK_PLOS_CNT        (uint8_t)0xF0 // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
#define nRF24_MASK_ARC_CNT         (uint8_t)0x0F // Mask for ARC_CNT[3:0] bits in OBSERVE_TX register

// Fake address to test transceiver presence (5 bytes long)
#define nRF24_TEST_ADDR            "abcde"

// Addresses of the RX_PW_P# registers
static const uint8_t nRF24_RX_PW_PIPE[6] = {
		nRF24_REG_RX_PW_P0,
		nRF24_REG_RX_PW_P1,
		nRF24_REG_RX_PW_P2,
		nRF24_REG_RX_PW_P3,
		nRF24_REG_RX_PW_P4,
		nRF24_REG_RX_PW_P5
};

// Addresses of the address registers
static const uint8_t nRF24_ADDR_REGS[7] = {
		nRF24_REG_RX_ADDR_P0,
		nRF24_REG_RX_ADDR_P1,
		nRF24_REG_RX_ADDR_P2,
		nRF24_REG_RX_ADDR_P3,
		nRF24_REG_RX_ADDR_P4,
		nRF24_REG_RX_ADDR_P5,
		nRF24_REG_TX_ADDR
};


static inline void nRF24_CSN_L(void) {
	HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
}

static inline void nRF24_CSN_H(void) {
	HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

static inline uint8_t nRF24_LL_RW(uint8_t data) {
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
static uint8_t nRF24_ReadReg(uint8_t reg) {
	uint8_t value;
	
	nRF24_CSN_L();
	nRF24_LL_RW(reg & nRF24_MASK_REG_MAP);
	value = nRF24_LL_RW(nRF24_CMD_NOP);
	nRF24_CSN_H();
	
	return value;
}

// Write a new value to register
// input:
//   reg - number of register to write
//   value - value to write
static void nRF24_WriteReg(uint8_t reg, uint8_t value) {
	nRF24_CSN_L();
	if (reg < nRF24_CMD_W_REGISTER) {
		// This is a register access
		nRF24_LL_RW(nRF24_CMD_W_REGISTER | (reg & nRF24_MASK_REG_MAP));
		nRF24_LL_RW(value);
	} else {
		// This is a single byte command or future command/register
		nRF24_LL_RW(reg);
		if ((reg != nRF24_CMD_FLUSH_TX) && (reg != nRF24_CMD_FLUSH_RX) && \
                (reg != nRF24_CMD_REUSE_TX_PL) && (reg != nRF24_CMD_NOP)) {
			// Send register value
			nRF24_LL_RW(value);
		}
	}
	nRF24_CSN_H();
}

// Read a multi-byte register
// input:
//   reg - number of register to read
//   pBuf - pointer to the buffer for register data
//   count - number of bytes to read
static void nRF24_ReadMBReg(uint8_t reg, uint8_t* pBuf, uint8_t count) {
	nRF24_CSN_L();
	nRF24_LL_RW(reg);
	while (count--) {
		*pBuf++ = nRF24_LL_RW(nRF24_CMD_NOP);
	}
	nRF24_CSN_H();
}

// Write a multi-byte register
// input:
//   reg - number of register to write
//   pBuf - pointer to the buffer with data to write
//   count - number of bytes to write
static void nRF24_WriteMBReg(uint8_t reg, uint8_t* pBuf, uint8_t count) {
	nRF24_CSN_L();
	nRF24_LL_RW(reg);
	while (count--) {
		nRF24_LL_RW(*pBuf++);
	}
	nRF24_CSN_H();
}

// Set transceiver to it's initial state
// note: RX/TX pipe addresses remains untouched
void nRF24_Init(void) {
	// Write to registers their initial values
	nRF24_WriteReg(nRF24_REG_CONFIG, 0x08);
	nRF24_WriteReg(nRF24_REG_EN_AA, 0x3F);
	nRF24_WriteReg(nRF24_REG_EN_RXADDR, 0x03);
	nRF24_WriteReg(nRF24_REG_SETUP_AW, 0x03);
	nRF24_WriteReg(nRF24_REG_SETUP_RETR, 0x03);
	nRF24_WriteReg(nRF24_REG_RF_CH, 0x02);
	nRF24_WriteReg(nRF24_REG_RF_SETUP, 0x0E);
	nRF24_WriteReg(nRF24_REG_STATUS, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P0, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P1, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P2, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P3, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P4, 0x00);
	nRF24_WriteReg(nRF24_REG_RX_PW_P5, 0x00);
	nRF24_WriteReg(nRF24_REG_DYNPD, 0x00);
	nRF24_WriteReg(nRF24_REG_FEATURE, 0x00);
	
	// Clear the FIFO's
	nRF24_FlushRX();
	nRF24_FlushTX();
	
	// Clear any pending interrupt flags
	nRF24_ClearIRQFlags();
	
	// Deassert CSN pin (chip release)
	nRF24_CSN_H();
}

// Check if the nRF24L01 present
// return:
//   1 - nRF24L01 is online and responding
//   0 - received sequence differs from original
bool nRF24_Check(void) {
	uint8_t rxbuf[5];
	uint8_t i;
	uint8_t* ptr = (uint8_t*)
			nRF24_TEST_ADDR;
	
	// Write test TX address and read TX_ADDR register
	nRF24_WriteMBReg(nRF24_CMD_W_REGISTER | nRF24_REG_TX_ADDR, ptr, 5);
	nRF24_ReadMBReg(nRF24_CMD_R_REGISTER | nRF24_REG_TX_ADDR, rxbuf, 5);
	
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
	
	reg = nRF24_ReadReg(nRF24_REG_CONFIG);
	if (mode == nRF24_PWR_UP) {
		// Set the PWR_UP bit of CONFIG register to wake the transceiver
		// It goes into Stanby-I mode with consumption about 26uA
		reg |= nRF24_CONFIG_PWR_UP;
	} else {
		// Clear the PWR_UP bit of CONFIG register to put the transceiver
		// into power down mode with consumption about 900nA
		reg &= ~nRF24_CONFIG_PWR_UP;
	}
	nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

// Set transceiver operational mode
// input:
//   mode - operational mode, one of nRF24_MODE_xx values
void nRF24_SetOperationalMode(transceiver_mode_t mode) {
	uint8_t reg;
	
	// Configure PRIM_RX bit of the CONFIG register
	reg = nRF24_ReadReg(nRF24_REG_CONFIG);
	reg &= ~nRF24_CONFIG_PRIM_RX;
	reg |= (mode & nRF24_CONFIG_PRIM_RX);
	nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

// Set transceiver DynamicPayloadLength feature for all the pipes
// input:
//   mode - status, one of nRF24_DPL_xx values
void nRF24_SetDynamicPayloadLength(dpl_state_t mode) {
	uint8_t reg;
	reg = nRF24_ReadReg(nRF24_REG_FEATURE);
	if (mode) {
		nRF24_WriteReg(nRF24_REG_FEATURE, reg | nRF24_FEATURE_EN_DPL);
		nRF24_WriteReg(nRF24_REG_DYNPD, 0x1F);
	} else {
		nRF24_WriteReg(nRF24_REG_FEATURE, reg & ~nRF24_FEATURE_EN_DPL);
		nRF24_WriteReg(nRF24_REG_DYNPD, 0x0);
	}
}

// Enables Payload With Ack. NB Refer to the datasheet for proper retransmit timing.
// input:
//   mode - status, 1 or 0
void nRF24_SetPayloadWithAck(auto_ack_state_t mode) {
	uint8_t reg;
	reg = nRF24_ReadReg(nRF24_REG_FEATURE);
	if (mode) {
		nRF24_WriteReg(nRF24_REG_FEATURE, reg | nRF24_FEATURE_EN_ACK_PAY);
	} else {
		nRF24_WriteReg(nRF24_REG_FEATURE, reg & ~nRF24_FEATURE_EN_ACK_PAY);
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
	reg = nRF24_ReadReg(nRF24_REG_CONFIG);
	reg &= ~nRF24_MASK_CRC;
	reg |= (scheme & nRF24_MASK_CRC);
	nRF24_WriteReg(nRF24_REG_CONFIG, reg);
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
void nRF24_SetRFChannel(uint8_t channel) {
	nRF24_WriteReg(nRF24_REG_RF_CH, channel);
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay, one of nRF24_ARD_xx values
//   arc - count of auto retransmits, value form 0 to 15
// note: zero arc value means that the automatic retransmission disabled
void nRF24_SetAutoRetr(retransmit_delay_t ard, uint8_t arc) {
	// Set auto retransmit settings (SETUP_RETR register)
	nRF24_WriteReg(nRF24_REG_SETUP_RETR, (uint8_t) ((ard << 4) | (arc & nRF24_MASK_RETR_ARC)));
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
void nRF24_SetAddrWidth(address_width_t addr_width) {
	nRF24_WriteReg(nRF24_REG_SETUP_AW, addr_width);
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
			addr_width = nRF24_ReadReg(nRF24_REG_SETUP_AW) + 1;
			// Write address in reverse order (LSByte first)
			addr += addr_width;
			nRF24_CSN_L();
			nRF24_LL_RW(nRF24_CMD_W_REGISTER | nRF24_ADDR_REGS[pipe]);
			do {
				nRF24_LL_RW(*addr--);
			} while (addr_width--);
			nRF24_CSN_H();
			break;
		case nRF24_PIPE2:
		case nRF24_PIPE3:
		case nRF24_PIPE4:
		case nRF24_PIPE5:
			// Write address LSBbyte (only first byte from the addr buffer)
			nRF24_WriteReg(nRF24_ADDR_REGS[pipe], *addr);
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
	reg = nRF24_ReadReg(nRF24_REG_RF_SETUP);
	reg &= ~nRF24_MASK_RF_PWR;
	reg |= tx_pwr;
	nRF24_WriteReg(nRF24_REG_RF_SETUP, reg);
}

// Configure transceiver data rate
// input:
//   data_rate - data rate, one of nRF24_DR_xx values
void nRF24_SetDataRate(data_rate_t data_rate) {
	uint8_t reg;
	
	// Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
	reg = nRF24_ReadReg(nRF24_REG_RF_SETUP);
	reg &= ~nRF24_MASK_DATARATE;
	reg |= data_rate;
	nRF24_WriteReg(nRF24_REG_RF_SETUP, reg);
}

// Configure a specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
//   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
//   payload_len - payload length in bytes
void nRF24_SetRXPipe(pipe_addr_t pipe, auto_ack_state_t aa_state, uint8_t payload_len) {
	uint8_t reg;
	
	// Enable the specified pipe (EN_RXADDR register)
	reg = (nRF24_ReadReg(nRF24_REG_EN_RXADDR) | (1 << pipe)) & nRF24_MASK_EN_RX;
	nRF24_WriteReg(nRF24_REG_EN_RXADDR, reg);
	
	// Set RX payload length (RX_PW_Px register)
	nRF24_WriteReg(nRF24_RX_PW_PIPE[pipe], payload_len & nRF24_MASK_RX_PW);
	
	// Set auto acknowledgment for a specified pipe (EN_AA register)
	reg = nRF24_ReadReg(nRF24_REG_EN_AA);
	if (aa_state == nRF24_AA_ON) {
		reg |= (1 << pipe);
	} else {
		reg &= ~(1 << pipe);
	}
	nRF24_WriteReg(nRF24_REG_EN_AA, reg);
}

// Disable specified RX pipe
// input:
//   PIPE - number of RX pipe, value from 0 to 5
void nRF24_ClosePipe(pipe_addr_t pipe) {
	uint8_t reg;
	
	reg = nRF24_ReadReg(nRF24_REG_EN_RXADDR);
	reg &= ~(1 << pipe);
	reg &= nRF24_MASK_EN_RX;
	nRF24_WriteReg(nRF24_REG_EN_RXADDR, reg);
}

// Enable the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
// input:
//   pipe - number of the RX pipe, value from 0 to 5
void nRF24_EnableAA(pipe_addr_t pipe) {
	uint8_t reg;
	
	// Set bit in EN_AA register
	reg = nRF24_ReadReg(nRF24_REG_EN_AA);
	reg |= (1 << pipe);
	nRF24_WriteReg(nRF24_REG_EN_AA, reg);
}

// Disable the auto retransmit (a.k.a. enhanced ShockBurst) for one or all RX pipes
// input:
//   pipe - number of the RX pipe, value from 0 to 5, any other value will disable AA for all RX pipes
void nRF24_DisableAA(pipe_addr_t pipe) {
	uint8_t reg;
	
	if (pipe > 5) {
		// Disable Auto-ACK for ALL pipes
		nRF24_WriteReg(nRF24_REG_EN_AA, 0x00);
	} else {
		// Clear bit in the EN_AA register
		reg = nRF24_ReadReg(nRF24_REG_EN_AA);
		reg &= ~(1 << pipe);
		nRF24_WriteReg(nRF24_REG_EN_AA, reg);
	}
}

// Get value of the STATUS register
// return: value of STATUS register
uint8_t nRF24_GetStatus(void) {
	return nRF24_ReadReg(nRF24_REG_STATUS);
}

// Get pending IRQ flags
// return: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
uint8_t nRF24_GetIRQFlags(void) {
	return (nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_STATUS_IRQ);
}

// Get status of the RX FIFO
// return: one of the nRF24_STATUS_RXFIFO_xx values
uint8_t nRF24_GetStatus_RXFIFO(void) {
	return (nRF24_ReadReg(nRF24_REG_FIFO_STATUS) & nRF24_MASK_RXFIFO);
}

// Get status of the TX FIFO
// return: one of the nRF24_STATUS_TXFIFO_xx values
// note: the TX_REUSE bit ignored
uint8_t nRF24_GetStatus_TXFIFO(void) {
	return ((nRF24_ReadReg(nRF24_REG_FIFO_STATUS) & nRF24_MASK_TXFIFO) >> 4);
}

// Get pipe number for the payload available for reading from RX FIFO
// return: pipe number or 0x07 if the RX FIFO is empty
uint8_t nRF24_GetRXSource(void) {
	return ((nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1);
}

// Get auto retransmit statistic
// return: value of OBSERVE_TX register which contains two counters encoded in nibbles:
//   high - lost packets count (max value 15, can be reseted by write to RF_CH register)
//   low  - retransmitted packets count (max value 15, reseted when new transmission starts)
uint8_t nRF24_GetRetransmitCounters(void) {
	return (nRF24_ReadReg(nRF24_REG_OBSERVE_TX));
}

// Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
void nRF24_ResetPLOS(void) {
	uint8_t reg;
	
	// The PLOS counter is reset after write to RF_CH register
	reg = nRF24_ReadReg(nRF24_REG_RF_CH);
	nRF24_WriteReg(nRF24_REG_RF_CH, reg);
}

// Flush the TX FIFO
void nRF24_FlushTX(void) {
	nRF24_WriteReg(nRF24_CMD_FLUSH_TX, nRF24_CMD_NOP);
}

// Flush the RX FIFO
void nRF24_FlushRX(void) {
	nRF24_WriteReg(nRF24_CMD_FLUSH_RX, nRF24_CMD_NOP);
}

// Clear any pending IRQ flags
void nRF24_ClearIRQFlags(void) {
	uint8_t reg;
	
	// Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
	reg = nRF24_ReadReg(nRF24_REG_STATUS);
	reg |= nRF24_MASK_STATUS_IRQ;
	nRF24_WriteReg(nRF24_REG_STATUS, reg);
}

// Write TX payload
// input:
//   pBuf - pointer to the buffer with payload data
//   length - payload length in bytes
void nRF24_WritePayload(uint8_t* pBuf, uint8_t length) {
	nRF24_WriteMBReg(nRF24_CMD_W_TX_PAYLOAD, pBuf, length);
}

static uint8_t nRF24_GetRxDplPayloadWidth() {
	uint8_t value;
	
	nRF24_CSN_L();
	nRF24_LL_RW(nRF24_CMD_R_RX_PL_WID);
	value = nRF24_LL_RW(nRF24_CMD_NOP);
	nRF24_CSN_H();
	
	return value;
	
}

static nRF24_RXResult nRF24_ReadPayloadGeneric(uint8_t* pBuf, uint8_t* length, uint8_t dpl) {
	uint8_t pipe;
	
	// Extract a payload pipe number from the STATUS register
	pipe = (nRF24_ReadReg(nRF24_REG_STATUS) & nRF24_MASK_RX_P_NO) >> 1;
	
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
			*length = nRF24_ReadReg(nRF24_RX_PW_PIPE[pipe]);
		}
		
		// Read a payload from the RX FIFO
		if (*length) {
			nRF24_ReadMBReg(nRF24_CMD_R_RX_PAYLOAD, pBuf, *length);
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

uint8_t nRF24_GetFeatures() {
	return nRF24_ReadReg(nRF24_REG_FEATURE);
}

void nRF24_ActivateFeatures() {
	nRF24_CSN_L();
	nRF24_LL_RW(nRF24_CMD_ACTIVATE);
	nRF24_LL_RW(0x73);
	nRF24_CSN_H();
}

void nRF24_WriteAckPayload(nRF24_RXResult pipe, char* payload, uint8_t length) {
	nRF24_CSN_L();
	nRF24_LL_RW(nRF24_CMD_W_ACK_PAYLOAD | pipe);
	while (length--) {
		nRF24_LL_RW((uint8_t) *payload++);
	}
	nRF24_CSN_H();
	
}
