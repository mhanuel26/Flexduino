#include "sdCardSPI.h"    /* Common include file for FatFs and disk I/O layer */
#include "diskio.h"    /* Common include file for FatFs and disk I/O layer */
#include <stdio.h>
#include <string.h>

/* Platform K82F  dependent includes */
#include "fsl_dspi.h"
#include "fsl_debug_console.h"
#include "board.h"
/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */

SDHostInterface  SDif;
dspi_transfer_t  _masterXfer;
static int cfgState = 0;

static uint8_t	rxBuffer[SD_SPI_TRANSFER_SIZE];
static uint8_t	txBuffer[SD_SPI_TRANSFER_SIZE];
/* Structure for SPI configuration. */
static dspi_master_config_t masterConfig;

int init_spi_port(void)
{
    if(!cfgState){
    	// configure CS output pin
        gpio_pin_config_t sd_cs_config = {
            kGPIO_DigitalOutput, 1,
        };
        GPIO_PinInit(BOARD_SD_CS_GPIO, BOARD_SD_CS_GPIO_PIN, &sd_cs_config);
        _delay_ms(1);
		/*
		* Init memory
		*/
    	_masterXfer.rxData = &rxBuffer[0];
    	_masterXfer.txData = &txBuffer[0];

		/*
		* Register the device
		*/
    	uint32_t srcClock_Hz;
        /* Master config */
        masterConfig.whichCtar = kDSPI_Ctar1;
        masterConfig.ctarConfig.baudRate = SDLS_TRANSFER_BAUDRATE;
        masterConfig.ctarConfig.bitsPerFrame = 8U;
        masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh; //CPOL=0
        masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge; 	//
        masterConfig.ctarConfig.direction = kDSPI_MsbFirst;			// SD card is MSB
        masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 0;
        masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 0;
        masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 0;

        masterConfig.whichPcs = kDSPI_Pcs5;			// SPI1_PSC1
        masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;
        masterConfig.enableContinuousSCK = false;
        masterConfig.enableRxFifoOverWrite = false;
        masterConfig.enableModifiedTimingFormat = false;
        masterConfig.samplePoint = kDSPI_SckToSin0Clock;
        srcClock_Hz = CLOCK_GetFreq(SD_DSPI_MASTER_CLK_SRC);
        DSPI_MasterInit(SD_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

#if SPI_SD_DEBUG
		PRINTF("INIT SPI PORT for SD card completed\r\n");
		cfgState = 1;
#endif
    }
    return 1;
}

void reconfigure_spi(void){
	// time to switch clock to a higher frequency
	masterConfig.ctarConfig.baudRate = SDHS_TRANSFER_BAUDRATE;
	uint32_t srcClock_Hz;
	srcClock_Hz = CLOCK_GetFreq(SD_DSPI_MASTER_CLK_SRC);
	DSPI_MasterSetBaudRate(SD_DSPI_MASTER_BASEADDR, masterConfig.whichCtar, masterConfig.ctarConfig.baudRate, srcClock_Hz);
}

void close_spi_port(void){
	DSPI_Deinit(SD_DSPI_MASTER_BASEADDR);
}


#define INIT_PORT(drv) { init_port(drv); }  /* Initialize control port (CS=H, CLK=L, DI=H, DO=pu) */
#define DLY_US(n) { dly_us(n); }    /* Delay n microseconds */

#define CS_H(drv) { GPIO_SetPinsOutput(BOARD_SD_CS_GPIO, 1U << BOARD_SD_CS_GPIO_PIN);  }/* Set MMC CS 'H' */
#define CS_L(drv) { GPIO_ClearPinsOutput(BOARD_SD_CS_GPIO, 1U << BOARD_SD_CS_GPIO_PIN); } /* Set MMC CS 'L' */

/*--------------------------------------------------------------------------

   Module Private FuncSDHostInterfacetions

---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* Transmit 80 clocks		 			                                 */
/*-----------------------------------------------------------------------*/
void xmit_mmc_init_clks (BYTE drv
)
{

	_masterXfer.txData = &txBuffer[0];
	memset(txBuffer ,0xff ,SD_SPI_TRANSFER_SIZE);
	_masterXfer.dataSize = 10;
	_masterXfer.configFlags = kDSPI_MasterCtar1;
	DSPI_MasterTransferBlocking(SD_DSPI_MASTER_BASEADDR, &_masterXfer);

}

/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card 			                                 */
/*-----------------------------------------------------------------------*/

void xmit_mmc (BYTE drv,
  const BYTE buff[], 	 	/* Data to be sent */
  UINT bc        			/* Number of bytes to send */
)
{
	_masterXfer.txData = &txBuffer[0];
	_masterXfer.rxData = &rxBuffer[0];
	_masterXfer.dataSize = bc;
	memcpy((void*)_masterXfer.txData ,(void*)buff ,bc);
    /* Start master transfer */
	_masterXfer.configFlags = kDSPI_MasterCtar1;			//| SD_DSPI_MASTER_PCS | kDSPI_MasterPcsContinuous
    DSPI_MasterTransferBlocking(SD_DSPI_MASTER_BASEADDR, &_masterXfer);
}

/*-----------------------------------------------------------------------*/
/* Receive bytes from the card                          */
/*-----------------------------------------------------------------------*/

void rcvr_mmc (BYTE drv,
  BYTE buff[],  /* Pointer to read buffer */
  UINT bc    /* Number of bytes to receive */
)
{
	_masterXfer.rxData = &buff[0];
	_masterXfer.txData = &txBuffer[0];
	memset(_masterXfer.txData ,0xff ,bc);
    /* Start master transfer */
	_masterXfer.dataSize = bc;
	_masterXfer.configFlags = kDSPI_MasterCtar1;
    DSPI_MasterTransferBlocking(SD_DSPI_MASTER_BASEADDR, &_masterXfer);
}

/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

int wait_ready (BYTE drv)  /* 1:OK, 0:Timeout */
{
	  BYTE d[1];
	  UINT Tinit;

	  Tinit = millis();
	  do {
		  rcvr_mmc(drv, d, 1);
		  if((millis() - Tinit) > 500)
			  return 0;
	  }while((d[0] != 0xFF));

	return 1;
}

/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

void deselect (BYTE drv)
{
  BYTE d[1];

  CS_H(drv);
  rcvr_mmc(drv, d, 1);  /* Dummy clock (force DO hi-z for multiple slave SPI) */
}

/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/

int Select (BYTE drv)  /* 1:OK, 0:Timeout */
{
  BYTE d[1];

  CS_L(drv);
  rcvr_mmc(drv, d, 1);  /* Dummy clock (force DO enabled) */

  if (wait_ready(drv))
	  return 1;  /* OK */
  deselect(drv);
  return 0;      /* Failed */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the card                                   */
/*-----------------------------------------------------------------------*/

int rcvr_datablock (BYTE drv,  /* 1:OK, 0:Failed */
  BYTE *buff,      /* Data buffer to store received data */
  UINT btr      /* Byte count */
)
{
	BYTE d[2];

	memset(d, 0, 2);
	for (int tmr = 1000; tmr; tmr--)
	{  /* Wait for data packet in timeout of 100ms */
		rcvr_mmc(drv, d, 1);
		if (d[0] != 0xFF) break;
	}

	if (d[0] != 0xFE) return 0;    		// If not valid data token, return with error
	rcvr_mmc(drv, buff, btr);      		// Receive the data block into buffer
	rcvr_mmc(drv, d, 2);          		// Discard CRC


  return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the card                                        */
/*-----------------------------------------------------------------------*/

int xmit_datablock (BYTE drv,  /* 1:OK, 0:Failed */
  const BYTE *buff,  /* 512 byte data block to be transmitted */
  BYTE token      /* Data/Stop token */
)
{
	BYTE d[2];

	if (!wait_ready(drv)) return 0;

	d[0] = token;
	xmit_mmc(drv, d, 1);        /* Xmit a token */
	if (token != 0xFD)
	{    /* Is it data token? */
	xmit_mmc(drv, buff, 512);  /* Xmit the 512 byte data block to MMC */
	rcvr_mmc(drv, d, 2);      /* Xmit dummy CRC (0xFF,0xFF) */
	rcvr_mmc(drv, d, 1);      /* Receive data response */
	if ((d[0] & 0x1F) != 0x05)  /* If not accepted, return with error */
	  return 0;
	}
  return 1;
}

/*-----------------------------------------------------------------------*/
/* Send a command packet to the card                                     */
/*-----------------------------------------------------------------------*/

BYTE send_cmd (BYTE drv,    /* Returns command response (bit7==1:Send failed)*/
  BYTE cmd,    /* Command byte */
  DWORD arg    /* Argument */
)
{
  BYTE n, d[1], buf[6];

  if (cmd & 0x80)
  {  /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;

    n = send_cmd(drv, CMD55, 0);
    if (n > 1) return n;
  }

  /* Select the card and wait for ready */
  if(cmd != CMD12){							// please don't deselect if we are in multiple block reads - M.I. bug detected
	  deselect(drv);
	  if (!Select(drv)){
//		  PRINTF("could not select SD card\r\n");
		  return 0xFF;
	  }
  }

  /* Send a command packet */
  buf[0] = 0x40 | cmd;      /* Start + Command index */
  buf[1] = arg >> 24;    /* Argument[31..24] */
  buf[2] = arg >> 16;    /* Argument[23..16] */
  buf[3] = arg >> 8;    /* Argument[15..8] */
  buf[4] = arg;        /* Argument[7..0] */
  n = 0x01;            /* Dummy CRC + Stop */
  if (cmd == CMD0) n = 0x95;    /* (valid CRC for CMD0(0)) */
  if (cmd == CMD8) n = 0x87;    /* (valid CRC for CMD8(0x1AA)) */
  buf[5] = n;
  xmit_mmc(drv, buf, 6);

  /* Receive command response */
  if (cmd == CMD12) rcvr_mmc(drv, d, 1);  /* Skip a stuff byte when stop reading */
  n = 10;                /* Wait for a valid response in timeout of 10 attempts */
  do
    rcvr_mmc(drv, d, 1);
  while ((d[0] & 0x80) && --n);
  return d[0];      /* Return with the response value */
}
