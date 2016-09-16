#ifndef _SDCARDSPI_DEFINED
#define _SDCARDSPI_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#define SD_DEBUG		1
#define SPI_SD_DEBUG	1
#define SDHS_TRANSFER_BAUDRATE 25000000U /*! Transfer baudrate - 25M */
#define SDLS_TRANSFER_BAUDRATE 100000U /*! Transfer baudrate - 100k */
#define SD_DSPI_MASTER_PCS kDSPI_MasterPcs5		// don't care
#define SD_DSPI_MASTER_BASEADDR SPI1
#define SD_DSPI_MASTER_CLK_SRC 	DSPI1_CLK_SRC
#define SD_SPI_TRANSFER_SIZE 	512U        /*! Transfer dataSize */

#include "diskio.h"
// K82F port include
#include "board.h"

/* MMC/SD command (SPI mode) */
#define CMD0   (0)     /* GO_IDLE_STATE */
#define CMD1   (1)     /* SEND_OP_COND */
#define ACMD41 (0x80+41) /* SEND_OP_COND (SDC) */
#define CMD8   (8)     /* SEND_IF_COND */
#define CMD9   (9)     /* SEND_CSD */
#define CMD10  (10)    /* SEND_CID */
#define CMD12  (12)    /* STOP_TRANSMISSION */
#define CMD13  (13)    /* SEND_STATUS */
#define ACMD13 (0x80+13) /* SD_STATUS (SDC) */
#define CMD16  (16)    /* SET_BLOCKLEN */
#define CMD17  (17)    /* READ_SINGLE_BLOCK */
#define CMD18  (18)    /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)    /* SET_BLOCK_COUNT */
#define ACMD23 (0x80+23)  /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)    /* WRITE_BLOCK */
#define CMD25  (25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)    /* SEND_OP_COND (ACMD) */
#define CMD55  (55)    /* APP_CMD */
#define CMD58  (58)    /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC    0x01    /* MMC ver 3 */
#define CT_SD1    0x02    /* SD ver 1 */
#define CT_SD2    0x04    /* SD ver 2 */
#define CT_SDC    (CT_SD1|CT_SD2)  /* SD */
#define CT_BLOCK  0x08    /* Block addressing */

#define dly_us 	_delay_us
#define dly_ms 	_delay_ms
#define millis _millis

void xmit_mmc_init_clks (BYTE drv);
void rcvr_mmc (BYTE drv, BYTE buff[], UINT bc);
BYTE send_cmd(BYTE drv, BYTE cmd, DWORD arg );
int xmit_datablock(BYTE drv, const BYTE *buff, BYTE token );
int rcvr_datablock(BYTE drv, BYTE *buff, UINT btrv );
int Select (BYTE drv);
void deselect (BYTE drv);
int wait_ready (BYTE drv);
int init_spi_port(void);
void reconfigure_spi(void);
void close_spi_port(void);


typedef struct SDHostInterface
{
/*
     C M     S   M
     s o     c   i
       s     l   s
       i     k   o
   __________________
  /  | | | | | | | | |
  || C D - + C - D   |
  |  S I G 3 L G O   |
  |      N . K N     |
  |      D 3   D     |
  |        V         |
*/
  /* fields returned after initialization */
  BYTE CardType; /* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */
  DSTATUS Stat; /* Disk status */
} SDHostInterface;

#ifdef __cplusplus
}
#endif

#endif
