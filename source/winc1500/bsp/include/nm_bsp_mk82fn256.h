/**
 *  \file
 *
 *  \brief This module contains NMC1500 BSP APIs definitions.
 *
 *
 */
#include "board.h"
#include <stdint.h>
#include <sys/types.h>
#include "fsl_debug_console.h"
#include "fsl_common.h"

#ifndef _NM_BSP_MK82F_H_
#define _NM_BSP_MK82F_H_

// define millis function - get the timer ms count
#define millis 		_millis
// define delay in ms
#define delay_ms	_delay_ms

//unsigned long millis();
//void delay_ms(int time);

int nanosleep(const struct timespec  *rqtp, struct timespec *rmtp);

#define CONF_WINC_DEBUG			1
#define CONF_WINC_PRINTF		PRINTF


#define CLK_SPEED	150000000
#define BOARD_WINC_INT_MASK ((uint32_t)(1U << BOARD_WINC_INT_GPIO_PIN))

#define NM_DEBUG			0
#define NM_BSP_PRINTF

#define CONF_WINC_USE_SPI	1

#define NM_EDGE_INTERRUPT	1

#endif /* _NM_BSP_MK82F_H_ */
