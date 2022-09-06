/*
 * bkp_regs.h
 *
 *  Created on: 29 de ago de 2022
 *      Author: paulo
 */


#include "bkp_regs.h"

/*
 * @brief: enables bkp register writing access
 */
void BKP_W_ENABLE(void) {
    RCC->APB1ENR |= (0b11 << 27);
    PWR->CR |= (1 << 8);
}

/*
 * @brief: disables bkp register writing access
 */
void BKP_W_DISABLE(void) {
    RCC->APB1ENR &= ~(0b11 << 27);
    PWR->CR &= ~(1 << 8);
}

/*
 * @brief: reads byte from given addr, splitting each bkp register 2byte writable space into 2 bytes
 * @param addr: address of the bkp register to be read, can be from 0 to 19 (20 bytes total)
 * @retval: byte readed from the given adress
 */
uint8_t BKP_read(uint8_t addr) {
    if (addr < 20) {
        uint32_t * target_DR = &BKP->DR1; // base

        target_DR += addr >> 1; // selects the bkp register address
        bool_t target_DR_shift = addr & 1; // selects if the byte should be written shifted

        uint8_t *saved_data = (uint8_t *) target_DR; //resizes the pointer to do 1byte steps

        return saved_data[target_DR_shift]; //reads the byte sized pointer content
    }
    return 0;
}


/*
 * @brief: writes char into bkp register address
 * @param addr: address of the bkp register to be written, can be from 0 to 19 (20 bytes total)
 * @param data: unsigned char data to be written at given address
 */
void BKP_write(uint8_t addr, uint8_t data) {
    if (addr < 20) {
        uint32_t * target_DR = &BKP->DR1; // base

        target_DR += addr >> 1; // selects the bkp register address
        bool_t target_DR_shift = addr & 1; // selects if the byte should be written shifted

        uint8_t *DR_bytes = (uint8_t *) target_DR;    //resizes the pointer to do 1byte steps

        if (DR_bytes[target_DR_shift] != data) { //checks for redundant write
            BKP_W_ENABLE();
            DR_bytes[target_DR_shift] = data; //assign value to corresponding "bit field"
            BKP_W_DISABLE();
        }
    }
    return;
}


