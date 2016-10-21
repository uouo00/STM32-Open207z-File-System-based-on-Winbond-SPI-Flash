/**
 * @file STM32FlashFS_LL.h
 * @author Max Hsu
 * @brief STM32 Flash File System Low Level Sector Layer
 *
 * The File system allocate storage in the unit of a sector(4096 bytes). Thus, we need to maintain a
 * Sector map for a total of 1024 sectors of SPI Flash. In the layer, we provide initialization of
 * sector map and Save/Load the sector map between RAM and Flash by the api of spi flash r/w layer.
 * Also, this layer also provide function which return an available sector when a file is created and
 * need sector to storage. On the other hand, when a file is deleted, it deallocates the sector and
 * set available of the sectors in sector map.
 *
 *
 * Foundation, VATek.
 * Copyright (c) 2016 VATek. All rights reserved.
 */
#include <stdint.h>
#include <VATSDKDefine.h>
#include <stdbool.h>

#ifndef INCLUDES_STM32FLASHFS_LL_H_
#define INCLUDES_STM32FLASHFS_LL_H_




typedef struct _SECTOR_INFO{
	uint8_t USAGE_STATUS; //indicate a sector is used or unused. used: 0, unused: 1
	uint32_t next_sector; //indicate the next sector to r/w when access a file.
}SECTOR_INFO,PSECTOR_INFO;


#define Unused 0
#define Used 1
#define	NO_PREVIOUS 0x401
#define END_OF_FILE 0x402



extern SECTOR_INFO Sectormap[500];


/**
  * @brief Initial Sector Map
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Sectormap_Init();
/**
  * @brief Load Sector map to RAM
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Load_Sectormap();
/**
  * @brief Save Sector Map back to flash
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Save_Sectormap();
/**
  * @brief Get number of total available sector.
  *
  * @retval number of total available sector
  */
uint32_t GETnEmpty_Sector();
/**
  * @brief Get the id(number) of available sector
  *
  * @retval id of an available sector
  */
uint32_t Find_Empty_Sector();
VATAPI_RESULT Fail_Handler();
/**
  * @brief Allocated a sector and write data in it.
  *
  * @param  *writebuf Pointer to the buffer to write into the sector
  * @param  *buff_size Size of buffer to write into the sector
  * @retval Allocated Sector id
  */
int32_t Sector_Allocate( uint8_t *writebuf, uint32_t buff_size);
VATAPI_RESULT Sector_DeAllocate(uint32_t startsector , uint32_t endsector , uint32_t numSector);


#endif /* INCLUDES_STM32FLASHFS_LL_H_ */
