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
static void lexer(const unsigned char *stream);
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

static void lexer(const unsigned char *stream) {
    printf("Lexer: %s\n",stream);
    //check if the string is not end
    bencodeParser *Head = NULL;
    bencodeParser *Tail = NULL;
    
    lint_16 pointer = 0; // points to beginning of first character in stream
    lint_16 pointer2 = 0; // allow us to navigate out final string result
    unsigned char *lexerResult = (unsigned char *) malloc(sizeof(char) * BUFFER);
    // check if memory allocation was successful
    if (lexerResult == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        (*exitProgram)(-1);
    }
    
    bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
    // check if memory allocation was successful
    if(element == NULL){
        fprintf(stderr, "Unable to initialise memory");
        (*exitProgram)(-1);
    }
    element->next = NULL;
    

        if (*(stream + pointer) == BencoderValues.INTEGER) {
	        strcpy(element->dataType,"Integer");
	        //make a node here for the INTEGER value

            pointer++;//skip the i
            //check for negative
            while (*(stream + pointer) != 'e') {
                *(lexerResult + pointer2) = *(stream + pointer);
                pointer++;
                pointer2++;
            }
            *(lexerResult+pointer2) = '\0';
            element->value.integerValue = stringToInteger(lexerResult);

        } else if (*(stream + pointer) == BencoderValues.LIST) {
            strcpy(element->dataType,"List");
            pointer++;//skip the l
            while (*(stream + pointer) != 'e') {
                //list can have nested data structures so we accompany for that
	            if (*(stream + pointer) == BencoderValues.INTEGER || *(stream + pointer) == BencoderValues.LIST || *(stream + pointer) == BencoderValues.DICTIONARY){
                    lexer(stream + pointer);
        	        while (*(stream + pointer) != 'e'){
			            pointer++; //move pointer to final position of e
                    }
                } else {
                    unsigned char num[1000] = {0};
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
                }
            }
            //pointer++; // Skip 'e'
        } else if (*(stream + pointer) == BencoderValues.DICTIONARY) {
            strcpy(element->dataType,"Dictionary");
    
            pointer++;
            while (*(stream + pointer) != 'e') {
                lexer(stream + pointer);
                while (*(stream + pointer) != 'e') pointer++;
            }
            pointer++; // Skip 'e'
        } else {
            strcpy(element->dataType,"String");
            unsigned char num[1000] = {0};
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
            if (*(stream + pointer) != '\0') {
                lexer(stream + pointer);
            }
            *(lexerResult+pointer2) = '\0';
            //printf("Moving across %s\n",lexerResult);
            element->value.stringValue = lexerResult;

        }

        if(Head == NULL){
            //linked list is empty
	        Head = element;
            Tail = element;
        }
        else{
            //We have stuff in our linked list
            //for our linked list, think of it like a snake, that is the head will always stay in the same position and the body/tail will grow
            Tail->next = element;
            Tail = element;
        }
   
	
    readBencode(Head);

    free(element);
    free(lexerResult);
    
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
        //printf("\n%s\n",Head->dataType);
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
	printf("ok\n");
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
            tempFileContent[counter] = '1';
        }
    }
    //printf("%s\n",tempFileContent);
	lexer(tempFileContent);
    //printf("\n%s",fileContent);
    free(tempFileContent);
	free((void *) fileContent);
}