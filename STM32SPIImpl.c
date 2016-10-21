
#include <stdio.h>
#include <stdlib.h>
#include <STM32SPIImpl.h>
#include "STM32FlashFS_Logical.h"
#include "STM32FlashFS_FF.h"

#define STM32_NSS_PIN_PORT GPIOA
uint16_t STM32_NSS_PIN = GPIO_PIN_4;

#define VAT202_NSS_PIN GPIO_PIN_9
#define VAT202_NSS_PIN_PORT GPIOB

#define CHECK_ADDR(a,b) (a[0]==b[0] && a[1]==b[1] && a[2]==b[2])

uint8_t buffDeviceManu[2];
uint8_t buffRegStatus[3];

static SPI_HandleTypeDef* STM32SPIEntry = NULL;
static SPI_HandleTypeDef* STM32VATSPIEntry = NULL;









VATAPI_RESULT STM32SPIPageProgram(uint8_t *writebuf, uint8_t* writeaddr, uint32_t buff_size);
/*
ADRF6755_I2C ADRF6755_I2CEntry =
{
	.ReadReg = STM32ReadReg,
	.WriteReg = STM32WriteReg,
	.ReadMem = STM32ReadMem,
	.WriteMem = STM32WriteMem,
};
*/

void STM32InitSPI_NSSPin(){
	GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = STM32_NSS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(STM32_NSS_PIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin ((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN, GPIO_PIN_SET);
}

void VAT202InitSPI_NSSPin(){
	GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = VAT202_NSS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(VAT202_NSS_PIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin ((GPIO_TypeDef*) VAT202_NSS_PIN_PORT, VAT202_NSS_PIN, GPIO_PIN_SET);
}

//STM32 SPI FLASH
void STM32GetSPIHandler(SPI_HandleTypeDef* SPI_Struct)
{
	if(STM32SPIEntry == NULL)
		STM32SPIEntry = SPI_Struct;

	STM32InitSPI_NSSPin();
}

void NSS_Pin_high(){
	HAL_GPIO_WritePin ((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN, GPIO_PIN_SET);
}
void NSS_Pin_low(){
	HAL_GPIO_WritePin ((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN, GPIO_PIN_RESET);
}

//VAT202 SPI FLASH
void VAT202_start_update(){
	STM32_NSS_PIN=GPIO_PIN_9;
	STM32SPIEntry = STM32VATSPIEntry;	//debug
	//DBGSTR("SPI3 : %p",STM32SPIEntry);
	HAL_GPIO_WritePin ((GPIO_TypeDef*) POWER_ON_202_GPIO_Port, POWER_ON_202_Pin, GPIO_PIN_RESET);
	//DBGSTR("202 %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) POWER_ON_202_GPIO_Port, POWER_ON_202_Pin));
	HAL_Delay(50);

	HAL_GPIO_WritePin ((GPIO_TypeDef*) SPI_UPDATE_EN_GPIO_Port, SPI_UPDATE_EN_Pin, GPIO_PIN_SET);
	//DBGSTR("UPDATE %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) SPI_UPDATE_EN_GPIO_Port, SPI_UPDATE_EN_Pin));

}
void STM32GetVATSPIHandler(SPI_HandleTypeDef* SPI_Struct)
{
	if(STM32VATSPIEntry == NULL)
		STM32VATSPIEntry = SPI_Struct;

	VAT202InitSPI_NSSPin();
}
void VAT202_NSS_Pin_high(){
	HAL_GPIO_WritePin ((GPIO_TypeDef*) VAT202_NSS_PIN_PORT, VAT202_NSS_PIN, GPIO_PIN_SET);
}
void VAT202_NSS_Pin_low(){
	HAL_GPIO_WritePin ((GPIO_TypeDef*) VAT202_NSS_PIN_PORT, VAT202_NSS_PIN, GPIO_PIN_RESET);
}


/**
* @brief Read Manufacturer/Device ID
*
* The instruction is initiated by driving the /CS pin low and shifting the instruction code “90h”
* followed by a 24-bit address (A23-A0) of 000000h. After which, the Manufacturer ID for Winbond (EFh)
* and the Device ID are shifted out on the falling edge of CLK with most significant bit (MSB) first as
* shown in Figure 39. The Device ID values for the W25Q32FV are listed in Manufacturer and Device
* Identification table. The instruction is completed by driving /CS high.
*
*/
uint8_t* STM32SPIreadDeviceManu()
{
	HAL_StatusTypeDef nres;
	uint8_t command[6] = {SPI_WinbondW25Q32FV_readManuDeviceID, 0x00,0x00,0x00, SPI_WinbondW25Q32FV_DUMMY,SPI_WinbondW25Q32FV_DUMMY};

	NSS_Pin_low();
	//DBGSTR("GOIP NSS = %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) GPIOE, GPIO_PIN_10));

	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buffDeviceManu[0], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[1], &buffDeviceManu[0], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[2], &buffDeviceManu[0], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[3], &buffDeviceManu[0], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[4], &buffDeviceManu[0], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[5], &buffDeviceManu[1], 1, 10)) != HAL_OK)
			DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);

	NSS_Pin_high();
	//DBGSTR("GOIP NSS = %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) GPIOE, GPIO_PIN_10));
	DBGSTR("Manufacture=%x, DeviceID=%x",buffDeviceManu[0],buffDeviceManu[1]);

	return &buffDeviceManu[0];
}

/**
* @brief Read SPI Status
*
* The Read Status Register instruction may be used at any time, even while a Program,
* Erase or Write Status Register cycle is in progress. This allows the BUSY status bit
* to be checked to determine when the cycle is complete and if the device can accept
* another instruction. The Status Register can be read continuously, as shown in Figure 8.
* The instruction is completed by driving /CS high.
*
*/
uint8_t* STM32SPIReadStatus()
{
	HAL_StatusTypeDef nres;
	uint8_t command[4] = {SPI_WinbondW25Q32FV_readStatus_1, SPI_WinbondW25Q32FV_readStatus_2, SPI_WinbondW25Q32FV_readStatus_3, SPI_WinbondW25Q32FV_DUMMY};

	NSS_Pin_low();
	//DBGSTR("CS %d, STM32_NSS_PIN:%d, PA15:%d, PA4:%d",HAL_GPIO_ReadPin((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN),STM32_NSS_PIN,GPIO_PIN_15,GPIO_PIN_4);

	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buffRegStatus[0], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[3], &buffRegStatus[0], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);

	NSS_Pin_high();
	NSS_Pin_low();

	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[1], &buffRegStatus[1], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[3], &buffRegStatus[1], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);

	NSS_Pin_high();
	NSS_Pin_low();
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[2], &buffRegStatus[2], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[3], &buffRegStatus[2], 1, 100)) != HAL_OK)
		DBGSTR("STM32SPIreadDeviceManu : send command fail! %d",nres);

	NSS_Pin_high();
	//DBGSTR("CS %d, STM32_NSS_PIN:%d, PA15:%d, PA4:%d",HAL_GPIO_ReadPin((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN),STM32_NSS_PIN,GPIO_PIN_15,GPIO_PIN_4);

	//HAL_Delay(100);
	//DBGSTR("Register status1=%x, status2=%x, status3=%x",buffRegStatus[0],buffRegStatus[1],buffRegStatus[2]);

	return &buffRegStatus[0];
}

/**
* @brief Loop until BUSY bit(In status register) are set to 0.
*
*/
void wait_for_BUSY(){
	uint8_t *regStatus1;

	do{
		regStatus1 = STM32SPIReadStatus();
	}while((regStatus1[0]&0x01) == 0x01);	//wait for BUSY bit
}


/**
* @brief Set WEL Bit to 1
*
* WEL Bit must be set to 1 before Page Program,
* Quad Page Program, Sector Erase, Block Erase, Chip Erase,
* Write Status Register and Erase/Program Security Registers instruction
*
*/
void write_enable(){
	HAL_StatusTypeDef nres;
	uint8_t command[1] = {SPI_WinbondW25Q32FV_writeEnable};
	uint8_t buff[2] = "";
	NSS_Pin_low();
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("Write_enable : send command fail! %d",nres);
	NSS_Pin_high();
}

/**
* @brief Set WEL Bit to 0
*
* WEL Bit must be set to 0 after Page Program,
* Quad Page Program, Sector Erase, Block Erase, Chip Erase,
* Write Status Register and Erase/Program Security Registers instruction
*
*/
void write_disable(){
	HAL_StatusTypeDef nres;
	uint8_t command[1] = {SPI_WinbondW25Q32FV_writeDisable};
	uint8_t buff[2] = "";

	NSS_Pin_low();
	DBGSTR("CS~ %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN));
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("Write_disable : send command fail! %d",nres);
	NSS_Pin_high();
	DBGSTR("CS~ %d",HAL_GPIO_ReadPin((GPIO_TypeDef*) STM32_NSS_PIN_PORT, STM32_NSS_PIN));
}

/**
* @brief  sets all memory within the device to the erased state of all 1s (FFh).
*
*
*/
VATAPI_RESULT STM32SPIEraseChip()
{
	DBGSTR("Start to erase flash, wait for a while~");
	HAL_StatusTypeDef nres;
	uint8_t command[1] = {SPI_WinbondW25Q32FV_eraseChip}, buff[2];

	write_enable();
	wait_for_BUSY();
	NSS_Pin_low();
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK){
		DBGSTR("STM32SPIEraseChip : send command fail! %d",nres);
		return VAT_UNKNOWN;
	}
	NSS_Pin_high();
	wait_for_BUSY();
	DBGSTR("Finish to erase flash~~");

	return VAT_SUCCESS;
}


/**
* @brief sets all memory within a specified sector (4K-bytes) to the erased state of all 1s (FFh).
*
* @param *secAddr Address of specified sector.
*/
VATAPI_RESULT STM32SPIEraseSector(uint8_t *secAddr)
{
	HAL_StatusTypeDef nres;
	uint8_t addr[3] = {secAddr[0], secAddr[1], secAddr[2]};
	uint8_t command[1] = {SPI_WinbondW25Q32FV_eraseSector}, buff[2] = "";

	//DBGSTR("Start to erase the sector[0x%2x%2x%2x] of flash, wait for a while~",addr[0],addr[1],addr[2]);
	write_enable();
	wait_for_BUSY();
	NSS_Pin_low();

	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIEraseSector : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIEraseSector : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[1], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIEraseSector : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[2], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIEraseSector : send command fail! %d",nres);

	NSS_Pin_high();
	wait_for_BUSY();

	if(nres == HAL_OK){
		//DBGSTR("Finish to erase the sector[0x%2x%2x%2x] of flash~~",addr[0],addr[1],addr[2]);
		return VAT_SUCCESS;
	}

	return VAT_UNKNOWN;
}

/**
* @brief Read Data from specified sector
*
*
* @param *readbuf pointer to the buffer to store the data to be read.
* @param *readaddr pointer to the address of specified sector.
* @param buf_size size of buffer.
*/
VATAPI_RESULT STM32SPIReadSector(void *readbuf, uint8_t *readaddr, uint32_t buff_size){
	HAL_StatusTypeDef nres;
	uint8_t command[2] = {SPI_WinbondW25Q32FV_readData,SPI_WinbondW25Q32FV_DUMMY}, buff[2] = "";
	uint8_t addr[3] = {readaddr[0], readaddr[1], readaddr[2]};
	uint32_t i = 1, j = 0, k = 0;
	uint8_t* drop = pvPortMalloc(40 * sizeof(uint8_t));


	NSS_Pin_low();
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[1], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[2], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);

	if(nres != HAL_OK){
		vPortFree(drop);
		return VAT_UNKNOWN;
	}

	buff_size = buff_size + buff_size / 255;

	while(i <= buff_size){

		if((i % 256 != 0)){
			if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[1], &readbuf[j], 1, 10)) != HAL_OK){
				DBGSTR("STM32SPIReadMem : send command [%d] fail! %d",nres);
				vPortFree(drop);
				return VAT_UNKNOWN;
			}
			j++;
		}
		else{
			if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[1], &drop[k], 1, 10)) != HAL_OK){
				DBGSTR("STM32SPIReadMem : send command [%d] fail! %d",nres);
				vPortFree(drop);
				return VAT_UNKNOWN;
			}
			k++;
			}
		i++;
	}
	NSS_Pin_high();

	vPortFree(drop);
	return VAT_SUCCESS;
}


VATAPI_RESULT STM32SPIReadMem(void *readbuf, uint8_t *readaddr, uint32_t buff_size)
{
	HAL_StatusTypeDef nres;
	uint8_t command[2] = {SPI_WinbondW25Q32FV_readData,SPI_WinbondW25Q32FV_DUMMY}, buff[2] = "";
	uint8_t addr[3] = {readaddr[0], readaddr[1], readaddr[2]};
	uint32_t i = 0;

	NSS_Pin_low();
	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[1], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[2], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIReadMem : send command fail! %d",nres);

	if(nres != HAL_OK)
		return VAT_UNKNOWN;
	while(i < buff_size){


		if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[1], &readbuf[i], 1, 10)) != HAL_OK){
			DBGSTR("STM32SPIReadMem : send command [%d] fail! %d",nres);
			return VAT_UNKNOWN;
		}

		//DBGSTR("STM32SPIReadMem - 0x%2x%2x%2x[%d]=%x",addr[0], addr[1], addr[2],i,readbuf[i]);
		i++;
	}
	NSS_Pin_high();

	return VAT_SUCCESS;
}




/*
* @brief Write data to sector
*
* The Page Program instruction allows from one byte to 256 bytes (a page)
*  of data to be programmed at previously erased (FFh) memory locations.
*
* @param *writebuf pointer to buffer which contents data to be written into sector.
* @param *writeaddr pointer to address of sector to be written.
* @param buff_size size of data to be written into sector.
*/
VATAPI_RESULT STM32SPIPageProgram(uint8_t *writebuf, uint8_t* writeaddr, uint32_t buff_size)
{
	HAL_StatusTypeDef nres;
	uint8_t command[2] = {SPI_WinbondW25Q32FV_Page_Program,SPI_WinbondW25Q32FV_DUMMY}, buff[2] = "";
	uint8_t addr[3] = {writeaddr[0], writeaddr[1], writeaddr[2]};
	uint32_t i = 0;

	///!!!!
	//DBGSTR("Start to program the start address[0x%2x%2x%2x] of flash, wait for a while~",addr[0],addr[1],addr[2]);
	if(buff_size >= Page_bufferSize){		//sector 0,1 for flash usable record
		buff_size = Page_bufferSize;
		writebuf[buff_size-1] = 0;
	}
	//writebuf[buff_size-1] = 0;

	write_enable();
	wait_for_BUSY();
	NSS_Pin_low();

	if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &command[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIPageProgram : send command fail! %d",nres);

	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[0], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIPageProgram : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[1], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIPageProgram : send command fail! %d",nres);
	if((nres |= HAL_SPI_TransmitReceive(STM32SPIEntry, &addr[2], &buff[0], 1, 10)) != HAL_OK)
		DBGSTR("STM32SPIPageProgram : send command fail! %d",nres);

	if(nres != HAL_OK)
		return VAT_UNKNOWN;
	while(i < buff_size){
		if((nres = HAL_SPI_TransmitReceive(STM32SPIEntry, &writebuf[i], &buff[0], 1, 10)) != HAL_OK){
			DBGSTR("STM32SPIPageProgram : send command fail! %d",nres);
			NSS_Pin_high();
			wait_for_BUSY();
			return VAT_UNKNOWN;
		}
		//DBGSTR("STM32SPIPageProgram [%d]=0x%x, read=0x%x",i,writebuf[i],buff[0]);
		i++;
	}
	NSS_Pin_high();
	wait_for_BUSY();

	return VAT_SUCCESS;
}



/**
* @brief Write MultiPage to flash.
*
* Page Program allows only 256 bytes to be written in once. Hence, if to write data more than 256
* bytes. we have to divide write execution into multiple 255-bytes-write(see note below).
*
* @param *writebuf pointer to buffer which contents data to be written into sector.
* @param *writeaddr pointer to address of sector to be written.
* @param buff_size size of data to be written into sector.
*/
//uint8_t sectorBuf[4096] = "";
//uint8_t pagebuf[255]= "";

VATAPI_RESULT STM32SPIProgram_multiPage(uint8_t *writebuf, uint8_t *writeAddr, uint32_t buff_size){
	VATAPI_RESULT nres;
	uint8_t tmpaddr[3] = {writeAddr[0], writeAddr[1], writeAddr[2]};
	uint8_t* sectorBuf = pvPortMalloc(4096 * sizeof(uint8_t));
	uint8_t* pagebuf = pvPortMalloc(255* sizeof(uint8_t));

	if(sectorBuf == NULL) DBGSTR(".");
	memset(&sectorBuf[0],0,4096);
	memset(&pagebuf[0],0,255);
	uint32_t i = 0, tmp_convert1, times = 0;

	if(buff_size < Page_bufferSize - 1)
		times = 1;
	else{
		times = buff_size / (Page_bufferSize-1);
		if((buff_size%(Page_bufferSize-1))!=0)
			times++;
	}
	/* Note : According to winbond flash feature, the last bytes of every 256 bytes should be 0, so we need to plus one byte on every 256 bytes*/
	i = 0;
	while(i < times){
		memset(&pagebuf[0], 0, Page_bufferSize - 1);
		memcpy(&pagebuf[0], &writebuf[i*255], Page_bufferSize - 1);
		memcpy(&sectorBuf[i*Page_bufferSize], &pagebuf[0], Page_bufferSize - 1);
		sectorBuf[((i+1)*Page_bufferSize)-1] = 0;
		i++;
	}

	i = 0;
	while(i < times){
		if((nres=STM32SPIPageProgram(&sectorBuf[Page_bufferSize*i], &tmpaddr[0], Page_bufferSize)) != VAT_SUCCESS){
			DBGSTR("STM32SPIProgram_allData write data fail on %d times!",i);
			vPortFree(sectorBuf);
			vPortFree(pagebuf);
			return nres;
		}
		tmp_convert1 = (tmpaddr[0]<<16 | tmpaddr[1]<<8 | tmpaddr[2]) + Page_bufferSize;
		tmpaddr[0] = (tmp_convert1&0xFF0000) >> 16;
		tmpaddr[1] = (tmp_convert1&0xFF00) >>8;
		tmpaddr[2] = 0x00;
		i++;
	}
	vPortFree(sectorBuf);
	vPortFree(pagebuf);
	return nres;
}



VATAPI_RESULT STM32SPIWriteSector(void *writebuf, uint8_t* SectorAddr, uint32_t buff_size){



	if(STM32SPIEraseSector(SectorAddr) == VAT_SUCCESS){
		//DBGSTR("ERASE SECTOR - 0x%2x %2x %2x", SectorAddr[0], SectorAddr[1], SectorAddr[2]);
	}else return VAT_UNKNOWN;
	if(STM32SPIProgram_multiPage(writebuf, SectorAddr, buff_size) == VAT_SUCCESS){
		//DBGSTR("WRTIE SECTOR SUCCESSFUL");
		return VAT_SUCCESS;
	}else return VAT_UNKNOWN;

	return VAT_UNKNOWN;

}




/*
 * =================================================================================================
 * =============== Winbond  Flash Utility
 * =================================================================================================
 */

uint8_t programBuf[SPI_WinbondW25Q32FV_sectorByte];

uint8_t *STM32SPI_getProgramBuf(){
	return &programBuf[0];
}



VATAPI_RESULT STM32SPI_Customize_program(SPIProperties_Customize *data){
	VATAPI_RESULT nres;
	uint8_t* writeBuf = pvPortMalloc(255 * sizeof(uint8_t));
	int FileNodeId;
	sprintf((char *)writeBuf, CUSTOMIZE_TXT, data->channelNum, data->Country, data->LCD_Timeout, data->OutputLevel, data->HDCP, \
				data->EP9555E_Control, data->reserved);
	DBGSTR("%s",writeBuf);

	if(CreateFile(Winbond_FlashFW_ChannelTable,writeBuf,strlen(writeBuf)) != VAT_SUCCESS){
		DBGSTR("Fail to Program Customized data");
		vPortFree(writeBuf);
		return VAT_UNKNOWN;
	}
	else{
		DBGSTR("STM32SPI_Customize_program successfully!");
	}
	vPortFree(writeBuf);
	return nres;
}


/*
 * =================================================================================================
 * =============== 202 Flash Utility
 * =================================================================================================
 */
VATAPI_RESULT STM32SPIReadMem_For202(){
	VATAPI_RESULT nres;
	uint8_t* read_buf = pvPortMalloc(SPI_WinbondW25Q32FV_sectorByte * sizeof(uint8_t));
	uint8_t read_addr[3] = {0x00,0x00,0x00};
	uint32_t i = 0;

	nres = STM32SPIReadMem(&read_buf[0], &read_addr[0], SPI_WinbondW25Q32FV_sectorByte);			//flash usable status map
	if(nres == VAT_SUCCESS){
		while(i < SPI_WinbondW25Q32FV_sectorByte){
			DBGSTR("[%d]=%x",i,read_buf[i]);
			i++;
		}
	}
	pvPortFree(read_buf);
	return nres;
}

VATAPI_RESULT STM32SPIReadFlag_For202(uint8_t *flag_value){
	VATAPI_RESULT nres;
	uint8_t read_addr[3] = {0x3F,0x00,0x00}, read_buf[4];

	nres = STM32SPIReadMem(&read_buf[0], &read_addr[0], sizeof(read_buf));			//flash usable status map
	if(nres == VAT_SUCCESS){
		memcpy(flag_value, &read_buf[0], sizeof(read_buf));
		//DBGSTR("%x %x %x %x",flag_value[0],flag_value[1],flag_value[2],flag_value[3]);
	}
	return nres;
}

void STM32SPIWriteFlag_noCheck(uint8_t *flag_value){
	VATAPI_RESULT nres;
	uint8_t program_addr[3] = {0x3F,0x00,0x00}, program_buf[4] = {flag_value[0], flag_value[1], flag_value[2], flag_value[3]};
	uint8_t readflag[4];
	int i = 0;

	if(STM32SPIEraseSector(&program_addr[0]) == VAT_SUCCESS){
		DBGSTR("STM32SPIWriteFlag_For202 - Finish to erase sector(0x%2x%2x%2x)", program_addr[0], program_addr[1], program_addr[2]);
	}

	if((nres=STM32SPIPageProgram(&program_buf[0], &program_addr[0], sizeof(program_buf))) != VAT_SUCCESS){
		DBGSTR("STM32SPIWriteFlag_For202 write data fail on %d times!",i);
	}

}

VATAPI_RESULT STM32SPIWriteFlag_For202(uint8_t *flag_value){
	VATAPI_RESULT nres;
	uint8_t program_addr[3] = {0x3F,0x00,0x00}, program_buf[4] = {flag_value[0], flag_value[1], flag_value[2], flag_value[3]};
	uint8_t readflag[4], burnFlag = 0xFF & STM32FlashFW_Burn202_Flag;
	int i = 0;

	if(STM32SPIEraseSector(&program_addr[0]) == VAT_SUCCESS){
		DBGSTR("STM32SPIWriteFlag_For202 - Finish to erase sector(0x%2x%2x%2x)", program_addr[0], program_addr[1], program_addr[2]);
	}else return VAT_UNKNOWN;

	if((nres=STM32SPIPageProgram(&program_buf[0], &program_addr[0], sizeof(program_buf))) != VAT_SUCCESS){
		DBGSTR("STM32SPIWriteFlag_For202 write data fail on %d times!",i);
		return VAT_UNKNOWN;
	}

	nres = STM32SPIReadFlag_For202(&readflag[0]);
	if((readflag[0]&readflag[1]&readflag[2]&readflag[3]) == burnFlag)
		DBGSTR("%x %x %x %x,jump to upgrade 202 FW~~\r\n\n",readflag[0], readflag[1], readflag[2], readflag[3]);
	else{
		DBGSTR("error flag - %x %x %x %x",readflag[0], readflag[1], readflag[2], readflag[3]);
		return VAT_UNKNOWN;
	}

	return nres;
}

VATAPI_RESULT STM32SPIProgramData_For202(uint8_t *writebuf, uint32_t data_type, uint32_t buff_size, uint32_t writeAddr){
	VATAPI_RESULT nres;
	uint8_t program_addr[3] = {(writeAddr>>16)&0xFF, (writeAddr>>8)&0xFF, writeAddr&0xFF}, program_buf[128];
	uint32_t write_times = buff_size / sizeof(program_buf), tmp_convert1;
	int i = 0;

	if(buff_size % sizeof(program_buf) > 0)
		write_times++;

	DBGSTR("write addr 0x%2x%2x%2x",program_addr[0],program_addr[1],program_addr[2]);
	while(i < write_times){
		if((nres=STM32SPIPageProgram(&writebuf[sizeof(program_buf)*i], &program_addr[0], sizeof(program_buf))) != VAT_SUCCESS){
			DBGSTR("STM32SPIProgramData_For202 write data fail on %d times!",i);
			return nres;
		}
		tmp_convert1 = (program_addr[0]<<16 | program_addr[1]<<8 | program_addr[2]) + sizeof(program_buf);
		program_addr[0] = (tmp_convert1&0xFF0000) >> 16;
		program_addr[1] = (tmp_convert1&0xFF00) >>8;
		program_addr[2] = tmp_convert1&0xFF;
		i++;
	}
	return nres;
}

VATAPI_RESULT STM32SPI_202_FWprogram(uint8_t *data, uint32_t size, uint8_t flag, uint32_t writeAddr){
	VATAPI_RESULT nres;

	if(flag == 0)
    	STM32SPIEraseChip();

	//DBGSTR("start burn modulator FW to flash, size=%d",size);

	if((nres = STM32SPIProgramData_For202(data, SPI_202WinbondW25Q32BV_TYPE_all, size, writeAddr)) == VAT_SUCCESS){
		//DBGSTR("STM32SPI_202_FWprogram successfully!");
	}
	else
		DBGSTR("STM32SPI_202_FWprogram unsuccessfully! Please re-plug USB, and reload channel table again!");

	return nres;
}

/*
 * =================================================================================================
 * =============== STM32 Flash Utility
 * =================================================================================================
 */

uint8_t stm32programBuf[4096];

uint8_t *STM32flash_getProgramBuf(){
	return &stm32programBuf[0];
}

int flash_erase_sec(int sec_no)
{
	FLASH_EraseInitTypeDef er = {
		.TypeErase = FLASH_TYPEERASE_SECTORS,		//all erase
		.Sector = sec_no,		//FLASH_SECTOR_0~5
		.NbSectors = 1,
		.VoltageRange = FLASH_VOLTAGE_RANGE_3,
		//.Banks = FLASH_BANK_1,
	};
	uint32_t fault_sec = 0;
	HAL_FLASH_Unlock();
	DBGSTR("start erase sector : %d",sec_no);
	HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&er, &fault_sec);
	DBGSTR("finish~");
	HAL_FLASH_Lock();

	HAL_Delay(50);
	return res == HAL_OK ? 0 : -1;
}

void Write_STM32FlashFlag(uint32_t newIAPFlag_Addr, uint8_t* flag){
	HAL_StatusTypeDef status;
	uint32_t i = 0;

	HAL_FLASH_Unlock();
	while(i < sizeof(uint32_t)){
		if((status = HAL_FLASH_Program(TYPEPROGRAM_BYTE, newIAPFlag_Addr, flag[i])) == HAL_OK){
			//DBGSTR("Writing[0x%x]:%x~~~~~",newIAPFlag_Addr,flag[i]);
			i++;
			newIAPFlag_Addr++;
		}
		else
			DBGSTR("Write fail. status : %d",status);
	}
	HAL_FLASH_Lock();
}

void WriteRevise(uint32_t *startAddr, uint8_t *data, uint32_t size, uint32_t writeSectorType, uint8_t erase_flag)
{
	HAL_StatusTypeDef status;
	uint32_t i = 0, writeAddr = *startAddr;


	DBGSTR("WriteRevise~~~size:%d",size);
	HAL_Delay(500);

	if(writeSectorType == STM32EraseFlash_APPSectors && erase_flag == 0){		//first time need to erase APP flash space
		uint32_t newIAPFlag_Addr = STM32FlashFW_Bootloader_STARTADDR;
		uint8_t flag[] = {0xF0, 0xF0, 0xF0, 0xF0};
			if(flash_erase_sec(FLASH_SECTOR_3) == HAL_OK){
				if(flash_erase_sec(FLASH_SECTOR_4) == HAL_OK){
					DBGSTR("Erase flash successfully!");

					HAL_FLASH_Unlock();
					while(i < sizeof(flag)){
						if((status = HAL_FLASH_Program(TYPEPROGRAM_BYTE/*TYPEPROGRAM_BYTE*/, newIAPFlag_Addr, flag[i])) == HAL_OK){
							//DBGSTR("Writing[0x%x]~~~~~",writeAddr);
							i++;
							newIAPFlag_Addr++;
						}
						else
							DBGSTR("Write fail. status : %d",status);
					}
					HAL_FLASH_Lock();
					i = 0;
				}
				else
					DBGSTR("Erase APP flash fail!");
			}
			else
				DBGSTR("Erase APP flash fail!");
	}
	else if(writeSectorType == STM32EraseFlash_IAPSectors && erase_flag == 0){		//first time need to erase IAP flash space
		uint32_t newIAPFlag_Addr = STM32FlashFW_Bootloader_STARTADDR;
		uint8_t flag[] = {0xF2, 0xF2, 0xF2, 0xF2};
		if(flash_erase_sec(FLASH_SECTOR_0) == HAL_OK){
			if(flash_erase_sec(FLASH_SECTOR_1) == HAL_OK){
				if(flash_erase_sec(FLASH_SECTOR_2) == HAL_OK){
					DBGSTR("Erase flash successfully!");

					HAL_FLASH_Unlock();
					while(i < sizeof(flag)){
							if((status = HAL_FLASH_Program(TYPEPROGRAM_BYTE/*TYPEPROGRAM_BYTE*/, newIAPFlag_Addr, flag[i])) == HAL_OK){
								//DBGSTR("Writing[0x%x]~~~~~",writeAddr);
								i++;
								newIAPFlag_Addr++;
							}
							else
								DBGSTR("Write fail. status : %d",status);
					}
					HAL_FLASH_Lock();
					i = 0;
				}else DBGSTR("Erase APP flash fail!");
			}else DBGSTR("Erase APP flash fail!");
		}else DBGSTR("Erase APP flash fail!");
	}else if(writeSectorType == STM32EraseFlash_BootloaderSectors && erase_flag == 0){		//first time need to erase bootloader flash space
		uint32_t newIAPFlag_Addr = STM32FlashFW_Bootloader_STARTADDR;
		uint8_t flag[] = {0xF3, 0xF3, 0xF3, 0xF3};
		if(flash_erase_sec(FLASH_SECTOR_3) == HAL_OK){
			if(flash_erase_sec(FLASH_SECTOR_4) == HAL_OK){
					DBGSTR("Erase flash successfully!");

					HAL_FLASH_Unlock();
					while(i < sizeof(flag)){
							if((status = HAL_FLASH_Program(TYPEPROGRAM_BYTE/*TYPEPROGRAM_BYTE*/, newIAPFlag_Addr, flag[i])) == HAL_OK){
								//DBGSTR("Writing[0x%x]~~~~~",writeAddr);
								i++;
								newIAPFlag_Addr++;
							}
							else DBGSTR("Write fail. status : %d",status);
					}
					HAL_FLASH_Lock();
					i = 0;
			}else DBGSTR("Erase APP flash fail!");
		}else DBGSTR("Erase APP flash fail!");
	}
	else{

	}
	HAL_FLASH_Unlock();
	//HAL_FLASHEx_Erase(0x8016000);
	while(i < size){
		if((status = HAL_FLASH_Program(TYPEPROGRAM_BYTE/*TYPEPROGRAM_BYTE*/, writeAddr, data[i])) == HAL_OK){
			//DBGSTR("Writing[0x%x]~~~~~",writeAddr);
			i++;
			writeAddr++;
		}
		else
			DBGSTR("Write fail. status : %d",status);
	}
	HAL_FLASH_Lock();

	*startAddr = writeAddr;
	DBGSTR("WriteRevise down- ~~",status);
}










