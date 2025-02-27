/*
Name : Huraimah Fatima
Email : huraimah@uoguelph.ca
*/

#include "LinkedListAPI.h"
#include "VCParser.h"
#include "VCParsingHelper.h"

//contstant for min file size
#define MIN_FILE_SIZE 57

VCardErrorCode iterateAndValidate (char* fileString, int size, char** newLine, int* iterator);