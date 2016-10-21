/**
 * @file STM32FlashFS_FF.h
 * @author Max Hsu
 * @brief STM32 Flash File System File Layer
 *
 * Inorder to keep track to all the file existing in the file system. We need to maintain a file
 * table which contains all the file existing in the file system. In file layer, it initializes the
 * file table and provides save/load between RAM and Flash. Also, it provides api to register and de-register
 * a file node. When a file node is registered, it is contains only file header which is stored in file
 * table, including filename, fsize, number of sectors...etc, it contains no data. Note that in stm32 environment
 * we don't have enough RAM space for us to keep the whole file data in the RAM in order to do a single
 * write to flash. Instead, we have to use strategy like batch write and chain the allocated sector
 * together.
 *
 *
 * Foundation, VATek.
 * Copyright (c) 2016 VATek. All rights reserved.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "VATDebug.h"
#include <VATSDKDefine.h>



#ifndef INCLUDES_STM32FLASHFS_FF_H_
#define INCLUDES_STM32FLASHFS_FF_H_

/*
 *  fsize: file size
 *  fname: file name
 *  numSector: number of sector to store file
 *  StartSector: start sector of file
 *  EndSector: ending sector of file
 */



#define Unused 0
#define Used 1
#define EmptyFile 0x403 //indicate the file is empty
#define NO_CHILD 0x404
#define NO_ATTRIB 0
#define FS_Error 9999

/*
 * The file system contains file and directory. We want our file system be structured as tree.
 * So we just link whole the file and directory together and set root directory as this tree's root
 * node and files must be leaf nodes. Thus the tree will be a n-degree tree where n equals number of
 * child of a directory which contains the most number of child. To implement this as data structure,
 * it may result too much waste space because not all the inner nodes contain n child. As the result,
 * we transform the tree into binary tree and, in this case, a file can have only right child since
 * it is a leaf node in original tree.
 *
 * */

typedef struct _FILE_NODE{
	//uint8_t attrib_left; never happen
	//uint16_t left_child; never happen
	uint8_t attrib_right;  //justify what type of its right child nodes. file or directory.
	uint8_t usage; //a file node is usage or not.
	uint32_t fsize; // file size
	uint8_t fname[40]; //file name with maximum length 40 char.
	uint16_t numSector; // number of sector for storage
	uint16_t StartSector; //start sector
	uint16_t EndSector; //end sector
	uint32_t right_child; //pointer to its right child
}FILE_NODE, *PFILE_NODE;


extern FILE_NODE File_Table[64];



/**
  * @brief Initialize file table and store it in the flash
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT File_Table_Init();
/**
  * @brief Load file table from flash to RAM
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Load_File_Table();
/**
  * @brief Save file table from RAM to flash
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Save_File_Table();
/**
  * @brief Register a file node
  *
  * @param  *fname Name of the file
  * @param  fsize Size of file
  * @param  nSector Number of sector to
  * @retval Registered File node id
  */
int Register_File_Node(uint8_t* fname, uint32_t fsize, uint16_t nSector);
/**
  * @brief Find the id of the specific file node
  *
  * @retval Id of specific file node
  */
uint32_t Find_File_Node(uint8_t* fname);
/**
  * @brief Perform batch write to specific file node.
  *
  * @param  *fname Name of the file
  * @param  batch_size Size of the batch write buffer
  * @param  *writebuf Pointer to the batch buffer
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Write_File_Node(uint8_t* fname, uint32_t batch_size, uint8_t* writebuf);

VATAPI_RESULT Read_File_Batch(uint8_t* fname, uint8_t* buf, uint32_t sectorid_Logical);
VATAPI_RESULT Delete_File_Node(uint8_t* fname);


#endif /* INCLUDES_STM32FLASHFS_FF_H_ */
