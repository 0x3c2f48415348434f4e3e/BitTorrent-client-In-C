#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stddef.h>
#include<string.h>

//will comment a lot, as this is just what i like to do
typedef unsigned long int lint_16; // allow for more values to be represented
#define BUFFER 1024
#define TORRENTFILENOTFOUNDERRORLOG(...) {printf("In file %s, line %i, given file not found: %s\n", __FILE__, __LINE__, __VA_ARGS__);}
#define MALLOCALLOCATIONERROR(...) {printf("In file %s, line %i, Memory allocation failed: %s", __FILE__, __LINE__, __VA_ARGS__);}

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

static unsigned char* readTorrentFile(const char* filePath, lint_16 *fileSize); //function declearation for reading file
static int compare(const char* string);
static void FAILUREEXIT(int errorCode);
static lint_16 lexer(const unsigned char *stream);
static int stringToInteger(unsigned char *string);
static char* subString(const char *string, lint_16 pointer);
static void readBencode(bencodeParser *Head);	
static void isValidTorrent(const char * fileExtension);
static int compareStrings(const char* str1, const char* str2, int lenOfFileExtension, int pointerForStr1, int pointerForStr2);
//The C language does not support function overloading, so lets add extremely bad code
static int compareUnsignedStrings(const unsigned char* str1, const char* str2, int lenOfFileExtension, int pointerForStr1, int pointerForStr2);

static void SHA1(const unsigned char *infoDictionary, lint_16 size);
static void requestTracker();

//due to the way that the printf() method works in c in
//regards ti handling null terminating characters, we will
//create our custom own
static void printBuffer(const unsigned char* buffer, lint_16 size);
void (*exitProgram)(int) = &FAILUREEXIT;

static int compareStrings(const char* str1, const char* str2, int lenOfFileExtension, int pointerForStr1, int pointerForStr2){
    //have an int to represent true or false
    //0 is false
    //1 is true
    int valid = 1;
    while(lenOfFileExtension){
        if(str1[pointerForStr1] != str2[pointerForStr2]){
            valid = 0;
            return valid;
        }
        //remeber we are passing pointerForStr1 and pointerForStr2 by value so chnages we make here will not affect the actual value we pass to the function
        pointerForStr1++;
        pointerForStr2++;
        lenOfFileExtension--;
    }

    return valid;
}

//function overloading i guess
static int compareUnsignedStrings(const unsigned char* str1, const char* str2, int lenOfFileExtension, int pointerForStr1, int pointerForStr2){
    //have an int to represent true or false
    //0 is false
    //1 is true
    int valid = 1;
    while(lenOfFileExtension){
        if(str1[pointerForStr1] != str2[pointerForStr2]){
            valid = 0;
            return valid;
        }
        //remeber we are passing pointerForStr1 and pointerForStr2 by value so chnages we make here will not affect the actual value we pass to the function
        pointerForStr1++;
        pointerForStr2++;
        lenOfFileExtension--;
    }

    return valid;
}

static void isValidTorrent(const char * fileExtension){
    //will check weather given file is a valide torrent file based solely on the file extension, otherwise, we exit
    const char *torrentName = ".torrent";
    //get the last .
    int lastPeriodCharacter = 0;
    int pointer = 0;
    while(*(fileExtension+pointer) !=  '\0'){
        //check if we have a period (use ASCII, more safer, i think)
        if(*(fileExtension+pointer) == 46){
            lastPeriodCharacter = pointer;
        }
        pointer++;
    }
    //since we have the last period, check the extension
    int str2Pointer = 0;
    if(compareStrings(fileExtension,torrentName,strlen(torrentName),lastPeriodCharacter,str2Pointer) == 1){
        printf("Valid File extension\n");
    }
    else{
        printf("Invalid File Extension\n");
        (*exitProgram)(-1);
    }
}

static unsigned char* readTorrentFile(const char* filePath, lint_16 *fileSize) {
    isValidTorrent(filePath);
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
        MALLOCALLOCATIONERROR("Varibale buffer could not be allocated");
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


//global varible to store position of the info dictionary
lint_16 pointerToInfoDictioinary = 0;

//global varibale to store the end of the info
//dictionary. Since we know the structure of a
//torrent file already that is the end of the 
//info dictorent will be before the end of the parent 
//dictionary
lint_16 pointerToEndOfInfoDictionary = 0;

//another global varibale to keep running, so every
//where we increment our pointer, we also increment
//this varibale
lint_16 infoPointerKeepRunning = 0;
static lint_16 lexer(const unsigned char *stream) {
    const char *INFODICT = "info";
    //printf("%s\n",stream);
    lint_16 pointer = 0; // points to beginning of first character in stream
    lint_16 pointer2 = 0; // allow us to navigate out final string result
    unsigned char *lexerResult = (unsigned char *) malloc(sizeof(char) * BUFFER);
    if (lexerResult == NULL) {
        MALLOCALLOCATIONERROR("Varibale lexerResult could not be allocated");
        (*exitProgram)(-1);
    }

    bencodeParser *element = (bencodeParser *) malloc(sizeof(bencodeParser));
    if (element == NULL) {
        MALLOCALLOCATIONERROR("Varibale element could not be allocated");
        (*exitProgram)(-1);
    }
    element->next = NULL;

    if (stream[pointer] == BencoderValues.INTEGER) {
        strcpy(element->dataType, "Integer");
        pointer++; // skip 'i'
        infoPointerKeepRunning++;
        while (stream[pointer] != 'e') {
            //check if pointer2(which is used for lexerResult surpasses BUFFER which is the allocated Buffer for lexerResult, if true realloc)
            lint_16 currentBufferSize = BUFFER;
            if ((pointer2) >= currentBufferSize) {
                currentBufferSize += BUFFER;
                lexerResult = (unsigned char *) realloc(lexerResult, currentBufferSize);
                if (lexerResult == NULL) {
                    MALLOCALLOCATIONERROR("Varibale lexerResult could not be allocated");
                    (*exitProgram)(-1);
                }
            }
            lexerResult[pointer2++] = stream[pointer++];
            infoPointerKeepRunning++;
        }
        lexerResult[pointer2] = '\0';
        element->value.integerValue = stringToInteger(lexerResult);
        pointer++; // skip 'e'
        infoPointerKeepRunning++;
    } else if (stream[pointer] == BencoderValues.LIST) {
        //Will add all nested children before adding List
        strcpy(element->dataType, "List");
        pointer++; // skip 'l'
        infoPointerKeepRunning++;
        while (stream[pointer] != 'e') {
            pointer += lexer(stream + pointer);
        }
        pointer++; // skip 'e'
        infoPointerKeepRunning++;
    } else if (stream[pointer] == BencoderValues.DICTIONARY) {
        //Will add all nested children before adding Dictionary
        strcpy(element->dataType, "Dictionary");
        pointer++; // skip 'd'
        infoPointerKeepRunning++;
        while (stream[pointer] != 'e') {
            pointer += lexer(stream + pointer);
        }
        pointer++; // skip 'e'
        infoPointerKeepRunning++;
    } else {
        strcpy(element->dataType, "String");
        unsigned char num[1000] = {0};
        int tempPoint = 0;
        while (stream[pointer] != ':') {
            num[tempPoint++] = stream[pointer++];
            infoPointerKeepRunning++;
        }
        num[tempPoint] = '\0';
        //get length of string and allocate memeory using that length
        int lengthOfString = stringToInteger(num);
        pointer++; // skip ':'
        infoPointerKeepRunning++;
        element->value.stringValue = (unsigned char *)malloc((lengthOfString + 1) * sizeof(unsigned char));
        if (element->value.stringValue == NULL) {
            MALLOCALLOCATIONERROR("Varibale element->value.stringValue could not be allocated");
            (*exitProgram)(-1);
        }
        memcpy(element->value.stringValue, &stream[pointer], lengthOfString);
        int str1Pointer = 0;
        int str2Pointer = 0;
        if(compareUnsignedStrings(element->value.stringValue,INFODICT,strlen(INFODICT),str1Pointer,str2Pointer) == 1){
            printf("\nFound the Info Dictionary: %s\n",element->value.stringValue);
            pointerToInfoDictioinary = infoPointerKeepRunning;
        }
        element->value.stringValue[lengthOfString] = '\0';
        pointer += lengthOfString;
        infoPointerKeepRunning+=lengthOfString;
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
        MALLOCALLOCATIONERROR("Varibale result could not be allocated");
        (*exitProgram)(-1);
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


//SHA-1 algorithm implementation
static void SHA1(const unsigned char *infoDictionary, lint_16 size){
/*
What we do here is take as input the info dictionary
without decoding it from the bencode format. then
we use the SHA1 format

What we will do first is find the position of the info dict within the infoDictionary input
*/
    printBuffer(infoDictionary,size);

}

//So what we have to do next, is get the value of the annouce (As this is the URL of trh tracker)
static void requestTracker(){
/*To build an apporiate request, there
are several things to keep in mind
1. Get a socket up to do the request
2. make sure the request url has all the necessary
parameters including:
a. info_hash - Need to use SHA1 algorithm. What this
does is takes an input and produces a fixed size
string of byte.
b. peer_id
c. uploaded
d. downloaded
e. left
f. port
g. compact
*/


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

    unsigned char *tempFileContent = (unsigned char *)  malloc(sizeof(char)*fileSize);
    if(tempFileContent == NULL){
        MALLOCALLOCATIONERROR("Varibale tempFileContent could not be allocated");
        (*exitProgram)(-1);
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
    

    lint_16 pointer = lexer(tempFileContent);
    pointerToEndOfInfoDictionary = fileSize;
    printf("\nPointer: %ld\n",pointer);
    printf("\nfileSize: %ld\n",fileSize);
    printf("\npointerToEndOfInfoDictioinary: %ld\n", pointerToEndOfInfoDictionary);
    printf("\npointertoInfoDictionary: %ld\n", pointerToInfoDictioinary);
    //subtract twice as the info is nested inside the parent
    //dictionary;
    pointerToEndOfInfoDictionary-=2;

    //lint_16 calculateResultant = pointerToEndOfInfoDictionary-pointerToInfoDictioinary;
    printf("\nstuff: %ld\n", (pointerToEndOfInfoDictionary-pointerToInfoDictioinary));
    unsigned char *info_hash = (unsigned char*) malloc(sizeof(char)*(pointerToEndOfInfoDictionary-pointerToInfoDictioinary));
    memset(info_hash,'0',sizeof(char)*(pointerToEndOfInfoDictionary-pointerToInfoDictioinary));
    //check for allocation failure
        if (info_hash == NULL) {
            MALLOCALLOCATIONERROR("Varibale element->value.stringValue could not be allocated");
            (*exitProgram)(-1);
        }

    //readBencode(Head);

    lint_16 copyOfPointerToInfoDictionary = pointerToInfoDictioinary;
    lint_16 counterInfo_hash = 0;

    for(; counterInfo_hash<=(pointerToEndOfInfoDictionary-pointerToInfoDictioinary); counterInfo_hash++, copyOfPointerToInfoDictionary++){
        *(info_hash+counterInfo_hash) =  *(fileContent+copyOfPointerToInfoDictionary);
    }

    //major memory issue - Not really memory issue, more of an undefined behaviour
    printf("\nLast Character: %c, %ld\n",*(info_hash+((pointerToEndOfInfoDictionary-pointerToInfoDictioinary))), (pointerToEndOfInfoDictionary-pointerToInfoDictioinary));

    //printf("\n%s\n",info_hash);

    /*
    //Debugging
    printf("\nPointer to info Dict is: %li\n",pointerToInfoDictioinary);
    printf("\n%c",fileContent[pointerToInfoDictioinary]);
    printf("\n%c",fileContent[pointerToInfoDictioinary+1]);
    printf("\n%c",fileContent[pointerToInfoDictioinary+2]);
    printf("\n%c",fileContent[pointerToInfoDictioinary+3]);
    
    printf("\nEnd of info Dictionary is: %li\n", pointerToEndOfInfoDictionary);
    printf("\n%c",fileContent[pointerToEndOfInfoDictionary]);
    printf("\n%s",fileContent);
    */

    SHA1((const unsigned char *) info_hash, ((pointerToEndOfInfoDictionary-pointerToInfoDictioinary)+1)); //+1 for the e
    free(info_hash);
    free(tempFileContent);
	free((void *) fileContent);
}