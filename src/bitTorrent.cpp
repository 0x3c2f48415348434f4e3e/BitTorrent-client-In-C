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

//https://blog.jse.li/posts/torrent/

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
static char* subString(char *string, lint_16 pointer)
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

static void lexer(char *stream) {
    lint_16 pointer = 0; // points to beginning of first character in stream
    lint_16 pointer2 = 0; // allow us to navigate out final string result
    char *lexerResult = malloc(sizeof(char) * BUFFER);

    // check if memory allocation was successful
    if (lexerResult == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        exit(-1);
    }

    if (*(stream + pointer) == BencoderValues.INTEGER) {
        printf("Is an Integer\n");
        pointer++;//skip the i
        //check for negative
        while (*(stream + pointer) != 'e') {
            *(lexerResult + pointer2) = *(stream + pointer);
            pointer++;
            pointer2++;
        }
        pointer++; // Skip 'e'
        //printf("Integer: %s\n", lexerResult);
        /*if (*(stream + pointer) != '\0') {
            lexer(stream + pointer);
        }*/
        //*(lexerResult + pointer2) = '\0';
    } else if (*(stream + pointer) == BencoderValues.LIST) {
        printf("Is a List\n");
        pointer++;//skip the l
        while (*(stream + pointer) != 'e') {
            //list can have nested data structures so we accompany for that
            if (*(stream + pointer) == 'i' || *(stream + pointer) == 'l' || *(stream + pointer) == 'd') {
                lexer(stream + pointer);
                while (*(stream + pointer) != 'e'){
                    pointer++; //move pointer to final position of e
                }
            } else {
                char num[1000] = {0};
                int tempPoint = 0;
                while (*(stream + pointer) != ':') {
                    num[tempPoint] = *(stream + pointer);
                    tempPoint++;
                    pointer++;
                }
                int lengthOfString = stringToInteger(num);
                pointer++; //skip colon
                for (int i = 0; i < lengthOfString; i++) {
                    *(lexerResult + pointer2++) = *(stream + pointer++);
                }
                //printf("String: %s\n", lexerResult);
            }
        }
        //pointer++; // Skip 'e'
    } else if (*(stream + pointer) == BencoderValues.DICTIONARY) {
        printf("Is a dictionary\n");
        pointer++;
        while (*(stream + pointer) != 'e') {
            lexer(stream + pointer);
            while (*(stream + pointer) != 'e') pointer++;
        }
        pointer++; // Skip 'e'
    } else {
        printf("Is a String\n");
        char num[1000] = {0};
        int tempPoint = 0;
        while (*(stream + pointer) != ':') {
            num[tempPoint] = *(stream + pointer);
            tempPoint++;
            pointer++;
        }
        int lengthOfString = stringToInteger(num);
        pointer++;
        for (int i = 0; i < lengthOfString; i++) {
            *(lexerResult + pointer2++) = *(stream + pointer++);
        }
        //*(lexerResult + pointer2) = '\0';
        //printf("String: %s\n", lexerResult);
        if (*(stream + pointer) != '\0') {
            lexer(stream + pointer);
        }
    }
    *(lexerResult+pointer2) = '\0';
    printf("%s\n",lexerResult);

    free(lexerResult);
}

static int stringToInteger(char *string) {
    int res = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        res = (res * 10) + (string[i] - '0');
    }
    return res;
}

static char* subString(char *string, lint_16 pointer) {
    char *result = malloc(sizeof(char) * BUFFER);
    if (result == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        exit(-1);
    }
    int i = 0;
    int currentBufferSize = BUFFER;
    while (string[pointer] != '\0') {
        if (i >= currentBufferSize) {
            currentBufferSize += BUFFER;
            result = realloc(result, currentBufferSize);
            if (result == NULL) {
                fprintf(stderr, "Unable to reallocate memory");
                exit(-1);
            }
        }
        result[i] = string[pointer];
        i++;
        pointer++;
    }
    result[i] = '\0'; // Null-terminate the string
    return result;
}

int main(int argc, char ** argv){
	readTorrentFile(argv[1]);
}
