/*
 * STM32FlashFS_Logical.c
 *
 *  Created on: Jul 29, 2016
 *      Author: pwanhsu
 */
#include <stdint.h>
#include <VATSDKDefine.h>
#include "STM32FlashFS_LL.h"
#include "STM32FlashFS_FF.h"
#include "STM32SPIImpl.h"
#include "STM32FlashFS_Logical.h"
#include <string.h>
#include <stdlib.h>


DIR_NODE FS_Tree[64];

static uint8_t FS_Tree_Addr[3] = { 0x00, 0x20, 0x00 };




static uint16_t curDIRId = 0;
uint16_t Layer = 0;


VATAPI_RESULT FileSystem_Init(){

	VATAPI_RESULT nres;


	nres = STM32SPIEraseChip();
	if(nres != VAT_SUCCESS) return VAT_UNKNOWN;
	nres = Sectormap_Init();
	if(nres != VAT_SUCCESS) return VAT_UNKNOWN;
	nres = File_Table_Init();
	if(nres != VAT_SUCCESS) return VAT_UNKNOWN;


	for(int i = 0; i < 64;i++){
		FS_Tree[i].USAGE = Unused;
		FS_Tree[i].attrib_left = NO_ATTRIB;
		FS_Tree[i].attrib_right = NO_ATTRIB;
		memset(&FS_Tree[i].dname,0,40);
		FS_Tree[i].number_of_child = 0;
		FS_Tree[i].left_child = NO_CHILD;
		FS_Tree[i].right_child = NO_CHILD;
		//DBGSTR("usage:%d  dname:%s  leftchild:%d  rightchild:%d  leftattrib:%d  rightattrib:%d",FS_Tree[i].USAGE,FS_Tree[i].dname,FS_Tree[i].left_child,FS_Tree[i].right_child,FS_Tree[i].attrib_left,FS_Tree[i].attrib_right);

	}
	FS_Tree[0].USAGE = Used;
	strcpy(FS_Tree[0].dname,"ROOT");

	if(STM32SPIWriteSector(&FS_Tree[0], &FS_Tree_Addr[0], sizeof(FS_Tree)) == VAT_SUCCESS){
		DBGSTR("Successful Initial File System Tree");

		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Intial File System Tree Fail");
		return VAT_UNKNOWN;
	}

}

VATAPI_RESULT Load_FS_Tree(){

	if(STM32SPIReadSector(&FS_Tree[0], &FS_Tree_Addr[0], sizeof(FS_Tree)) == VAT_SUCCESS){
		//DBGSTR("Successful Load FS TREE");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Load File Table Fail");
		return VAT_UNKNOWN;
	}
}

VATAPI_RESULT Save_FS_Tree(){

	if(STM32SPIWriteSector(&FS_Tree[0] , &FS_Tree_Addr[0], sizeof(FS_Tree)) == VAT_SUCCESS){
		//DBGSTR("Successful Save File Tree");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Save File Tree Fail");
		return VAT_UNKNOWN;
	}

}

uint16_t Find_Empty_DIRNode(){

	int i = 0;

	for( i = 0; i < 64; i++)
		if(FS_Tree[i].USAGE == Unused)	return i; //found empty sector.

	if(i == 64) return VAT_UNKNOWN; //No Empty Sector.
	//}
	else return VAT_UNKNOWN;
}



VATAPI_RESULT InorderTraversal(uint32_t nodeid){

	uint8_t Attrib = DIRECTORY;
	char space[30] = "-";
	char tmp[30] = "----";
	if(nodeid == 0){
		DBGSTR("[Root]:");
	}
	for(int i = 0;i < Layer;i++){
		strcat(space,tmp);
		//DBGSTR("%s", space);
	}

	if(FS_Tree[nodeid].left_child != NO_CHILD){
		Attrib = FS_Tree[nodeid].attrib_left;
		nodeid = FS_Tree[nodeid].left_child;


		while(nodeid != END_OF_DIR){
			if(Attrib == DIRECTORY){
				DBGSTR("%s[Directory]: %s", space, FS_Tree[nodeid].dname);
				Layer ++;
				if(InorderTraversal(nodeid) != VAT_SUCCESS) return VAT_UNKNOWN;
				Layer --;
				if(FS_Tree[nodeid].attrib_right == FILE){
					Attrib = FILE;
				}
				nodeid = FS_Tree[nodeid].right_child;
			}
			else if(Attrib == FILE){
				DBGSTR("%s[File]: %s", space, File_Table[nodeid].fname);
				if(File_Table[nodeid].attrib_right == DIRECTORY){
					Attrib = DIRECTORY;
				}
				nodeid = File_Table[nodeid].right_child;
			}
		}

	}
	return VAT_SUCCESS;
}

VATAPI_RESULT FS_Traversal(){

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{
		if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
		else{
			Layer = 0;
			if(InorderTraversal(ROOT_ID) != VAT_SUCCESS) return VAT_UNKNOWN;
		}
	}
	return VAT_SUCCESS;
}

/*VATAPI_RESULT DIR_Traversal(uint16_t nodeid){

	VATAPI_RESULT nres;
	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{
		if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
		else{

		}
	}


}*/


VATAPI_RESULT CreateFile(uint8_t* fname,uint8_t* writebuf ,uint32_t fsize){

	uint32_t FileNodeId; //
	uint32_t nodeid;
	uint8_t Attrib = DIRECTORY;//
	int sector_count = fsize / 4080;
	int last_write_size; //size for the last write. ex: file size: 4080 + 100  -> last_write_size: 100
	if(fsize % 4080){
		last_write_size = fsize % 4080;
		sector_count++;
	}
	else last_write_size = 4080;

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;

	if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;


	int len = strlen(writebuf);


	FileNodeId = Register_File_Node(fname, fsize, sector_count);
	nodeid = FileNodeId;


	if(FileNodeId < 0) return VAT_UNKNOWN;
	else{
		for(int i = 0;i < sector_count;i++){

			if(len <= 4080){
				Write_File_Node(fname, last_write_size, writebuf);
			}

			else{
			Write_File_Node(fname, 4080, writebuf);
			len = len - 4080;
			}

		}
		if(FS_Tree[curDIRId].left_child == NO_CHILD){
			FS_Tree[curDIRId].left_child = FileNodeId;
			FS_Tree[curDIRId].attrib_left = FILE;
		}
		else{
			nodeid = FS_Tree[curDIRId].left_child;
			Attrib = FS_Tree[curDIRId].attrib_left;
			while(nodeid != END_OF_DIR){
				if(Attrib == DIRECTORY){
					if(FS_Tree[nodeid].right_child == END_OF_DIR){
						FS_Tree[nodeid].right_child = FileNodeId;
						FS_Tree[nodeid].attrib_right = FILE;
						break;
					}
					if(File_Table[nodeid].attrib_right == FILE){
						Attrib = FILE;
					}
					nodeid = FS_Tree[nodeid].right_child;

				}
				else if(Attrib == FILE){
						if(File_Table[nodeid].right_child == END_OF_DIR){
							File_Table[nodeid].right_child = FileNodeId;
							File_Table[nodeid].attrib_right = FILE;
							break;
						}
						if(File_Table[nodeid].attrib_right == DIRECTORY){
							Attrib = DIRECTORY;
						}
						nodeid = File_Table[nodeid].right_child;

				}
			}
		}
		File_Table[FileNodeId].attrib_right = NO_ATTRIB;
		File_Table[FileNodeId].right_child = END_OF_DIR;

	}
/*
	for(int i = 0; i < 10;i++){
		DBGSTR("Usage: %d,Name: %s, Size: %d, StartSector: %d, EndSector:%d, TotalSector: %d, rightchild: %d, rightattrib: %d", File_Table[i].usage,File_Table[i].fname,File_Table[i].fsize,File_Table[i].StartSector,File_Table[i].EndSector,File_Table[i].numSector ,File_Table[i].right_child, File_Table[i].attrib_right);
	}
	for(int i = 0;i < 10;i++){
			DBGSTR("usage:%d  dname:%s  leftchild:%d  rightchild:%d  leftattrib:%d  rightattrib:%d",FS_Tree[i].USAGE,FS_Tree[i].dname,FS_Tree[i].left_child,FS_Tree[i].right_child,FS_Tree[i].attrib_left,FS_Tree[i].attrib_right);
		}
*/
	if(Save_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;

	return VAT_SUCCESS;
}

VATAPI_RESULT Read_File(uint8_t* fname, uint8_t* buf){

	uint32_t FileNodeId;
	uint32_t StartSector;
	uint32_t sectorid;
	uint32_t EndSector;
	uint8_t SectorAddr[3];
	uint16_t SizeRemain;
	uint16_t ReadLength;
	uint8_t *Readbuf = pvPortMalloc(4080 * sizeof(uint8_t));
	memset(Readbuf,0,4080);
	Load_Sectormap();
	if(Load_File_Table() != VAT_SUCCESS) {
		vPortFree(Readbuf);
		return VAT_UNKNOWN;
	}
	else{
		FileNodeId = Find_File_Node(fname);
		if(FileNodeId == FS_Error){
			vPortFree(Readbuf);
			return VAT_UNKNOWN;
		}
		StartSector = File_Table[FileNodeId].StartSector;
		EndSector = File_Table[FileNodeId].EndSector;
		sectorid = StartSector;
		SizeRemain = File_Table[FileNodeId].fsize;
		for(int i = 0;i < File_Table[FileNodeId].numSector;i++){
			SectorAddr[0] = (sectorid & 0x00000FF0) >> 4;
			SectorAddr[1] = (sectorid & 0x0000000F) << 4;
			SectorAddr[2] = 0x00;

			if(SizeRemain <= 4080){
				ReadLength = SizeRemain;
			}
			else{
				ReadLength = 4080;
				SizeRemain = SizeRemain - 4080;
			}
			//DBGSTR("Remain:%d, Read:%d, sectorid:%d",SizeRemain,ReadLength,sectorid);

			STM32SPIReadSector(&buf[0+i*4080], &SectorAddr[0],	ReadLength);
			sectorid = Sectormap[sectorid].next_sector;

		}
		//DBGSTR("%s",buf);
		//for(int i = 0;i<4800;i++)
		//	DBGSTR("%d : %d %c",i,i%10,Readbuf[i]);

	}
	vPortFree(Readbuf);

	return VAT_SUCCESS;
}

uint16_t CreateDirectory(uint8_t* dname){


	uint32_t DIRNodeId;
	uint32_t nodeid;
	uint8_t Attrib = DIRECTORY;

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{
		DIRNodeId = Find_Empty_DIRNode();
		if(DIRNodeId == VAT_UNKNOWN) return VAT_UNKNOWN;
		else{
			FS_Tree[DIRNodeId].USAGE = Used;
			strcpy(&FS_Tree[DIRNodeId].dname[0], dname);
			FS_Tree[DIRNodeId].attrib_left = NO_ATTRIB;
			FS_Tree[DIRNodeId].left_child = NO_CHILD;
			FS_Tree[DIRNodeId].attrib_right = NO_ATTRIB;
			FS_Tree[DIRNodeId].right_child = NO_CHILD;
			FS_Tree[DIRNodeId].number_of_child = 0;

		}
		if(FS_Tree[curDIRId].left_child == NO_CHILD){
			FS_Tree[curDIRId].left_child = DIRNodeId;
			FS_Tree[curDIRId].attrib_left = DIRECTORY;
		}
		else{
			nodeid = FS_Tree[curDIRId].left_child;
			Attrib = FS_Tree[curDIRId].attrib_left;
			while(nodeid != END_OF_DIR){
				if(Attrib == DIRECTORY){
					if(FS_Tree[nodeid].right_child == END_OF_DIR){
						FS_Tree[nodeid].attrib_right = DIRECTORY;
						FS_Tree[nodeid].right_child = DIRNodeId;
						break;
					}
					if(File_Table[nodeid].attrib_right == FILE){
						Attrib = FILE;
					}
					nodeid = FS_Tree[nodeid].right_child;
				}
				else if(Attrib == FILE){
					if(File_Table[nodeid].right_child == END_OF_DIR){
						File_Table[nodeid].attrib_right = DIRECTORY;
						File_Table[nodeid].right_child = DIRNodeId;
						break;
					}
					if(File_Table[nodeid].attrib_right == DIRECTORY){
						Attrib = DIRECTORY;
					}
					nodeid = File_Table[nodeid].right_child;
				}
			}
		}
		FS_Tree[DIRNodeId].attrib_right = NO_ATTRIB;
		FS_Tree[DIRNodeId].right_child = END_OF_DIR;
	}
	/*
	for(int i = 0; i < 10;i++){
		DBGSTR("Usage: %d,Name: %s, Size: %d, StartSector: %d, EndSector:%d, TotalSector: %d, rightchild: %d, rightattrib: %d", File_Table[i].usage,File_Table[i].fname,File_Table[i].fsize,File_Table[i].StartSector,File_Table[i].EndSector,File_Table[i].numSector ,File_Table[i].right_child, File_Table[i].attrib_right);
		}
	for(int i = 0;i < 10;i++){
		DBGSTR("usage:%d  dname:%s  leftchild:%d  rightchild:%d  leftattrib:%d  rightattrib:%d",FS_Tree[i].USAGE,FS_Tree[i].dname,FS_Tree[i].left_child,FS_Tree[i].right_child,FS_Tree[i].attrib_left,FS_Tree[i].attrib_right);
	}
*/
	if(Save_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;

	return DIRNodeId;

}


VATAPI_RESULT Delete_File(uint8_t* fname){

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;

	uint32_t nodeid = Find_File_Node(fname);
	if(nodeid == FS_Error) return VAT_UNKNOWN;
	uint32_t file_right_child = File_Table[nodeid].right_child;
	uint8_t attrib = File_Table[nodeid].attrib_right;

	if(Delete_File_Node(fname) != VAT_SUCCESS){
		DBGSTR("Delete File Fail");
		return VAT_UNKNOWN;
	}
	else{
		if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
		for(int i = 0;i < 64;i++){
			if(FS_Tree[i].USAGE == Used){
				if(FS_Tree[i].attrib_left == FILE){
					if(FS_Tree[i].left_child == nodeid){
						//DBGSTR("1");
						FS_Tree[i].left_child = file_right_child;
						FS_Tree[i].attrib_left = attrib;
						if(Save_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
						if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;
						return VAT_SUCCESS;
					}
				}
				if(FS_Tree[i].attrib_right == FILE){
					if(FS_Tree[i].right_child == nodeid){
						//DBGSTR("2");
						FS_Tree[i].right_child = file_right_child;
						FS_Tree[i].attrib_right = attrib;
						if(Save_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
						if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;
						return VAT_SUCCESS;
					}
				}
			}
			if(File_Table[i].usage == Used){
				if(File_Table[i].attrib_right == FILE){
					if(File_Table[i].right_child == nodeid){
						//DBGSTR("3");
						File_Table[i].right_child = file_right_child;
						File_Table[i].attrib_right = attrib;
						if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
						if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;
						return VAT_SUCCESS;
					}
				}
			}

		}

		return VAT_UNKNOWN;
	}
}



VATAPI_RESULT Add_File_To_Tree(uint32_t FileNodeId){

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	uint32_t nodeid;
	uint8_t Attrib;
	nodeid = FileNodeId;
	if(FS_Tree[curDIRId].left_child == NO_CHILD){
		FS_Tree[curDIRId].left_child = FileNodeId;
		FS_Tree[curDIRId].attrib_left = FILE;
	}
	else{
		nodeid = FS_Tree[curDIRId].left_child;
		Attrib = FS_Tree[curDIRId].attrib_left;
		while(nodeid != END_OF_DIR){
			if(Attrib == DIRECTORY){
				if(FS_Tree[nodeid].right_child == END_OF_DIR){
					FS_Tree[nodeid].right_child = FileNodeId;
					FS_Tree[nodeid].attrib_right = FILE;
					break;
				}
				if(File_Table[nodeid].attrib_right == FILE){
					Attrib = FILE;
				}
				nodeid = FS_Tree[nodeid].right_child;

			}
			else if(Attrib == FILE){
				if(File_Table[nodeid].right_child == END_OF_DIR){
					File_Table[nodeid].right_child = FileNodeId;
					File_Table[nodeid].attrib_right = FILE;
					break;
				}
				if(File_Table[nodeid].attrib_right == DIRECTORY){
					Attrib = DIRECTORY;
				}
				nodeid = File_Table[nodeid].right_child;

			}
		}
	}

	File_Table[FileNodeId].attrib_right = NO_ATTRIB;
	File_Table[FileNodeId].right_child = END_OF_DIR;

	if(Save_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(FS_Traversal() != VAT_SUCCESS) return VAT_UNKNOWN;

	return VAT_SUCCESS;

}
uint32_t Find_DIR_Node(uint8_t* fname){

	if(Load_FS_Tree() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{

		for(int i = 0;i < 64;i++){
			if(strcmp(&FS_Tree[i].dname[0], fname) == 0)	return i;
		}
		DBGSTR("File Node Not Found");
		return FS_Error;
	}
}
void change_curdir(uint8_t* dname){

	uint32_t DirId = Find_DIR_Node(dname);
	if(DirId == FS_Error) DBGSTR("No such directory");
	else{
		curDIRId = DirId;
	}
}


uint16_t getCurDIRId(){
	return curDIRId;
}



