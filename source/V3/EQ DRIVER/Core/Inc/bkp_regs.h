/*
 * bkp_regs.c
 *
 *  Created on: Aug 29, 2022
 *      Author: paulo
 */

#ifndef INC_BKP_REGS_H_
#define INC_BKP_REGS_H_

#include "main.h"

void BKP_W_ENABLE(void);
void BKP_W_DISABLE(void);
uint8_t BKP_read(uint8_t addr);
void BKP_write(uint8_t addr, uint8_t data);

#endif /* INC_BKP_REGS_H_ */
