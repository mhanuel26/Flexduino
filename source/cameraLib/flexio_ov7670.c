/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_flexio_camera.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_port.h"
#include "fsl_pit.h"

#include "ov7670_driver.h"

#include "flexio_ov7670.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

volatile tpfPingPongBufSwitchCb		gpfJpegMacroBlkCb;

edma_handle_t 	g_EDMA_Camera_Handle;
static uint8_t mblk_cnt_ = 0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void FLEXIO_Ov7670SccbInit(void);
static void FLEXIO_CameraInit(void);
static void EDMA_ChannelTransferInit(DMA_Type *EDMAUsed, uint8_t channelNum, uint8_t source, edma_tcd_t *tcd);
static void EDMA_CameraTransferInitTCD(DMA_Type *base, uint32_t channel, edma_tcd_t *tcd);
static void FLEXIO_Ov7670VsynInit(void);
void Camera_Handler(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile __attribute__((aligned(32), section(".myRAM"))) uint8_t g_FlexioCameraMacroblockBuffer[MACROLINES][MACROBLOCK];
//volatile __attribute__((aligned(32), section(".myRAM"))) uint8_t g_FlexioCameraFrameBuffer[2][OV7670_FRAME_BYTES + 32];
static __attribute__((aligned(32), section(".myRAM"))) edma_tcd_t s_TcdMemoryPtrFlexioToFrame[1];


/* OV7670 configuration structures. */
static ov7670_handler_t s_Ov7670CameraHandler = {
    BOARD_FLEXIO_I2C_INSTANCE_BASE, OV7670_I2C_ADDR,
};

static ov7670_advanced_config_t s_Ov7670CameraAdvancedConfig = OV7670_ADVANCED_CONFIGURATION;
static ov7670_config_t s_Ov7670CameraConfig = OV7670_CONFIGURATION;

static FLEXIO_CAMERA_Type s_FlexioCameraDevice = {
    .flexioBase = BOARD_FLEXIO_INSTANCE_BASE,
    .datPinStartIdx = BOARD_FLEXIO_DATA_PIN_START_INDEX, /* fxio_pin 4 - 11 are used. */
    .pclkPinIdx = BOARD_FLEXIO_PCLK_PIN_INDEX,           /* fxio_pin 0 is used as pclk pin. */
    .hrefPinIdx = BOARD_FLEXIO_HREF_PIN_INDEX,           /* flexio_pin 3 is used as href pin. */
    .shifterStartIdx = 0U,                               /* Shifter 0 = 3 are used. */
    .shifterCount = 4,									// BOARD_FLEXIO_DATA_PINS,  taking advantage of this non related defines is not making code readable...
    .timerIdx = 0U,										// so we change the number of shifters to 4 so we can use another FlexIO Peripheral.
};

static flexio_camera_config_t s_FlexioCameraConfig;

static volatile uint8_t s_FlexioStartCapture = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/

void FLEXIO_Ov7670Init(void)
{
    /* OV7670 status indicates whether the configuration is successful */
    ov7670_status_t ov7670Status;
    edma_channel_Preemption_config_t  video_preemption_cfg;

    /* Init the I2C to communicate with the SCCB in OV7670 */
    FLEXIO_Ov7670SccbInit();
    /* Configure the OV7670 camera */
    do
    {
        ov7670Status = OV7670_Init(&s_Ov7670CameraHandler, (ov7670_config_t *)&s_Ov7670CameraConfig);
    } while (ov7670Status != kStatus_OV7670_Success);

    /*Init the flexio to the camera mode */
    FLEXIO_CameraInit();

    /* Configure DMAMUX */
    DMAMUX_Init(DMAMUX0);
    /* Configure DMA */
    edma_config_t edmaConfig;
    EDMA_GetDefaultConfig(&edmaConfig);
    edmaConfig.enableDebugMode = true;
    EDMA_Init(DMA0, &edmaConfig);
    EDMA_CreateHandle(&g_EDMA_Camera_Handle, DMA0, FLEXIO_DMA_CHANNEL);
    video_preemption_cfg.channelPriority = 1;
    video_preemption_cfg.enablePreemptAbility = true;
    video_preemption_cfg.enableChannelPreemption = false;
    EDMA_SetChannelPreemptionConfig(DMA0, FLEXIO_DMA_CHANNEL, &video_preemption_cfg);
    EDMA_SetCallback(&g_EDMA_Camera_Handle, Camera_Handler, NULL);

    // In the following configuration we change from 32 bytes to 16 bytes transfers due to the reduced shifters we are using for Flex IO camera.
    s_TcdMemoryPtrFlexioToFrame->SADDR = FLEXIO_CAMERA_GetRxBufferAddress(&s_FlexioCameraDevice);
    s_TcdMemoryPtrFlexioToFrame->SOFF = 0;
    s_TcdMemoryPtrFlexioToFrame->ATTR =
        DMA_ATTR_SSIZE(kEDMA_TransferSize16Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize16Bytes);			// here we change to 16 bytes...
    s_TcdMemoryPtrFlexioToFrame->NBYTES = 16;				// change to 32 if using 8 shifters...
    s_TcdMemoryPtrFlexioToFrame->SLAST = 0;
    s_TcdMemoryPtrFlexioToFrame->DADDR = (uint32_t)&g_FlexioCameraMacroblockBuffer[0][0];
    s_TcdMemoryPtrFlexioToFrame->DOFF = 16;					// also here
    s_TcdMemoryPtrFlexioToFrame->CITER = ((MACROBLOCK >> 4)); 		//	((MACROBLOCK >> 5)); in case we are transferring 32 bytes
    s_TcdMemoryPtrFlexioToFrame->DLAST_SGA = 0;	//(uint32_t)(0 - ((int32_t)OV7670_FRAME_BYTES));
    s_TcdMemoryPtrFlexioToFrame->CSR = DMA_CSR_INTMAJOR_MASK;					// Enable DMA Interrupts when Major Loop Finish
    s_TcdMemoryPtrFlexioToFrame->BITER = ((MACROBLOCK >> 4));			//	((MACROBLOCK >> 5)); idem as above

    EDMA_ChannelTransferInit(DMA0, FLEXIO_DMA_CHANNEL, (s_FlexioCameraDevice.shifterStartIdx + 1U),
                             s_TcdMemoryPtrFlexioToFrame);
    FLEXIO_Ov7670VsynInit();
}

void FLEXIO_Ov7670StartCapture(uint8_t bufferIndex)
{
    s_FlexioStartCapture = bufferIndex;
}

void FLEXIO_Ov7670AsynCallback(void)
{
    FLEXIO_CAMERA_ClearStatusFlags(&s_FlexioCameraDevice,
                                   kFLEXIO_CAMERA_RxDataRegFullFlag | kFLEXIO_CAMERA_RxErrorFlag);
    FLEXIO_CAMERA_EnableRxDMA(&s_FlexioCameraDevice, false);
    if (s_FlexioStartCapture > 0)
    {
    	s_FlexioStartCapture = 0;
		/* Reset the EDMA source, dest address and transfer counter to sync picture frame */
    	// we always start with buffer at index 0
		mblk_cnt_ = 0;
		DMA0->TCD[FLEXIO_DMA_CHANNEL].SADDR = FLEXIO_CAMERA_GetRxBufferAddress(&s_FlexioCameraDevice);
		DMA0->TCD[FLEXIO_DMA_CHANNEL].DADDR = (uint32_t)&g_FlexioCameraMacroblockBuffer[mblk_cnt_][0];
		DMA0->TCD[FLEXIO_DMA_CHANNEL].CITER_ELINKNO =  ((MACROBLOCK >> 4)); //((OV7670_FRAME_BYTES >> 5));
		DMA0->TCD[FLEXIO_DMA_CHANNEL].BITER_ELINKNO =  ((MACROBLOCK >> 4)); //((OV7670_FRAME_BYTES >> 5));
		FLEXIO_CAMERA_EnableRxDMA(&s_FlexioCameraDevice, true);
    }
}

static void FLEXIO_Ov7670SccbInit(void)
{
    uint32_t sourceClock = 0;
    /* i2c master init */
    i2c_master_config_t masterConfig;
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = OV7670_I2C_BAUDRATE;
    sourceClock = CLOCK_GetFreq(OV7670_I2C_CLK_SRC);
    I2C_MasterInit(BOARD_FLEXIO_I2C_INSTANCE_BASE, &masterConfig, sourceClock);
}

static void FLEXIO_CameraInit(void)
{
    FLEXIO_CAMERA_GetDefaultConfig(&s_FlexioCameraConfig);
    FLEXIO_CAMERA_Init(&s_FlexioCameraDevice, &s_FlexioCameraConfig);
    /* Clear all the flag. */
    FLEXIO_CAMERA_ClearStatusFlags(&s_FlexioCameraDevice,
                                   kFLEXIO_CAMERA_RxDataRegFullFlag | kFLEXIO_CAMERA_RxErrorFlag);
    /* Enable the flexio edma request:  from frame buffer */
    FLEXIO_EnableShifterStatusDMA(s_FlexioCameraDevice.flexioBase, 0x02, true);
    /* Enable the flexio edma request:  from FLEXIO to frame buffer */
    FLEXIO_CAMERA_EnableRxDMA(&s_FlexioCameraDevice, true);
    FLEXIO_CAMERA_Enable(&s_FlexioCameraDevice, true);
}

static void EDMA_CameraTransferInitTCD(DMA_Type *base, uint32_t channel, edma_tcd_t *tcd)
{
    assert(channel < FSL_FEATURE_EDMA_MODULE_CHANNEL);
    assert(tcd != NULL);
    assert(((uint32_t)tcd & 0x1FU) == 0);

    /* Push tcd into hardware TCD register */
    base->TCD[channel].SADDR = tcd->SADDR;
    base->TCD[channel].SOFF = tcd->SOFF;
    base->TCD[channel].ATTR = tcd->ATTR;
    base->TCD[channel].NBYTES_MLNO = tcd->NBYTES;
    base->TCD[channel].SLAST = tcd->SLAST;
    base->TCD[channel].DADDR = tcd->DADDR;
    base->TCD[channel].DOFF = tcd->DOFF;
    base->TCD[channel].CITER_ELINKNO = tcd->CITER;
    base->TCD[channel].DLAST_SGA = tcd->DLAST_SGA;
    base->TCD[channel].CSR = 0;
    base->TCD[channel].CSR = tcd->CSR;
    base->TCD[channel].BITER_ELINKNO = tcd->BITER;
}

static void EDMA_ChannelTransferInit(DMA_Type *EDMAUsed, uint8_t channelNum, uint8_t source, edma_tcd_t *tcd)
{
    DMAMUX_SetSource(DMAMUX0, channelNum, source);
    EDMA_CameraTransferInitTCD(EDMAUsed, channelNum, tcd);
    EDMA_SetModulo(EDMAUsed, channelNum, kEDMA_ModuloDisable, kEDMA_ModuloDisable);
    EDMA_ClearChannelStatusFlags(EDMAUsed, channelNum, kEDMA_DoneFlag);
    EDMA_EnableChannelRequest(EDMAUsed, channelNum);
    DMAMUX_EnableChannel(DMAMUX0, channelNum);
}

static void FLEXIO_Ov7670VsynInit(void)
{
    GPIO_Type *gpioBase[] = GPIO_BASE_PTRS;
    IRQn_Type portIRQn[] = PORT_IRQS;
    gpio_pin_config_t pinConfig;

    /* Configure OV7670 ASYN input */
    pinConfig.pinDirection = kGPIO_DigitalInput;

    GPIO_PinInit(gpioBase[BOARD_FLEXIO_VSYNC_PORT_INDEX], BOARD_FLEXIO_VSYNC_PIN_INDEX, &pinConfig);
    NVIC_EnableIRQ(portIRQn[BOARD_FLEXIO_VSYNC_PORT_INDEX]);

}


void Camera_Handler(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
	EDMA_ClearChannelStatusFlags(DMA0, FLEXIO_DMA_CHANNEL, kEDMA_InterruptFlag);
	gpfJpegMacroBlkCb(mblk_cnt_);
	mblk_cnt_++;
	DMA0->TCD[FLEXIO_DMA_CHANNEL].DADDR = (uint32_t)&g_FlexioCameraMacroblockBuffer[mblk_cnt_][0];
	return;
}

void registerCamCallback(tpfPingPongBufSwitchCb pfJpegMacroBlkCb){
	gpfJpegMacroBlkCb = pfJpegMacroBlkCb;
}

void camDeInit(void){
	s_FlexioStartCapture = 0;
	gpfJpegMacroBlkCb = NULL;
}
