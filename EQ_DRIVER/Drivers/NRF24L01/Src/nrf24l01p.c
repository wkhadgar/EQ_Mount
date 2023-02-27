//
// Created by paulo on 21/11/2022.
//

#include "nrf24l01p.h"
#include "spi.h"
#include "variables.h"

// Fake address to test transceiver presence (5 bytes long)
#define NRF_CS_GPIO NRF_CSN_GPIO_Port
#define NRF_CS_PIN NRF_CSN_Pin

#define NRF_CE_GPIO NRF_CE_GPIO_Port
#define NRF_CE_PIN NRF_CE_Pin

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

typedef enum {
    REG_CONFIG = 0x00,
    REG_EN_AA = 0x01,
    REG_EN_RXADDR = 0x02,
    REG_SETUP_AW = 0x03,
    REG_SETUP_RETR = 0x04,
    REG_RF_CH = 0x05,
    REG_RF_SETUP = 0x06,
    REG_STATUS = 0x07,
    REG_OBSERVE_TX = 0x08,
    REG_RPD = 0x09,
    REG_RX_ADDR_P0 = 0x0A,
    REG_RX_ADDR_P1 = 0x0B,
    REG_RX_ADDR_P2 = 0x0C,
    REG_RX_ADDR_P3 = 0x0D,
    REG_RX_ADDR_P4 = 0x0E,
    REG_RX_ADDR_P5 = 0x0F,
    REG_TX_ADDR = 0x10,
    REG_RX_PW_P0 = 0x11,
    REG_RX_PW_P1 = 0x12,
    REG_RX_PW_P2 = 0x13,
    REG_RX_PW_P3 = 0x14,
    REG_RX_PW_P4 = 0x15,
    REG_RX_PW_P5 = 0x16,
    REG_FIFO_STATUS = 0x17,
    REG_DYNPD = 0x1C,
    REG_FEATURE = 0x1D,

    _NRF_REG_AMOUNT = 26,
} nrf_regs_t;

static struct {
    uint8_t registers[_NRF_REG_AMOUNT];
    struct {
        retransmit_delay_t rt_delay;
        uint8_t rt_cnt;
        data_rate_t data_rate;
        output_power_t tx_pwr;
        crc_scheme_t crc;
        power_state_t pwr_state;
        transceiver_mode_t mode;
        dpl_state_t dpl;
        auto_ack_state_t auto_ack;
        address_width_t addr_w;
        uint8_t tx_addr[5];
        uint8_t channel;
    } configs;
    struct {
        rx_fifo_status_t rx_fifo_status;
        tx_fifo_status_t tx_fifo_status;
        bool is_p_variant;
    } info;
} nRF24 = {
        .registers = {0},
        .configs = {
                .rt_delay = nRF24_ARD_1500us,
                .rt_cnt = 15,
                .data_rate = nRF24_DR_1Mbps,
                .tx_pwr = nRF24_TXPWR_0dBm,
                .crc = nRF24_CRC_2byte,
                .pwr_state = nRF24_PWR_DOWN,
                .mode = nRF24_MODE_TX,
                .dpl = nRF24_DPL_OFF,
                .auto_ack = nRF24_AA_OFF,
                .addr_w = nRF24_ADDR_5_BITS,
                .tx_addr = {0},
                .channel = 0,
        },
        .info = {
                .rx_fifo_status = nRF24_STATUS_RXFIFO_EMPTY,
                .tx_fifo_status = nRF24_STATUS_TXFIFO_EMPTY,
                .is_p_variant = false,
        },
};

static inline void nRF24_enable(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO, NRF_CE_PIN, GPIO_PIN_SET);
}

static inline void nRF24_disable(void) {
    HAL_GPIO_WritePin(NRF_CE_GPIO, NRF_CE_PIN, GPIO_PIN_RESET);
}

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
static uint8_t nRF24_read_register(const uint8_t reg) {
    uint8_t value;


    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);

    nRF24_SPI_TR(reg & nRF24_MASK_REG_MAP);
    value = nRF24_SPI_TR(nRF24_CMD_NOP);

    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);

    if (reg <= REG_FIFO_STATUS) {
        nRF24.registers[reg] = value;
    } else if ((reg == REG_DYNPD) || (reg == REG_FEATURE)) {
        nRF24.registers[reg - 4] = value;
    }

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

    if (reg <= REG_FIFO_STATUS) {
        nRF24.registers[reg] = value;
    } else if ((reg == REG_DYNPD) || (reg == REG_FEATURE)) {
        nRF24.registers[reg - 4] = value;
    }
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
        if (nRF24_SPI_TR(*pBuf++) != HAL_OK) {
            break;
        }
    }
    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

void nRF24_init(const uint8_t* addr, uint8_t channel) {
    uint8_t prev_feat;
    bool is_p_variant = false;

    HAL_Delay(5);
    nRF24_SetAutoRetr(nRF24_ARD_2500us, 15);
    nRF24_SetDataRate(nRF24_DR_250kbps);
    nRF24_SetTXPower(nRF24_TXPWR_6dBm);

    prev_feat = nRF24_GetFeatures();
    nRF24_ActivateFeatures();
    nRF24_GetFeatures();
    nRF24.info.is_p_variant = (prev_feat == nRF24.registers[REG_FEATURE - 4]);
    if (nRF24.registers[REG_FEATURE - 4]) {
        if (nRF24.info.is_p_variant) {
            nRF24_ActivateFeatures();
        }
        nRF24_write_register(nRF24_REG_FEATURE, 0x00);
    }

    nRF24_write_register(nRF24_REG_DYNPD, 0x00);
    nRF24_write_register(nRF24_REG_EN_AA, 0x3F);
    nRF24_write_register(nRF24_REG_EN_RXADDR, 0x01);
    nRF24_setPayloadSize(32);
    nRF24_SetAddrWidth(nRF24_ADDR_5_BITS);
    nRF24_SetRFChannel(channel);
    nRF24_write_multi_register(nRF24_CMD_W_REGISTER | nRF24_REG_TX_ADDR, addr, 5);
    nRF24_write_multi_register(nRF24_CMD_W_REGISTER | nRF24_REG_RX_ADDR_P0, addr, 5);
    nRF24_ClearIRQFlags();
    nRF24_FlushRX();
    nRF24_FlushTX();

    nRF24_SetCRCScheme(nRF24_CRC_off);
    nRF24_SetPowerMode(nRF24_PWR_UP);
}

// Check if the nRF24L01 present
// return:
//   1 - nRF24L01 is online and responding
//   0 - received sequence differs from original
bool nRF24_check(const uint8_t* curr_addr) {
    nRF24_read_multi_register(nRF24_CMD_R_REGISTER | nRF24_REG_TX_ADDR, nRF24.configs.tx_addr, 5);

    // Compare buffers, return error on first mismatch
    for (uint8_t i = 0; i < 5; i++) {
        if (nRF24.configs.tx_addr[i] != *curr_addr++) {
            return false;
        }
    }

    return true;
}

void nRF24_setPayloadSize(uint8_t size) {

    if (size > 32) {
        size = 32;
    } else {
        size = (size < 1) ? 1 : size;
    }

    for (uint8_t i = 0; i < 6; i++) {
        nRF24_write_register(nRF24_REG_RX_PW_P0 + i, size);
    }
}

// Control transceiver power mode
// input:
//   mode - new state of power mode, one of nRF24_PWR_xx values
void nRF24_SetPowerMode(power_state_t mode) {
    uint8_t reg;

    reg = nRF24_read_register(nRF24_REG_CONFIG);
    if (mode == nRF24_PWR_UP) {
        // Set the PWR_UP bit of CONFIG register to wake the transceiver
        // It goes into Stanby-I mode with consumption about 26uA
        reg |= nRF24_CONFIG_PWR_UP;
    } else {
        // Clear the PWR_UP bit of CONFIG register to put the transceiver
        // into power down mode with consumption about 900nA
        reg &= ~nRF24_CONFIG_PWR_UP;
    }
    nRF24_write_register(nRF24_REG_CONFIG, reg);
    HAL_Delay(5);

    nRF24.configs.pwr_state = mode;
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

    nRF24.configs.mode = mode;
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

    nRF24.configs.dpl = mode;
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

    nRF24.configs.auto_ack = mode;
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

    nRF24.configs.crc = scheme;
}

// Set frequency channel
// input:
//   channel - radio frequency channel, value from 0 to 127
// note: frequency will be (2400 + channel)MHz
// note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
void nRF24_SetRFChannel(uint8_t channel) {
    nRF24_write_register(nRF24_REG_RF_CH, channel);
    nRF24.configs.channel = channel;
}

// Set automatic retransmission parameters
// input:
//   ard - auto retransmit delay, one of nRF24_ARD_xx values
//   arc - count of auto retransmits, value form 0 to 15
// note: zero arc value means that the automatic retransmission disabled
void nRF24_SetAutoRetr(retransmit_delay_t ard, uint8_t arc) {
    // Set auto retransmit settings (SETUP_RETR register)
    nRF24_write_register(nRF24_REG_SETUP_RETR, (uint8_t) ((ard << 4) | (arc & nRF24_MASK_RETR_ARC)));
    nRF24.configs.rt_delay = ard;
    nRF24.configs.rt_cnt = arc;
}

// Set of address widths
// input:
//   addr_width - RX/TX address field width, value from 3 to 5
// note: this setting is common for all pipes
void nRF24_SetAddrWidth(address_width_t addr_width) {
    nRF24_write_register(nRF24_REG_SETUP_AW, addr_width);
    nRF24.configs.addr_w = addr_width;
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

    nRF24_read_multi_register(nRF24_ADDR_REGS[pipe], nRF24.configs.tx_addr, 5);

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

    nRF24.configs.tx_pwr = tx_pwr;
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

    nRF24.configs.data_rate = data_rate;
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
    nRF24.registers[REG_STATUS] = nRF24_read_register(nRF24_REG_STATUS);
    return nRF24.registers[REG_STATUS];
}

// Get pending IRQ flags
// return: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
uint8_t nRF24_GetIRQFlags(void) {
    return (nRF24_read_register(nRF24_REG_STATUS) & nRF24_MASK_STATUS_IRQ);
}

// Get status of the RX FIFO
// return: one of the nRF24_STATUS_RXFIFO_xx values
uint8_t nRF24_GetStatus_RXFIFO(void) {
    nRF24.info.rx_fifo_status = (nRF24_read_register(nRF24_REG_FIFO_STATUS) & nRF24_MASK_RXFIFO);
    return (uint8_t) nRF24.info.rx_fifo_status;
}

// Get status of the TX FIFO
// return: one of the nRF24_STATUS_TXFIFO_xx values
// note: the TX_REUSE bit ignored
uint8_t nRF24_GetStatus_TXFIFO(void) {
    nRF24.info.tx_fifo_status = ((nRF24_read_register(nRF24_REG_FIFO_STATUS) & nRF24_MASK_TXFIFO) >> 4);
    return (uint8_t) nRF24.info.tx_fifo_status;
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

uint8_t nRF24_GetFeatures(void) {
    return nRF24_read_register(nRF24_REG_FEATURE);
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

void nRF24_ActivateFeatures(void) {
    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
    nRF24_SPI_TR(nRF24_CMD_ACTIVATE);
    nRF24_SPI_TR(0x73);
    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);
}

// Write TX payload
// input:
//   pBuf - pointer to the buffer with payload data
//   length - payload length in bytes
void nRF24_WritePayload(const uint8_t* pBuf, uint8_t length) {
    nRF24_write_multi_register(nRF24_CMD_W_TX_PAYLOAD, pBuf, length);
}

static uint8_t nRF24_GetRxDplPayloadWidth(void) {
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

    uint32_t wait = nRF24_WAIT_TIMEOUT;

    // Transfer a data from the specified buffer to the TX FIFO

    nRF24_WritePayload(pBuf, length);
    HAL_Delay(1);
    nRF24_enable();

    while ((!get_flag(NRF_SENT)) && (!get_flag(NRF_MAX_RT)) && wait) {
        HAL_Delay(1);
        wait--;
    }

    nRF24_disable();

    if (wait == 0) {
        // Timeout
        return nRF24_TX_TIMEOUT;
    }

    // Clear pending IRQ flags
    nRF24_ClearIRQFlags();

    if (get_flag(NRF_MAX_RT)) {
        // Auto retransmit counter exceeds the programmed maximum limit
        clear_flag(NRF_MAX_RT);
        nRF24_FlushTX();
        return nRF24_TX_MAXRT;
    }

    if (get_flag(NRF_SENT)) {
        // Successful transmission
        clear_flag(NRF_SENT);
        return nRF24_TX_SUCCESS;
    }

    nRF24_FlushTX();

    return nRF24_TX_ERROR;
}

void nRF24_WriteAckPayload(nRF24_RXResult pipe, char* payload, uint8_t length) {
    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_RESET);
    nRF24_SPI_TR(nRF24_CMD_W_ACK_PAYLOAD | pipe);
    while (length--) {
        nRF24_SPI_TR((uint8_t) *payload++);
    }
    HAL_GPIO_WritePin(NRF_CS_GPIO, NRF_CS_PIN, GPIO_PIN_SET);

}

void nRF24_ReuseTX(void) {
    uint8_t status = nRF24_read_register(nRF24_REG_STATUS);
    nRF24_write_register(nRF24_REG_STATUS, status | nRF24_FLAG_MAX_RT);
    nRF24_write_register(nRF24_CMD_REUSE_TX_PL, nRF24_CMD_NOP);
    nRF24_disable();
    nRF24_enable();
}

void nRF24_StartCarrier(output_power_t pwr, uint8_t channel) {
    nRF24_StopListening();
    uint8_t rf_status = nRF24_read_register(nRF24_REG_RF_SETUP);
    nRF24_write_register(nRF24_REG_RF_SETUP, rf_status | nRF24_FLAG_CONT_WAVE | nRF24_FLAG_PLL_LOCK);

    if (nRF24.info.is_p_variant) {
        nRF24_write_register(nRF24_REG_EN_AA, 0x00);
        nRF24_SetAutoRetr(0, 0);
        uint8_t buff[PLD_LEN];
        for (uint8_t i = 0; i < PLD_LEN; i++) {
            buff[i] = 0xFF;
        }

        nRF24_write_multi_register(nRF24_REG_TX_ADDR, buff, 5);
        nRF24_FlushTX();
        nRF24_WritePayload(buff, PLD_LEN);
        nRF24_SetCRCScheme(nRF24_CRC_off);
    }
    nRF24_SetTXPower(pwr);
    nRF24_SetRFChannel(channel);
    nRF24_enable();
    if (nRF24.info.is_p_variant) {
        HAL_Delay(1);
        nRF24_disable();
        nRF24_ReuseTX();
    }
}

void nRF24_StopCarrier(void) {
    nRF24_SetPowerMode(nRF24_PWR_DOWN);
    uint8_t rf_status = nRF24_read_register(nRF24_REG_RF_SETUP);
    nRF24_write_register(nRF24_REG_RF_SETUP, (rf_status & ~nRF24_FLAG_PLL_LOCK) & ~nRF24_FLAG_CONT_WAVE);
    nRF24_disable();
}

bool nRF24_CarrierDetect(void) {
    nRF24.registers[REG_RPD] = nRF24_read_register(nRF24_REG_RPD);
    return nRF24.registers[REG_RPD] & 1;
}

bool nRF24_SendData(const uint8_t* nRF24_payload, uint8_t payload_length) {

    nRF24_TXResult tx_res;

    if (payload_length > 32) {
        return false;
    }

    // Transmit a packet
    tx_res = nRF24_TransmitPacket(nRF24_payload, payload_length);

    switch (tx_res) {
        case nRF24_TX_SUCCESS:
            return true;
        case nRF24_TX_MAXRT:
            nRF24_ResetPLOS();
            break;
        case nRF24_TX_TIMEOUT:
        case nRF24_TX_ERROR:
            break;
    }

    return false;
}

bool nRF24_GetData(uint8_t* nRF24_payload, uint8_t* payload_length) {
    if (nRF24_GetStatus_RXFIFO() != nRF24_STATUS_RXFIFO_EMPTY) {
        // Get a payload from the transceiver
        nRF24_ReadPayload(nRF24_payload, payload_length);

        return true;
    }

    return false;
}

void nRF24_StartListening(void) {
    nRF24_SetPowerMode(nRF24_PWR_UP);
    nRF24_SetOperationalMode(nRF24_MODE_RX);
    nRF24_ClearIRQFlags();
    nRF24_enable();
}

void nRF24_StopListening(void) {
    nRF24_disable();
    HAL_Delay(100);
    nRF24_SetOperationalMode(nRF24_MODE_TX);
}

bool nRF24_Talk(const nrf24_data_t* question, nrf24_data_t* answer) {
    uint8_t answer_size;
    uint8_t timeout = nRF24_WAIT_TIMEOUT;
    nRF24_StopListening();

    while (!nRF24_SendData((const uint8_t*) question, PLD_LEN) && timeout) {
        timeout--;
        HAL_Delay(10);
    }

    if (timeout == 0) {
        return false;
    }

    if (question->kind == REQUEST) {
        timeout = nRF24_WAIT_TIMEOUT;
        nRF24_StartListening();
        while (!get_flag(NRF_RECEIVE) && timeout) {
            timeout--;
            HAL_Delay(10);
        }

        if (timeout == 0) {
            return false;
        }

        clear_flag(NRF_RECEIVE);
        nRF24_GetData((uint8_t*) answer, &answer_size);
    }

    return true;
}