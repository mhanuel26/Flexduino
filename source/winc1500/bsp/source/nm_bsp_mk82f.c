/**
 *
 * \file
 *
 * \brief This module contains K82F BSP APIs implementation.
 *
 *
 **/

#include "bsp/include/nm_bsp.h"
#include "bsp/include/nm_bsp_mk82fn256.h"
#include "common/include/nm_common.h"
#include <stdint.h>
#include "board.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/
//volatile bool pitIsrFlag = false;
//volatile unsigned long timer0_millis;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static tpfNmBspIsr gpfIsr;
static void chip_isr(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_WINC_INT_IRQ_HANDLER(void){
    /* Check if WINC INT pin was the cause. */
    uint32_t id = GPIO_GetPinsInterruptFlags(BOARD_WINC_INT_GPIO);
    if(id && BOARD_WINC_INT_MASK){
    	chip_isr();
    }
    /* Clear external interrupt flag. */
	GPIO_ClearPinsInterruptFlags(BOARD_WINC_INT_GPIO, 1U << BOARD_WINC_INT_GPIO_PIN);
}

int nanosleep(const struct timespec  *rqtp, struct timespec *rmtp){

	return 0;
}

static void chip_isr(void)
{
	if (gpfIsr) {
		gpfIsr();
	}
}

/*
 *	@fn		init_chip_pins
 *	@brief	Initialize reset, chip enable and wake pin
 */
static void init_chip_pins(void)
{
    /* Define the init structure for the output WINC_RST pin */
	gpio_pin_config_t winc_rst_config = {
		kGPIO_DigitalOutput, 0,
	};
	GPIO_PinInit(BOARD_WINC_RST_GPIO, BOARD_WINC_RST_GPIO_PIN, &winc_rst_config);
	/* Define the init structure for the WINC_INT interrupt pin */
	gpio_pin_config_t winc_int_config = {
		kGPIO_DigitalInput, 0,
	};
	GPIO_PinInit(BOARD_WINC_INT_GPIO, BOARD_WINC_INT_GPIO_PIN, &winc_int_config);
	/* Define the init structure for the WINC_EN enable pin */
	gpio_pin_config_t winc_en_config = {
		kGPIO_DigitalOutput, 0,
	};
	GPIO_PinInit(BOARD_WINC_EN_GPIO, BOARD_WINC_EN_GPIO_PIN, &winc_en_config);
}

/*
*	@fn		nm_bsp_init
*	@brief	Initialize BSP
*	@return	0 in case of success and -1 in case of failure
*/
sint8 nm_bsp_init(void)
{
	gpfIsr = NULL;

	/* Initialize chip IOs. */
	init_chip_pins();

	/* Perform chip reset. */
	nm_bsp_reset();

	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_deinit
 *	@brief	De-iInitialize BSP
 *	@return	0 in case of success and -1 in case of failure
 */
sint8 nm_bsp_deinit(void)
{
	return M2M_SUCCESS;
}

/**
 *	@fn		nm_bsp_reset
 *	@brief	Reset NMC1500 SoC by setting CHIP_EN and RESET_N signals low,
 *           CHIP_EN high then RESET_N high
 */
void nm_bsp_reset(void)
{
	GPIO_ClearPinsOutput(BOARD_WINC_EN_GPIO, 1U << BOARD_WINC_EN_GPIO_PIN);
	GPIO_ClearPinsOutput(BOARD_WINC_RST_GPIO, 1U << BOARD_WINC_RST_GPIO_PIN);
	nm_bsp_sleep(100);
	// Power Up the Board
	GPIO_SetPinsOutput(BOARD_WINC_EN_GPIO, 1U << BOARD_WINC_EN_GPIO_PIN);
	nm_bsp_sleep(100);
	GPIO_SetPinsOutput(BOARD_WINC_RST_GPIO, 1U << BOARD_WINC_RST_GPIO_PIN);
	nm_bsp_sleep(100);

//	GPIO_ClearPinsOutput(BOARD_WINC_RST_GPIO, 1U << BOARD_WINC_RST_GPIO_PIN);
//	nm_bsp_sleep(200);
//	GPIO_SetPinsOutput(BOARD_WINC_RST_GPIO, 1U << BOARD_WINC_RST_GPIO_PIN);
//	nm_bsp_sleep(100);
}

/*
*	@fn		nm_bsp_sleep
*	@brief	Sleep in units of mSec
*	@param[IN]	u32TimeMsec
*				Time in milliseconds
*/
void nm_bsp_sleep(uint32 u32TimeMsec)
{
	while(u32TimeMsec--) {
		delay_ms(1);
	}
}

/*
*	@fn		nm_bsp_register_isr
*	@brief	Register interrupt service routine
*	@param[IN]	pfIsr
*				Pointer to ISR handler
*/
void nm_bsp_register_isr(tpfNmBspIsr pfIsr)
{
	gpfIsr = pfIsr;
	PORT_SetPinInterruptConfig(BOARD_WINC_INT_PORT, BOARD_WINC_INT_GPIO_PIN, kPORT_InterruptFallingEdge);
	EnableIRQ(BOARD_WINC_INT_IRQ);
}

/*
*	@fn		nm_bsp_interrupt_ctrl
*	@brief	Enable/Disable interrupts
*	@param[IN]	u8Enable
*				'0' disable interrupts. '1' enable interrupts
*/
void nm_bsp_interrupt_ctrl(uint8 u8Enable)
{
	if (u8Enable) {
	    /* Init input IRQ GPIO. */
	    PORT_SetPinInterruptConfig(BOARD_WINC_INT_PORT, BOARD_WINC_INT_GPIO_PIN, kPORT_InterruptFallingEdge);
	    EnableIRQ(BOARD_WINC_INT_IRQ);
	}
	else {
	    /* Init input IRQ GPIO. */
	    DisableIRQ(BOARD_WINC_INT_IRQ);
	}
}

