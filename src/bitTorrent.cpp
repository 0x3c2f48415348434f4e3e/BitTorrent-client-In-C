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
	char LIST;
	char DICTIONARY;
}Bencoder;


//Article about implementing Bencode parser: https://www.codeproject.com/articles/37533/bencode-lexing-in-c-2nd-stage


Bencoder BencoderValues = (Bencoder){' ','i','l','d'};

#define SIZE
typedef struct{
	int sizeOfMap;
	char ***MapData;
}Map;

static void readTorrentFile(const char* filePath);
static void BencoderReader(const unsigned char* fileContent);
static void FAILUREEXIT(int errorCode);
static void lexer();
static void stringToInteger(char *string);
//We will be implementing a map to store the values we get decode from
//the torrent file

/*
A map is essentially a key-value pair link
*/
static Map* createMap(); //Will create our map

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
	//printf("\n%lu\n",sizeof(buffer));
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


	//This algorithm assumes that the torrent file is correctly encoded using Bencode
	//check if we have a dictionary, list or integer, otherwise we have a string

	//This is for just the first character

	if(firstCharacter == BencoderValues.INTEGER){ //is an Integer
		printf("\nInteger\n");
	}
	else if(firstCharacter == BencoderValues.LIST){//is a List
		printf("\nList\n");
	}
	else if(firstCharacter == BencoderValues.DICTIONARY){//is a Dictionary
		printf("\nDictionary\n");
	}
	else{//if above conditions are not true, must be a BYTE string
		printf("\nByte String\n");
	}

}

static void FAILUREEXIT(int errorCode){
	exit(errorCode);
}

static Map* createMap(){
	Map map = {0,NULL};
	return map;
}

static void lexer(char *stream)
	//We will try and just create a lexer that takes the input
	//stream and convets it to (somewhat) token and outputs it
	//
	//we will need 2 pointers
	lint_16 pointer = 0; //points to beginning of first character in stream
	lint_16 pointer2 = 0; //allow us to navigate out final string result
	char *lexerResult = malloc(sizeof(char)*1024);
	//char firstCharacter = stream; //stream is a pointer that points to ffirst character
	int StreamNotEnd = 1;
	//use while loop here
	if(*(stream+pointer) == BencoderValues.INTEGER){
		//increment pointer to skip i
		pointer++;
		while(*(stream+pointer) != 'e'){
			*(lexerResult+pointer2) = *(stream+pointer);
			pointer++;
			pointer2++;
		}
	else if(*(stream+pointer) == BencoderValues.LIST){
		//do stuff for list
		//increment pointer to skip the l
		pointer++;
		//will check if is negative later
		char num[1000]; //assuming we do not surpass the number 1000
		memset(num,0,size0f(char));
		int tempPoint = 0;
		while(*(stream+pointer) != ':'){
			*(num+tempPoint) = *(stream+pointer);
			tempPoint++;
			pointer++;
		}

		//coonver the string to an integer 
		int lengthOfString = stringToInteger(num);
		//incrment pointer to skip colon
		pointer++;
		register int i = 0;
		char string
		for(;i<lengthOfString;i++){
			*(lexerResult+pointer2) = *(stream+(pointer++)); 
		}
	}
	else if(*(stream+pointer) == BencoderValues.DICTIONARY){
		//do stuff for Dictionary
	}

		//do string as final
	else{
		//get the number, then add to the pointer
		//but before we get number, we have to use loop to see when we reach a colon :
		char * num = NULL;
		int tempPoint = 0;
		while(*(stream+pointer) ! = ':'){
			num[tempPoint] = *(stream+pointer);
			tempPoint++;
			pointer++;

		}


	}

	//testing

	printf("lexerResult");

	free(lexerResult)
}

static int stringToInteger(char *string){
	register int i = 0;
	static int res = 0;
	for(;string[i] != '\0';){
		res = (res*10) + (string[i] - 48);// ASCII stuff
		i++;
	}
	return res;
}

int main(int argc, char ** argv){
	readTorrentFile(argv[1]);
}
