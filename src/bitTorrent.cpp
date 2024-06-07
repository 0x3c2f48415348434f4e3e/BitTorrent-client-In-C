#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef unsigned long int lint_16;

static void readTorrentFile(const char* filePath);



//Make torrent file is in current directory
static void readTorrentFile(const char* filePath){
	FILE *fileptr;
	fileptr = fopen(filePath,"rb");
	if(!fileptr){
		fprintf(stdout,"\nUnable to open given file\n");
		exit(-1);
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
	lint_16 readSize = fread(buffer,1,fileSize,fileptr);
	//check if we read everything
	/*if(readSize != fileSize){
		fprintf(stdin,"\nCould not read the contents\n");
		exit(-1);
	}*/
	assert(readSize == fileSize);
	printf("%s",buffer);
	free(buffer);
	fclose(fileptr);
	return;
}

int main(int argc, char ** argv){
	readTorrentFile(argv[1]);
}
