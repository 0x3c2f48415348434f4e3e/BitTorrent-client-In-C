#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stddef.h>
#include<string.h>

typedef unsigned long int lint_16;
#define BUFFER 1024
#define TORRENTFILENOTFOUNDERRORLOG(...) {printf("In file %s, given file not found: %s\n",__FILE__,__VA_ARGS__);}
#define MALLOCALLOCATIONERROR(...) {printf("In file %s, Memory allocation failed",__FILE__);}

//not really necessary but why not. Could have used Enums
typedef struct{
	char BYTE_STRING;
	char INTEGER;
	char LIST;
	char DICTIONARY;
}Bencoder;

//for our actual BencodeParser.
//The way this works is that we are going to have a structure that represents a single element like a string, integer, List, Dictionary
// etc, and then within this structure we will have a linked list that represents the next element.
//We are using a union within this structure which will allow us to represent any data
typedef struct B bencodeParser;
typedef struct B {
    char dataType[100];
	union{
		lint_16 integerValue;
		unsigned char* stringValue;
		bencodeParser *List_Dictionary;
	}value;
	bencodeParser *next;
}bencodeParser;

//To learn more about implementing a lexer, read the below article, also talks about dumping the Bencode data into Formats like XML etc
//Article about implementing Bencode parser: https://www.codeproject.com/articles/37533/bencode-lexing-in-c-2nd-stage

//https://blog.jse.li/posts/torrent/

Bencoder BencoderValues;

//static unsigned char* readTorrentFile(const char* filePath);
static unsigned char* readTorrentFile(const char* filePath, lint_16 *fileSize);
static int compare(const char* string);
static void FAILUREEXIT(int errorCode);
static lint_16 lexer(const unsigned char *stream);
static int stringToInteger(unsigned char *string);
static char* subString(const char *string, lint_16 pointer);
static void readBencode(bencodeParser *Head);	
static void isValidTorrent(const char * fileExtension);

//due to the way that the printf() method works in c in
//regards ti handling null terminating characters, we will
//create our custom own
static void printBuffer(const unsigned char* buffer, lint_16 size);
void (*exitProgram)(int) = &FAILUREEXIT;



static unsigned char* readTorrentFile(const char* filePath, lint_16 *fileSize) {
    FILE *fileptr = fopen(filePath, "rb"); // Open file in binary mode
    if (!fileptr) {
        TORRENTFILENOTFOUNDERRORLOG(filePath);
        (*exitProgram)(-1);
    }

    fseek(fileptr, 0, SEEK_END);
    *fileSize = ftell(fileptr);
    //check if ftell returns -1
    fseek(fileptr, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(*fileSize + 1); // Allocate extra space for null terminator
    if (buffer == NULL) {
        MALLOCALLOCATIONERROR();
        fclose(fileptr);
        (*exitProgram)(-1);
    }
    memset(buffer,0,sizeof(char)*(*fileSize+1));

    lint_16 readSize = fread(buffer, 1, *fileSize, fileptr);
    //check if fread return -1

    buffer[*fileSize] = '\0'; // Null-terminate the buffer
    assert(readSize == *fileSize);

  
    fclose(fileptr);
    return buffer;
}

static int compare(const char* string){

	
	// * Will return:
	// * 1 for string
	// * 2 for integer
	// * 3 for List and dictionary
	// *
    
	if(strcmp(string,"String") == 0) return 1;
	if(strcmp(string,"Integer") == 0) return 2;
	if((strcmp(string,"List") == 0) || (strcmp(string,"Dictionary") == 0)) return 3;
    return -1;
}

static void FAILUREEXIT(int errorCode){
	exit(errorCode);
}

//What we will actually do is that within the 
//integer and string part
// we will not add anny recursion
//insted we return the pointer to the 
//call function in the List/Dictionary
//then add that returned poiunter to the
//pointer in the List/Dictionary

bencodeParser *Head = NULL;
bencodeParser *Tail = NULL;

static lint_16 lexer(const unsigned char *stream) {
    //printf("%s\n",stream);
    lint_16 pointer = 0; // points to beginning of first character in stream
    lint_16 pointer2 = 0; // allow us to navigate out final string result
    unsigned char *lexerResult = (unsigned char *) malloc(sizeof(char) * BUFFER);
    if (lexerResult == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        (*exitProgram)(-1);
    }

    bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
    if (element == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        (*exitProgram)(-1);
    }
    element->next = NULL;

    if (stream[pointer] == BencoderValues.INTEGER) {
        strcpy(element->dataType, "Integer");
        pointer++; // skip 'i'
        while (stream[pointer] != 'e') {
            lexerResult[pointer2++] = stream[pointer++];
        }
        lexerResult[pointer2] = '\0';
        element->value.integerValue = stringToInteger(lexerResult);
        pointer++; // skip 'e'
    } else if (stream[pointer] == BencoderValues.LIST) {
        //Will add all nested children before adding List
        strcpy(element->dataType, "List");
        pointer++; // skip 'l'
        while (stream[pointer] != 'e') {
            pointer += lexer(stream + pointer);
        }
        pointer++; // skip 'e'
    } else if (stream[pointer] == BencoderValues.DICTIONARY) {
        //Will add all nested children before adding Dictionary
        strcpy(element->dataType, "Dictionary");
        pointer++; // skip 'd'
        while (stream[pointer] != 'e') {
            pointer += lexer(stream + pointer);
        }
        pointer++; // skip 'e'
    } else {
        strcpy(element->dataType, "String");
        unsigned char num[1000] = {0};
        int tempPoint = 0;
        while (stream[pointer] != ':') {
            num[tempPoint++] = stream[pointer++];
        }
        num[tempPoint] = '\0';
        int lengthOfString = stringToInteger(num);
        pointer++; // skip ':'
        element->value.stringValue = (unsigned char *)malloc((lengthOfString + 1) * sizeof(unsigned char));
        if (element->value.stringValue == NULL) {
            fprintf(stderr, "Unable to initialise memory");
            (*exitProgram)(-1);
        }
        memcpy(element->value.stringValue, &stream[pointer], lengthOfString);
        element->value.stringValue[lengthOfString] = '\0';
        pointer += lengthOfString;
    }

    if (Head == NULL) {
        Head = element;
        Tail = element;
    } else {
        Tail->next = element;
        Tail = element;
    }

    free(lexerResult);
    return pointer;
}


static int stringToInteger(unsigned char *string) {
    int res = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        res = (res * 10) + (string[i] - '0');
    }
    return res;
}

static char* subString(const char *string, lint_16 pointer) {
    char *result = (char *) malloc(sizeof(char) * BUFFER);
    if (result == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        exit(-1);
    }
    int i = 0;
    int currentBufferSize = BUFFER;
    while (string[pointer] != '\0') {
        if (i >= currentBufferSize) {
            currentBufferSize += BUFFER;
            result = (char *) realloc(result, currentBufferSize);
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

static void readBencode(bencodeParser *Head){
	while(Head){
        printf("\n%s\n",Head->dataType);
		if(compare(Head->dataType) == 1){
			printf("%s\n",Head->value.stringValue);
		}
		else if(compare(Head->dataType) == 2){
			printf("%li\n",Head->value.integerValue);
		}
		else if(compare(Head->dataType) == 3 || compare(Head->dataType) == 3){
			readBencode(Head->value.List_Dictionary);
		}
        else{
            printf("Unable to find");
        }
		Head = Head->next;
	}
    return;
}

static void printBuffer(const unsigned char* buffer, lint_16 size){
    lint_16 counter = 0;
    for(; counter<size; counter++){
        //Looking at the ASCII table
        //we want to ignore character less than 32
        if(buffer[counter] >= 32 && buffer[counter] <= 126){
            putchar(buffer[counter]);
        }
        else{
            printf("\\x%02x", buffer[counter]);
        }
       
    }
    printf("\n");
}


int main(int argc, char ** argv){
	// *char BYTE_STRING;                                                           
    // char INTEGER;                                                               
    // char LIST;                                                                  
    // char DICTIONARY;
	 
	BencoderValues.BYTE_STRING = ' ';
    BencoderValues.INTEGER = 'i';
	BencoderValues.LIST = 'l';
	BencoderValues.DICTIONARY = 'd';
	//unsigned char *fileContent = readTorrentFile(argv[1]);
    lint_16 fileSize;	
    const unsigned char *fileContent = readTorrentFile((const char*) argv[1], &fileSize);
    //printBuffer(fileContent,fileSize);
    unsigned char *tempFileContent = (unsigned char *)  malloc(sizeof(char)*fileSize);
    if(tempFileContent == NULL){
        fprintf(stderr, "Unable to initialise memory");
        (*exitProgram)(-1);;
    }

    lint_16 counter = 0;
    for(; counter< fileSize; counter++){
        if(fileContent[counter] >= 32 && fileContent[counter] <= 126){
            //replace with 0
            tempFileContent[counter] = fileContent[counter];
        }
        else{
            tempFileContent[counter] = '1'; //replace useless chracters with a 1
        }
    }
    
	//lexer(tempFileContent);

    lint_16 pointer = lexer(tempFileContent);
    //printf("\n%s",fileContent);
    readBencode(Head);
    free(tempFileContent);
	free((void *) fileContent);
}