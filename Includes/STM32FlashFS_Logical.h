/**
 * @file STM32FlashFS_FF.h
 * @author Max Hsu
 * @brief STM32 Flash File System Logic Layer
 *
 * In order to a better management. The file system should be structured as tree structure that it is
 * a must to have directory nodes and chain all the files and directories. In logic layer, It provides
 * api to initial the whole file system, including sectormap, file table, and directory table. Also, it
 * provide api to Load/Store the file system tree which is a directory table with the 1th item "root" and
 * there are api to create directory and perform traversal in the file system tree.
 *
 *
 * Foundation, VATek.
 * Copyright (c) 2016 VATek. All rights reserved.
 */


#ifndef INCLUDES_STM32FLASHFS_LOGICAL_H_
#define INCLUDES_STM32FLASHFS_LOGICAL_H_



typedef struct _DIR_NODE{
	uint8_t USAGE; //indicate directory is used or unused
	uint8_t attrib_left; //justify what type of their child nodes. file / directory.
	uint8_t attrib_right;
	uint8_t dname[41]; //Directory name.
	uint8_t number_of_child; //number of child
	uint32_t left_child; //first child
	uint32_t right_child; //sibling
}DIR_NODE, *PDIR_NODE;

#define END_OF_DIR 0x403

#define DIRECTORY 1
#define FILE 2
#define ROOT_ID 0
#define FS_Tree_Size sizeof(FS_Tree) + (sizeof(FS_Tree) / 255)

extern uint16_t Layer;
extern DIR_NODE FS_Tree[64];

/**
  * @brief Initial FS System, including Sectormap, File Table, FS Tree
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT FileSystem_Init();
/**
  * @brief Load FS Tree from flash to RAM
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Load_FS_Tree();
/**
  * @brief Save FS Tree from RAM to flash
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Save_FS_Tree();
/**
  * @brief Perform Whole File System Traversal
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT FS_Traversal();
/**
  * @brief Perform Whole Directory Traversal
  *
  * @retval nres VATAPI RESULT STATUS
  */
VATAPI_RESULT DIR_Traversal();
/**
  * @brief Create a File and write data in it
  *
  * Although the RAM size limitation, "Register file node and Batch write" is suggest,
  * sometimes we still want to create file with a single api function when file is small.
  * thus this file system provide create file api for those file is smaller than 2 sector(8160bytes).
  *
  * @param  *fname File name
  * @param  *writebuf Pointer to the buffer that contains file data
  * @param  *f_size File size
  * @retval  nres VATAPI RESULT STATUS
  */
VATAPI_RESULT CreateFile(uint8_t* fname,uint8_t* writebuf ,uint32_t fsize);
/**
  * @brief Read a File
  *
  * Although the RAM size limitation, "Batch read with loop" is suggest,
  * sometimes we still want to read file with a single API function when file is small.
  * thus this file system provide read file API for those file is smaller than 2 sector(8160bytes).
  *
  * @param  *fname File name
  * @param  *buf Pointer to the buffer to store the read data
  * @retval  nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Read_File(uint8_t* fname,uint8_t* buf);

/**
  * @brief Delete a File
  *
  * @param  *fname File name
  * @retval  nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Delete_File(uint8_t* fname);


/**
  * @brief Create a Directory
  *
  * @param  *dname Directory name
  * @retval  nres VATAPI RESULT STATUS
  */
uint16_t CreateDirectory(uint8_t* dname); //

VATAPI_RESULT DeleteDirectory(); //


/**
  * @brief Add a file to current directory
  *
  *
  * @param  FileNodeId Id of specific file
  * @retval  nres VATAPI RESULT STATUS
  */
VATAPI_RESULT Add_File_To_Tree(uint32_t FileNodeId);
/**
  * @brief Change directory
  *
  * @param  *dname Directory name
  */
void change_curdir(uint8_t* dname);
/**
  * @brief Get Current Directory
  *
  * @retval curDIRId current directory
  */
uint16_t getCurId();
#endif /* INCLUDES_STM32FLASHFS_LOGICAL_H_ */
