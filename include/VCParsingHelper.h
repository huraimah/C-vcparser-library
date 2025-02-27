/*
Name : Huraimah Fatima
Email : huraimah@uoguelph.ca
*/

#include "LinkedListAPI.h"
#include "VCParser.h"
#include <ctype.h>

VCardErrorCode parseGroupName(char* newLine, char** groupName);

VCardErrorCode parsePropName(char* newLine, char** propName);

VCardErrorCode parseParameters(char* newLine, List* parameters);

VCardErrorCode parseValues (char* newLine, List* values);

VCardErrorCode buildOptionalProperties(char* newLine, List* optionalProperties);

VCardErrorCode extractFN(char* newLine, Card* obj);

VCardErrorCode dateAndTime (char* dtValue, DateTime* dt);