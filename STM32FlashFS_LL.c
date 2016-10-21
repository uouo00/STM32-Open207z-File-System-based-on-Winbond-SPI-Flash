/*
 * STM32FlashFS.c
 *
 *  Created on: Jul 26, 2016
 *      Author: pwanhsu
 */
#include <stdlib.h>
#include <string.h>
#include "STM32FlashFS_LL.h"
#include "VATSDKDefine.h"
#include "VATDebug.h"
#include "STM32SPIImpl.h"
#include <stdint.h>
#include <stdbool.h>




SECTOR_INFO Sectormap[500];
static int nEmpty_Sector = 1024 - 1; // number of empty sectors left. Initially, 1024(total) - 1(bitmap)



#define NO_EMPTY_SECTOR 9999;
#define Bitmap_Size sizeof(bitmap) + sizeof(bitmap)/255




static uint8_t Sectormap_Addr[3]={ 0x00, 0x00, 0x00};



VATAPI_RESULT Sectormap_Init(){


	for(int i = 0;i < 500; i++){
		Sectormap[i].USAGE_STATUS = Unused;
		Sectormap[i].next_sector = 0;
	}
	/*Sector 0: sector map. Sector 1: File table. Sector 2:Director table*/
	Sectormap[0].USAGE_STATUS = Used;
	Sectormap[0].next_sector = END_OF_FILE;
	Sectormap[1].USAGE_STATUS = Used;
	Sectormap[1].next_sector = END_OF_FILE;
	Sectormap[2].USAGE_STATUS = Used;
	Sectormap[2].next_sector = END_OF_FILE;
	if(STM32SPIWriteSector(&Sectormap[0] , &Sectormap_Addr[0], sizeof(Sectormap)) == VAT_SUCCESS){

		DBGSTR("Successful Initial Bitmap");


		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Initial Bitmap Fail");
		return VAT_UNKNOWN;
	}

}

VATAPI_RESULT Load_Sectormap(){


	if(STM32SPIReadSector(&Sectormap[0], &Sectormap_Addr[0], sizeof(Sectormap)) == VAT_SUCCESS){
		//DBGSTR("Successful Load Bitmap");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Load Bitmap Fail");
		return VAT_UNKNOWN;
	}
}

uint32_t GETnEmpty_Sector(){

	 return nEmpty_Sector;
}

VATAPI_RESULT Save_Sectormap(){
	if(STM32SPIWriteSector(&Sectormap[0] , &Sectormap_Addr[0], sizeof(Sectormap)) == VAT_SUCCESS){
		//DBGSTR("Successfully Save Bitmap");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Save Bitmap Fail");
		return VAT_UNKNOWN;
	}
}

uint32_t Find_Empty_Sector(){

	//if(Load_Bitmap() == VAT_SUCCESS){
		int i = 0;

		for( i = 0; i < 500; i++)
			if(Sectormap[i].USAGE_STATUS == Unused)	return i; //found empty sector.

		if(i == 499) return VAT_UNKNOWN; //No Empty Sector.
	//}
	else return VAT_UNKNOWN;

}

VATAPI_RESULT Fail_Handler(){

}



int32_t Sector_Allocate( uint8_t *writebuf, uint32_t buff_size){

	/* calculate how much sector needed to store the certain file. */
	uint8_t SectorAddr[3] = { 0x00, 0x00, 0x00};

	if(Load_Sectormap() == VAT_SUCCESS){
		//write data to sector;
		uint16_t sectorid;

		sectorid = Find_Empty_Sector();

		if(sectorid == VAT_UNKNOWN){

			Fail_Handler();
			//erase sector already write.
			return VAT_UNKNOWN; //NO Empty Sector

		}
		else{
			SectorAddr[0] = (sectorid & 0x00000FF0) >> 4;
			SectorAddr[1] = (sectorid & 0x0000000F) << 4;
			SectorAddr[2] = 0x00;
			if(STM32SPIWriteSector(writebuf, SectorAddr, buff_size) == VAT_SUCCESS){
				DBGSTR("Write to Sector: %d Size: %d",sectorid,SPI_WinbondW25Q32FV_sectorByte);
			}

			Sectormap[sectorid].USAGE_STATUS = Used;
/*
			for(int i = 0; i < 20; i++){
				DBGSTR("usage: %d, next: %d",bitmap[i].USAGE_STATUS,bitmap[i].next_sector);
			}
			*/

			if(Save_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;
			else{
				nEmpty_Sector--;
				return sectorid;
			}

		}
	}else return VAT_UNKNOWN;


}

VATAPI_RESULT Sector_DeAllocate(uint32_t startsector , uint32_t endsector , uint32_t numSector){

	uint16_t erase_sector = startsector;
	uint8_t SectorAddr[3];
	uint8_t next_erase_sector;

	if(Load_Sectormap() != VAT_SUCCESS)
		return VAT_UNKNOWN;
/*
	for(int i = 0; i < 40; i++){
		DBGSTR("usage: %d, next: %d",Sectormap[i].USAGE_STATUS,Sectormap[i].next_sector);
	}*/

	for(int i = 0; i < numSector; i++){

		SectorAddr[0] = (erase_sector & 0x00000FF0) >> 4;
		SectorAddr[1] = (erase_sector & 0x0000000F) << 4;
		SectorAddr[2] = 0x00;
		if(STM32SPIEraseSector(&SectorAddr[0]) != VAT_SUCCESS){
			Fail_Handler();
		}
		else{

			DBGSTR("ERASE SECTOR - 0x%2x %2x %2x", SectorAddr[0], SectorAddr[1], SectorAddr[2]);

		}
		Sectormap[erase_sector].USAGE_STATUS = Unused;
		next_erase_sector = Sectormap[erase_sector].next_sector;
		Sectormap[erase_sector].next_sector = 0;
		erase_sector = next_erase_sector;
	}
	/*
	for(int i = 0; i < 40; i++){
			DBGSTR("usage: %d, next: %d",Sectormap[i].USAGE_STATUS,Sectormap[i].next_sector);
		}
		*/
	if(Save_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{
		nEmpty_Sector = nEmpty_Sector + numSector;
		return VAT_SUCCESS;
	}

	//erase the sector;
	//bitmap[sectorid] = Unused;


	return VAT_SUCCESS;
}


// WHAT IF FLASD MAP FILE. SOLUTION: 2 COPY OF FLASH MAP
// WHAT IF WRITE AND HALT. CLEAR. POOLING
// WHAT IF CLEAR AND HALT. CLEAR.





