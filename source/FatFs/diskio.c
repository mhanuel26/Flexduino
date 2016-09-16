/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sdCardSPI.h"
#include "fsl_dspi.h"
#include "fsl_debug_console.h"

/* Definitions of physical drive number for each drive */
#define ATA		1	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

extern SDHostInterface  SDif;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	BYTE d[1];
	DSTATUS s;

	stat = STA_NOINIT;
	switch (pdrv) {
	case ATA :
		break;

	case MMC :
		  /* Check if the card is kept initialized */
		  s = SDif.Stat;
		  if (!(s & STA_NOINIT))
		  {
			  d[0] = send_cmd(pdrv, CMD13, 0);
			  if (d[0]){  /* Read card status */
				  s = STA_NOINIT;
			  }
			  rcvr_mmc(pdrv, d, 1);    /* Receive following half of R2 */
			  deselect(pdrv);
		  }
		  SDif.Stat = s;
		  stat = s;
		break;

	case USB :
		break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	DSTATUS 	stat;
	BYTE 		n, ty, cmd, buf[4], debug_buf[4];
	UINT 		tmr;
	DSTATUS 	s;

	stat = STA_NOINIT;
	memset(buf, 0, 4);

	switch (pdrv) {
		case ATA :
			break;

		case MMC :
			  if(pdrv >= sizeof(SDif)/sizeof(SDHostInterface)) return RES_NOTRDY;
			  if(!init_spi_port()) 	return 	STA_NOINIT;			//INIT_PORT(pdrv);        /* Initialization pdrv control port */
//			  for (n = 10; n; n--) rcvr_mmc(pdrv, buf, 1);  /* 80 dummy clocks */
			  xmit_mmc_init_clks(pdrv);
			  ty = 0;
			  if (send_cmd(pdrv, CMD0, 0) == 1) {       //Enter Idle state
				if (send_cmd(pdrv, CMD8, 0x1AA) == 1) {   //SDv2?
					rcvr_mmc(pdrv, buf, 4);               //Get trailing return value of R7 resp
//#if SPI_SD_DEBUG
//					PRINTF("R7 frame buffer\r\n");
//					PRINTF("0x%02X 0x%02X 0x%02X 0x%02X \r\n", buf[0], buf[1], buf[2], buf[3]);
//#endif
					if ((buf[2] == 0x01) && (buf[3] == 0xAA)) {     //The card can work at vdd range of 2.7-3.6V
						for (tmr = 1000; tmr; tmr--) {       //Wait for leaving idle state (ACMD41 with HCS bit)
						  if (send_cmd(pdrv, ACMD41, 1UL << 30) == 0) break;
						  dly_us(1000);
						}
						if (tmr && send_cmd(pdrv, CMD58, 0) == 0) {   //Check CCS bit in the OCR
						  rcvr_mmc(pdrv, buf, 4);
						  ty = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;   //SDv2
						}
					}
				} else {               //SDv1 or MMCv3
				  if (send_cmd(pdrv, ACMD41, 0) <= 1)   {
					ty = CT_SD1; cmd = ACMD41;   //SDv1
				  } else {
					ty = CT_MMC; cmd = CMD1;   //MMCv3
				  }
				  for (tmr = 1000; tmr; tmr--) {       //Wait for leaving idle state
					if (send_cmd(pdrv, ACMD41, 0) == 0) break;
					dly_us(1000);
				  }
				  if (!tmr || send_cmd(pdrv, CMD16, 512) != 0)   //Set R/W block length to 512
					ty = 0;
				}
			  }
			  SDif.CardType = ty;
			  s = ty ? 0 : STA_NOINIT;
			  SDif.Stat = s;

			  deselect(pdrv);
			  reconfigure_spi();
			  stat = s;
			break;

		case USB:
			break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
		case ATA:
			break;

		case MMC :
			if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;
			if (!count) return RES_PARERR;
			if (!(SDif.CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */
			if (count == 1) {  /* Single block read */
				if ((send_cmd(pdrv, CMD17, sector) == 0)  /* READ_SINGLE_BLOCK */
				  && rcvr_datablock(pdrv, buff, 512))
				  count = 0;
				}else{        /* Multiple block read */
					if (send_cmd(pdrv, CMD18, sector) == 0) {  /* READ_MULTIPLE_BLOCK */
						do {
							if (!rcvr_datablock(pdrv, buff, 512)) break;
							buff += 512;
						} while (--count);
						send_cmd(pdrv, CMD12, 0);        /* STOP_TRANSMISSION */
					}
			}
			deselect(pdrv);
			return count ? RES_ERROR : RES_OK;

		case USB :
			break;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
	case ATA :
		break;

	case MMC :
		  if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;
		  if (!count) return RES_PARERR;
		  if (!(SDif.CardType & CT_BLOCK)) sector *= 512;  /* Convert LBA to byte address if needed */

		  if (count == 1) {  /* Single block write */
			if ((send_cmd(pdrv, CMD24, sector) == 0)  /* WRITE_BLOCK */
			  && xmit_datablock(pdrv, buff, 0xFE))
			  count = 0;
		  }
		  else {        /* Multiple block write */
			if (SDif.CardType & CT_SDC) send_cmd(pdrv, ACMD23, count);
			if (send_cmd(pdrv, CMD25, sector) == 0) {  /* WRITE_MULTIPLE_BLOCK */
			  do {
		        if (!xmit_datablock(pdrv, buff, 0xFC)) break;
		        buff += 512;
			  } while (--count);
		      if (!xmit_datablock(pdrv, NULL, 0xFD))  /* STOP_TRAN token */
				count = 1;
			}
		  }
		  deselect(pdrv);
		  return count ? RES_ERROR : RES_OK;

	case USB :
		break;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	char *buff		/* Buffer to send/receive control data */
)
{
	BYTE 	n, i, csd[16];
	WORD 	cs;
	DRESULT res;
	DWORD 	Val;
	BYTE  	*Ptr;

	switch (pdrv) {
	case ATA :
		break;

	case MMC :
		  if (disk_status(pdrv) & STA_NOINIT) return RES_NOTRDY;  /* Check if card is in the socket */
		  res = RES_ERROR;
		  switch (cmd) {
			case CTRL_SYNC:    					/* Make sure that no pending write process */
			  if (Select(pdrv)) {
				deselect(pdrv);
				res = RES_OK;
			  }
			break;
			case GET_SECTOR_COUNT :  			/* Get number of sectors on the disk (DWORD) */
			  if ((send_cmd(pdrv, CMD9, 0) == 0) && rcvr_datablock(pdrv, csd, 16)) {
				if ((csd[0] >> 6) == 1) {  		/* SDC ver 2.00 */
				  cs= csd[9] + ((WORD)csd[8] << 8) + 1;
				  //*(DWORD*)buff = (DWORD)cs << 10;
				  cs <<= 10;
				  Ptr = (BYTE*)&Val;
				  for(Val = cs, i = 0; i < sizeof(DWORD); i++){
					buff[i] = *Ptr++;
				  }
				} else {          /* SDC ver 1.XX or MMC */
				  n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				  cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				  //*(DWORD*)buff = (DWORD)cs << (n - 9);
				  cs <<= (n - 9);
				  Ptr = (BYTE*)&Val;
				  for(Val = cs, i = 0; i < sizeof(DWORD); i++){
					buff[i] = *Ptr++;
				  }
				}
				res = RES_OK;
			  }
			break;
			case GET_BLOCK_SIZE :  /* Get erase block size in unit of sector (DWORD) */
				Ptr = (BYTE*)&Val;
				for(Val = 128, i = 0; i < sizeof(DWORD); i++)
					buff[i] = *Ptr++;
				res = RES_OK;
			break;

			default:
			  res = RES_PARERR;
			break;
		  }
		  deselect(pdrv);
		  return res;

	case USB :
		break;
	}

	return RES_PARERR;
}

