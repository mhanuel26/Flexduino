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

//#if !defined(WINC1501_RESET_PIN)
//  #define WINC1501_RESET_PIN  5
//#endif
//#if !defined(WINC1501_INTN_PIN)
//  #define WINC1501_INTN_PIN   7
//#endif
//#if !defined(WINC1501_SPI_CS_PIN)
//  #define WINC1501_SPI_CS_PIN 10
//#endif
//#if !defined(WINC1501_CHIP_EN_PIN)
//  #define WINC1501_CHIP_EN_PIN -1
//#endif

extern int8_t gi8Winc1501CsPin;
extern int8_t gi8Winc1501ResetPin;
extern int8_t gi8Winc1501IntnPin;
extern int8_t gi8Winc1501ChipEnPin;


#endif /* _NM_BSP_MK82F_H_ */
