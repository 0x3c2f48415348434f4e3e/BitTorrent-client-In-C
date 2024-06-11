#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stddef.h>

typedef unsigned long int lint_16;
#define TORRENTFILENOTFOUNDERRORLOG(...) {printf("In file %s, given file not found: %s\n",__FILE__,__VA_ARGS__);}
#define MALLOCALLOCATIONERROR(...) {printf("In file %s, Memory allocation failed",__FILE__);}
typedef struct{
	char BYTE_STRING;
	char INTEGER;
	char LISTS;
	char DICTIONARY;
}Bencoder;

Bencoder BencoderValues = (Bencoder){' ','i','l','d'};

static void readTorrentFile(const char* filePath);
static void BencoderReader(const unsigned char* fileContent);
static void FAILUREEXIT(int errorCode);


//Make sure file is in current directory
static void readTorrentFile(const char* filePath){
	FILE *fileptr;
	fileptr = fopen(filePath,"r");
	void (*exitProgram)(int) = &FAILUREEXIT;
	if(!fileptr){
		//fprintf(stdout,"\nUnable to open given file\n");
		TORRENTFILENOTFOUNDERRORLOG(filePath);
		(*exitProgram)(-1);
	}

	//The torrent file is also known as the metainfo file
	/*
	These files are bencoded, menaing we have to parse it out
	to understand the meaning
	*/
	//get the size of the file in bytes
	fseek(fileptr,0,SEEK_END);
	lint_16 fileSize = ftell(fileptr);
	fseek(fileptr,0,SEEK_SET);
	//allocate space in memory to hold content as size of file will
	//only be known at run time
	unsigned char *buffer = (unsigned char *) malloc(fileSize);
	if(buffer == NULL){
		//memory allocation failed
		MALLOCALLOCATIONERROR();
		(*exitProgram)(-1);
	}
	//buffer
	lint_16 readSize = fread(buffer,1,fileSize,fileptr);
	//check if we read everything
	/*if(readSize != fileSize){
		fprintf(stdin,"\nCould not read the contents\n");
		exit(-1);
	}*/
	assert(readSize == fileSize);
	printf("\n%s\n",buffer);
	printf("\nRead Size = %li, File Size = %li\n",readSize,fileSize);
	printf("\n%lu\n",sizeof(buffer));
	BencoderReader(buffer);
	free(buffer);
	fclose(fileptr);
	return;
}

static void BencoderReader(const unsigned char* fileContent){
	//let do things step by step
	//Noow logically, the first character in the file will tell use
	//the type of value/data we are dealing with
	//can be:
	/*
	1.) bytestring
	2.) integer
	3.) lists
	4.) Dictionary
	*/

	//have pointers to navigate the file
	int pointer1 = 0;
	int pointer2 = 0;
	char firstCharacter = fileContent[0];

	printf("\n%c\n",firstCharacter);
}

static void FAILUREEXIT(int errorCode){
	exit(errorCode);
}

int main(int argc, char ** argv){
	readTorrentFile(argv[1]);
}
