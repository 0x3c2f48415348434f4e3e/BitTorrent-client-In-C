#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stddef.h>
#include<string.h>

typedef unsigned long int lint_16;
#define BUFFER 2028
#define TORRENTFILENOTFOUNDERRORLOG(...) {printf("In file %s, given file not found: %s\n",__FILE__,__VA_ARGS__);}
#define MALLOCALLOCATIONERROR(...) {printf("In file %s, Memory allocation failed",__FILE__);}
typedef struct{
	char BYTE_STRING;
	char INTEGER;
	char LIST;
	char DICTIONARY;
}Bencoder;

//for our actual BencodeParser.
//The way this works is that we are going to have a structure that represents a single element like a string, integer, List, Dictionary etc, and then within this structure we will have a linked list that represents the next element
typedef struct B bencodeParser;
typedef struct B {
    char dataType[100];
	union{
		lint_16 integerValue;
		char* stringValue;
		bencodeParser *List_Dictionary;
	}value;
	bencodeParser *next;
}bencodeParser;

//Article about implementing Bencode parser: https://www.codeproject.com/articles/37533/bencode-lexing-in-c-2nd-stage

//https://blog.jse.li/posts/torrent/

Bencoder BencoderValues;// = (Bencoder){' ','i','l','d'};

#define SIZE
typedef struct{
	int sizeOfMap;
	char ***MapData;
}Map;

static char* readTorrentFile(const char* filePath);
static int compare(char* string);
static void FAILUREEXIT(int errorCode);
static void lexer(char *stream);
static int stringToInteger(char *string);
static char* subString(char *string, lint_16 pointer);
static void readBencode(bencodeParser *Head);	
	//We will be implementing a map to store the values we get decode from
//the torrent file

//Make sure file is in current directory
static char* readTorrentFile(const char* filePath){
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
	char *buffer = (char *) malloc(fileSize);
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
	//printf("\n%s\n",buffer);
	//printf("\nRead Size = %li, File Size = %li\n",readSize,fileSize);
	//printf("\n%lu\n",sizeof(buffer));
	//BencoderReader(buffer);
	//free(buffer);
	fclose(fileptr);
	return buffer;
}

static int compare(char* string){

	/*
	 * Will return:
	 * 1 for string
	 * 2 for integer
	 * 3 for List and dictionary
	 */
    
	/*if(strcpy(string,myString) == 0) return 1;
	if(strcpy(string,myInteger) == 0) return 2;
	if((strcpy(string,myList) == 0) || (strcpy(string,myDictionary) == 0)) return 3;*/
	return -1; //for some error
}

static void FAILUREEXIT(int errorCode){
	exit(errorCode);
}


static void lexer(char *stream) {
    //check if the string is not end
    bencodeParser *Head = NULL;
    bencodeParser *Tail = NULL;
    
    lint_16 pointer = 0; // points to beginning of first character in stream
    lint_16 pointer2 = 0; // allow us to navigate out final string result
    char *lexerResult = (char *) malloc(sizeof(char) * BUFFER);
    // check if memory allocation was successful
    if (lexerResult == NULL) {
        fprintf(stderr, "Unable to initialise memory");
        exit(-1);
    }
    
    bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
    // check if memory allocation was successful
    if(element == NULL){
        fprintf(stderr, "Unable to initialise memory");
        exit(-1);
    }
    element->next = NULL;
    
    if (*(stream + pointer) == BencoderValues.INTEGER) {
	   strcpy(element->dataType,"Integer");
        //element->next = NULL;
	    //printf("Is an Integer\n");
	    //make a node here for the INTEGER value

        pointer++;//skip the i
        //check for negative
        while (*(stream + pointer) != 'e') {
            *(lexerResult + pointer2) = *(stream + pointer);
            pointer++;
            pointer2++;
        }

    } else if (*(stream + pointer) == BencoderValues.LIST) {
	//bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
        strcpy(element->dataType,"List");
	    //printf("Is a List\n");
        pointer++;//skip the l
        while (*(stream + pointer) != 'e') {
            //list can have nested data structures so we accompany for that
	        if (*(stream + pointer) == BencoderValues.INTEGER || *(stream + pointer) == BencoderValues.LIST || *(stream + pointer) == BencoderValues.DICTIONARY){
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
        //bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
        strcpy(element->dataType,"Dictionary");
	    
	    //printf("Is a dictionary\n");
    
        pointer++;
        while (*(stream + pointer) != 'e') {
            lexer(stream + pointer);
            while (*(stream + pointer) != 'e') pointer++;
        }
        pointer++; // Skip 'e'
    } else {
	    //bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
        strcpy(element->dataType,"String");
        //element->next = NULL;
        //printf("Is a String\n");
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
    
    //element->next = NULL;
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

    //printf("%s\n",lexerResult);
	
    readBencode(Head);

    free(element);
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
		if(strcmp(Head->dataType, "String") == 0){
			printf("%s",Head->value.stringValue);
		}
		else if(strcmp(Head->dataType,"Integer") == 0){
			printf("%li",Head->value.integerValue);
		}
		else if(strcmp(Head->dataType,"List") == 0 || strcpy(Head->dataType,"List") == 0){
			readBencode(Head->value.List_Dictionary);
		}
        else{
            printf("Unable to find");
        }
		Head = Head->next;
	}
}

int main(int argc, char ** argv){
	printf("Main");
	/*
	 *char BYTE_STRING;                                                           char INTEGER;                                                               char LIST;                                                                  char DICTIONARY;
	 */
	BencoderValues.BYTE_STRING = ' ';
       	BencoderValues.INTEGER = 'i';
	BencoderValues.LIST = 'l';
	BencoderValues.DICTIONARY = 'd';
	char *fileContent = readTorrentFile(argv[1]);	
	lexer(fileContent);
	free(fileContent);
}
