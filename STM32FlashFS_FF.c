#include "STM32FlashFS_FF.h"
#include "STM32FlashFS_LL.h"
#include "STM32SPIImpl.h"


FILE_NODE File_Table[64];


static uint8_t File_Table_Addr[3] = { 0x00, 0x10, 0x00}; // 2nd sector in flash.

#define File_Table_Size 4096
#define Sector_Size 4096


VATAPI_RESULT File_Table_Init(){


	for(int i = 0; i < 64;i++){
		File_Table[i].usage = Unused;
		File_Table[i].attrib_right = NO_ATTRIB;
		File_Table[i].fsize = 0;
		memset(&File_Table[i].fname,0,40);
		File_Table[i].numSector = 0;
		File_Table[i].StartSector = 0;
		File_Table[i].EndSector = 0;
		File_Table[i].right_child = NO_CHILD;
		//DBGSTR("usage:%d fname:%s fsize:%d numsector:%d",File_Table[i].usage,File_Table[i].fname,File_Table[i].fsize,File_Table[i].numSector);
	}
	if(STM32SPIWriteSector(&File_Table[0] , &File_Table_Addr[0], sizeof(File_Table)) == VAT_SUCCESS){
		//DBGSTR("Successful Initial File Table");

		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Intial File Table Fail");
		return VAT_UNKNOWN;
	}

}

VATAPI_RESULT Load_File_Table(){

	if(STM32SPIReadSector(&File_Table[0], &File_Table_Addr[0], sizeof(File_Table)) == VAT_SUCCESS){
		//DBGSTR("Successful Load File Table");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Load File Table Fail");
		return VAT_UNKNOWN;
	}
}


VATAPI_RESULT Save_File_Table(){

	if(STM32SPIWriteSector(&File_Table[0] , &File_Table_Addr[0], sizeof(File_Table)) == VAT_SUCCESS){
		//DBGSTR("Successful Save File Table");
		return VAT_SUCCESS;
	}
	else{
		DBGSTR("Save File Table Fail");
		return VAT_UNKNOWN;
	}
}


uint32_t Find_Empty_FileNode(){

	int i = 0;

	for( i = 0; i < 64; i++)
		if(File_Table[i].usage == Unused)	return i; //found empty sector.

	if(i == 63) return VAT_UNKNOWN; //No Empty Sector.
	//}
	else return VAT_UNKNOWN;
}


uint16_t Find_Last_Sector(FILE_NODE FILENODE){

	if(FILENODE.StartSector == EmptyFile) return EmptyFile;
	else{
		if(Load_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;

		if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
		else{
			return FILENODE.EndSector;

		}
	}

}

int Register_File_Node(uint8_t* fname, uint32_t batch_size, uint16_t nSector){

	uint32_t FileNodeId;
	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{
		FileNodeId = Find_Empty_FileNode();
		if(FileNodeId == VAT_UNKNOWN) return VAT_UNKNOWN;
		else{
			File_Table[FileNodeId].usage = Used;
			strcpy(&File_Table[FileNodeId].fname[0], fname);
			File_Table[FileNodeId].StartSector = EmptyFile;
			File_Table[FileNodeId].EndSector = EmptyFile;
			File_Table[FileNodeId].fsize = batch_size;
			File_Table[FileNodeId].numSector = nSector;

			Save_File_Table();
			int i = FileNodeId;

			DBGSTR("usage:%d fname:%s fsize:%d numsector:%d",File_Table[i].usage,File_Table[i].fname,File_Table[i].fsize,File_Table[i].numSector);


			return FileNodeId;
		}

	}
}

uint32_t Find_File_Node(uint8_t* fname){

	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{

		for(int i = 0;i < 64;i++){
			if(strcmp(&File_Table[i].fname[0], fname) == 0)	return i;
		}

		DBGSTR("File Node Not Found");

		return FS_Error;
	}
}

//batch write fsize must less than 4096
VATAPI_RESULT Write_File_Node(uint8_t* fname, uint32_t fsize, uint8_t* writebuf){

	FILE_NODE curFile;
	uint32_t FileNodeId;
	uint16_t lastSector;
	uint16_t curSector;

	if(fsize > Sector_Size){
		DBGSTR("Write Buffer Overflow");
		return VAT_UNKNOWN;
	}

	if(GETnEmpty_Sector() == 0){
		DBGSTR("Not Enough Sector to Allocate");
		return VAT_UNKNOWN;
	}
	if(Load_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Load_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
	else{

		FileNodeId = Find_File_Node(fname);

		if(FileNodeId == FS_Error) return VAT_UNKNOWN;
		else{
			curFile = File_Table[FileNodeId];
			lastSector = Find_Last_Sector(curFile);
			/* "Last sector" of file is the last time written sector of a specific file
			 * When the file node just registered, the file node's startsector and endsector
			 * are initialized to "EmptyFile". Hence, the first write to file should set
			 * the "startsector" to the first allocated sector.
			 * */
			if(lastSector == EmptyFile){
				curSector = Sector_Allocate(writebuf, fsize);
				File_Table[FileNodeId].StartSector = curSector;
				File_Table[FileNodeId].EndSector = curSector;
				//bitmap[lastSector].next_sector
				Sectormap[curSector].next_sector = END_OF_FILE;
			}
			else{
				curSector = Sector_Allocate(writebuf, fsize);
				File_Table[FileNodeId].EndSector = curSector;
				Sectormap[lastSector].next_sector = curSector;
				Sectormap[curSector].next_sector = END_OF_FILE;
			}

		}


		if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;

		if(Save_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;
		{
			//DBGSTR("Batch Write File Successful.Usage: %d,Name: %s, Size: %d, StartSector: %d, EndSector:%d, TotalSector: %d", File_Table[FileNodeId].usage,File_Table[FileNodeId].fname,File_Table[FileNodeId].fsize,File_Table[FileNodeId].StartSector,File_Table[FileNodeId].EndSector,File_Table[FileNodeId].numSector);
			return VAT_SUCCESS;
		}

	}

}

VATAPI_RESULT Read_File_Batch(uint8_t* fname, uint8_t* buf, uint32_t sectorid_Logical){ // sector id: logical id. start with 1,2,3,4,...

	uint32_t FileNodeId;
	uint32_t sectorid_phy;
	uint8_t SectorAddr[3];
	uint16_t ReadLength;

	if(Load_Sectormap() != VAT_SUCCESS) return VAT_UNKNOWN;
	if(Load_File_Table() != VAT_SUCCESS) {
		return VAT_UNKNOWN;
	}
	else{
		FileNodeId = Find_File_Node(fname);
		if(FileNodeId == FS_Error){
			return VAT_UNKNOWN;
		}
		sectorid_phy = File_Table[FileNodeId].StartSector;
		for(int i = 1;i < sectorid_Logical;i++){
			sectorid_phy = Sectormap[sectorid_phy].next_sector;
		}
		if(sectorid_Logical == File_Table[FileNodeId].numSector){
			ReadLength = File_Table[FileNodeId].fsize % 4080;
			if(ReadLength == 0) ReadLength = 4080;
		}
		else
			ReadLength = 4080;

		SectorAddr[0] = (sectorid_phy & 0x00000FF0) >> 4;
		SectorAddr[1] = (sectorid_phy & 0x0000000F) << 4;
		SectorAddr[2] = 0x00;


		STM32SPIReadSector(&buf[0], &SectorAddr[0],	ReadLength);


		return VAT_SUCCESS;
	}
}

VATAPI_RESULT Delete_File_Node(uint8_t* fname){

	uint32_t FileNodeId;

	if(Load_File_Table() != VAT_SUCCESS) {
		return VAT_UNKNOWN;
	}

	FileNodeId = Find_File_Node(fname);
	if(FileNodeId == FS_Error){
		DBGSTR("No Such File");
		return VAT_UNKNOWN;
	}
	else{
		if(Sector_DeAllocate(File_Table[FileNodeId].StartSector,File_Table[FileNodeId].EndSector, \
				File_Table[FileNodeId].numSector) != VAT_SUCCESS){
			DBGSTR("Hardfault! Delete file improperly");
			return VAT_UNKNOWN;
		}
		else{
			File_Table[FileNodeId].usage = Unused;
			File_Table[FileNodeId].attrib_right = NO_ATTRIB;
			File_Table[FileNodeId].fsize = 0;
			memset(&File_Table[FileNodeId].fname,0,40);
			File_Table[FileNodeId].numSector = 0;
			File_Table[FileNodeId].StartSector = 0;
			File_Table[FileNodeId].EndSector = 0;
			File_Table[FileNodeId].right_child = NO_CHILD;
			//DBGSTR("Delete File Success");
			if(Save_File_Table() != VAT_SUCCESS) return VAT_UNKNOWN;
			else return VAT_SUCCESS;
		}
	}
}



