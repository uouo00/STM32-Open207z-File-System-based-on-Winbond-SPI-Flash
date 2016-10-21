
#ifndef __WinbondW25Q32FV_STM32SPI
#define __WinbondW25Q32FV_STM32SPI

#include <STM32Impl.h>
#include <VATSDKDefine.h>
#include "STM32USB_hostImpl.h"

#define SPI_WinbondW25Q32FV_writeEnable 0x06
#define SPI_WinbondW25Q32FV_writeDisable 0x04
#define SPI_WinbondW25Q32FV_Page_Program 0x02
#define SPI_WinbondW25Q32FV_eraseChip 0x60
#define SPI_WinbondW25Q32FV_eraseSector 0x20
#define SPI_WinbondW25Q32FV_readManuDeviceID 0x90
#define SPI_WinbondW25Q32FV_readStatus_1 0x05
#define SPI_WinbondW25Q32FV_readStatus_2 0x35
#define SPI_WinbondW25Q32FV_readStatus_3 0x15
#define SPI_WinbondW25Q32FV_readData 0x03
#define SPI_WinbondW25Q32FV_DUMMY 0x00

#define SPI_WinbondW25Q32FV_sec0_ERROR 0xF0
#define SPI_WinbondW25Q32FV_sec1_ERROR 0x0F
#define SPI_WinbondW25Q32FV_sec0_startAddr 0x00000000	//record flash usable status
#define SPI_WinbondW25Q32FV_sec1_startAddr 0x00001000	//record flash usable status(backup)
#define SPI_WinbondW25Q32FV_flash_lastAddr 0x3FFFFF
#define SPI_WinbondW25Q32FV_sectorByte 4096	// each sector content 4096 KB
#define SECTOR_NUM 1024						// 16(sector)*64(block)
#define Page_bufferSize 256

#define SPI_WinbondW25Q32FV_TYPE_all 				0x00000000
#define SPI_WinbondW25Q32FV_TYPE_multiChannelTable 	0x02000000
#define SPI_WinbondW25Q32FV_TYPE_reserved 			0x04000000
#define SPI_WinbondW25Q32FV_TYPE_VATProperty 		0x06000000
#define SPI_WinbondW25Q32FV_TYPE_CustomizeProperty 	0x08000000
#define SPI_WinbondW25Q32FV_TYPE_WEBPAGE			0x0A000000
#define SPI_WinbondW25Q32FV_TYPE_unknown 			0xFE000000
#define SPI_WinbondW25Q32FV_TYPE_empty 				0xFF000000
#define SPI_WinbondW25Q32FV_sec_check 0x78	// sector check sum

#define SPI_202WinbondW25Q32BV_TYPE_all 			0x00000000

#define DEF_CUSTOMIZE_RESERVED 		0x00000000
#define MANUAL_FREQSETTING			0x00000001

enum{
	STM32EraseFlash_BootloaderSectors = 0,
	STM32EraseFlash_IAPSectors = 1,
	STM32EraseFlash_APPSectors = 2,
	STM32EraseFlash_202Sectors = 3,
}FlashSectorNum;

typedef struct _SPIProperties_Customize
{
	uint8_t channelNum[16];
	uint8_t Country[64];
	uint8_t LCD_Timeout[6];
	uint8_t OutputLevel[6];
	uint8_t HDCP[2];
	uint8_t EP9555E_Control[2];
	uint32_t reserved;
}SPIProperties_Customize,*PSPIProperties_Customize;

#define DEF_CUSTOMIZE_COUN "AUSTRALIA"
#define DEF_CUSTOMIZE_TIMEOUT "180"
#define DEF_CUSTOMIZE_OUTLE "85"
#define DEF_CUSTOMIZE_HDCP "1"				// off/on 		: 0/1
#define DEF_CUSTOMIZE_EPCONTROL "1"			// 1080p/720p 	: 0/1
#define DEF_CUSTOMIZE_OUTPUTLEVEL "85"
#define CUSTOMIZE_TXT "\
channelNum,%s,\
country,%s,\
timeout,%s,\
outputlevel,%s,\
HDCP,%s,\
EP9555E_Control,%s,\
reserved,%d,"





VATAPI_RESULT STM32SPI_FindTargetDataInFlash(uint32_t dataType, uint8_t *data, uint32_t size);
VATAPI_RESULT STM32SPIWriteFlag_For202(uint8_t *flag_value);
VATAPI_RESULT STM32SPI_Init_flashMap();
VATAPI_RESULT STM32SPIEraseChip();
VATAPI_RESULT STM32SPIReadFlag_For202(uint8_t *flag_value);
uint8_t* STM32SPIreadDeviceManu();
VATAPI_RESULT STM32SPIReadMem(void *readbuf, uint8_t *readaddr, uint32_t buff_size);
VATAPI_RESULT STM32SPIWriteSector(void *writebuf, uint8_t* SectorAddr, uint32_t buff_size);
VATAPI_RESULT STM32SPIReadSector(void *readbuf, uint8_t *readaddr, uint32_t buff_size);
#endif







