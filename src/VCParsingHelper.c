/*
Name : Huraimah Fatima
Email : huraimah@uoguelph.ca
*/


#include "VCParsingHelper.h"


VCardErrorCode parseGroupName(char* newLine, char** groupName){

    char* result = NULL;
    int indexColon = 0;
    int indexPoint = 0;

    // check if the line is empty
    if(newLine == NULL){
        printf("Empty Line!");
        return INV_PROP;
    }

    // check if group name exists in line, it should be before the colon
    result = strchr(newLine, ':');
    if(result == NULL){
        printf("No property in line found!\n");
        return INV_PROP;
    }
    

    // get the index of the colon
    indexColon = result - newLine;

    // look for group name in the string before the colon index
    result = strchr(newLine, '.');
    if(result == NULL || result - newLine >= indexColon){
        *groupName = malloc(1);
        if (*groupName == NULL) {
            return OTHER_ERROR;
        }
        (*groupName)[0] = '\0';
        return OK;
    }
    // get the index of the point
    indexPoint = result - newLine;

    if (indexPoint > indexColon){
        printf("No group name found!\n");
        return OK;
    }

    //allocate memory for the group name
     *groupName = malloc(sizeof(char)*256);

     if(*groupName == NULL){
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
     }

    // copy the group name into the group name variable
    strncpy(*groupName, newLine, indexPoint);
    (*groupName)[indexPoint] = '\0';

    return OK;
}
VCardErrorCode parsePropName(char* newLine, char** propName) {
    char* colonPos = NULL;
    char* dotPos = NULL;
    char* semiColonPos = NULL;
    int indexColon = 0;
    int indexStart = 0;
    int propLength = 0;

    // Check if the line is empty
    if (newLine == NULL) {
        printf("Empty Line!\n");
        return INV_PROP;
    }

    // Locate the colon (`:`), which separates property name from values
    colonPos = strchr(newLine, ':');
    if (colonPos == NULL) {
        printf("No property in line found!\n");
        return INV_PROP;
    }
    indexColon = colonPos - newLine;
    if (indexColon == 0) {
        return INV_PROP;
    }

    // Locate the dot (`.`) which separates group name (if exists)
    dotPos = strchr(newLine, '.');
    if (dotPos != NULL && (dotPos - newLine) < indexColon) {
        indexStart = (dotPos - newLine) + 1; // Start after `.`
    } else {
        indexStart = 0; // No group name, start from the beginning
    }

    // Locate the semicolon (`;`), which separates parameters (if exists)
    semiColonPos = strchr(newLine, ';');
    if (semiColonPos != NULL && (semiColonPos - newLine) < indexColon) {
        propLength = semiColonPos - newLine - indexStart;
    } else {
        propLength = indexColon - indexStart; // Extract full property name
    }

    // Allocate only required memory
    *propName = malloc(propLength + 1);
    if (*propName == NULL) {
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }

    // Copy the property name
    strncpy(*propName, newLine + indexStart, propLength);
    (*propName)[propLength] = '\0'; // Null-terminate the string

    // Check if property name is empty after extraction
    if (strlen(*propName) == 0) {
        printf("No property name found!\n");
        free(*propName);
        return INV_PROP;
    }

    return OK;
}


VCardErrorCode parseValues(char* newLine, List* values) {
    if (newLine == NULL || values == NULL) {
        return INV_PROP;
    }

    // Locate the first colon which separates property name and values
    char* result = strchr(newLine, ':');
    if (result == NULL || *(result + 1) == '\0') {
        return INV_PROP; // No colon means no values, or empty value case
    }

    char* valueStart = result + 1; // Move past the colon

    // Allocate a temporary buffer for parsing values
    char* tempValue = malloc(strlen(valueStart) + 1);
    if (tempValue == NULL) {
        return OTHER_ERROR;
    }
    strcpy(tempValue, valueStart);

    char* token = tempValue;
    do {
        char* nextToken = strchr(token, ';'); // Find next semicolon
        if (nextToken != NULL) {
            *nextToken = '\0'; // Replace ';' with null terminator
            nextToken++; // Move to the next character
        }

        // Trim leading and trailing spaces
        while (*token == ' ') token++; // Skip leading spaces

        // Allocate memory for value, even if it's an empty string (for cases like ";;")
        char* value = malloc(strlen(token) + 1);
        if (value == NULL) {
            free(tempValue);
            return OTHER_ERROR;
        }
        strcpy(value, token);

        // Trim trailing spaces
        int len = strlen(value);
        while (len > 0 && isspace(value[len - 1])) {
            value[len - 1] = '\0';
            len--;
        }

        // Insert value into the list
        insertBack(values, value);

        token = nextToken; // Move to next token
    } while (token != NULL); // Ensure full parsing

    free(tempValue);
    return OK;
}

VCardErrorCode parseParameters(char* newLine, List* parameters) {
    if (newLine == NULL || parameters == NULL) {
        printf("Invalid input parameters!\n");
        return INV_PROP;
    }

    char* colonPos = strchr(newLine, ':');
    char* semiColonPos = strchr(newLine, ';');

    if (colonPos == NULL || semiColonPos == NULL || semiColonPos > colonPos) {
        return OK;  // No parameters present
    }

    // Allocate memory for parameter extraction
    char* temp = malloc(colonPos - semiColonPos);
    if (temp == NULL) {
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }

    strncpy(temp, semiColonPos + 1, colonPos - semiColonPos - 1);
    temp[colonPos - semiColonPos - 1] = '\0'; // Null terminate

    // Tokenize by semicolon to extract parameters
    char* token = strtok(temp, ";");
    while (token != NULL) {
        while (*token == ' ') {  // Trim leading spaces
            token++;
        }

        char* equalSign = strchr(token, '=');
        if (equalSign == NULL) {
            free(temp);
            printf("Invalid parameter format!\n");
            return INV_PROP;
        }

        *equalSign = '\0'; // Split into name and value

        // Trim spaces
        while (*(equalSign + 1) == ' ') {
            equalSign++;
        }

        Parameter* newParameter = malloc(sizeof(Parameter));
        if (newParameter == NULL) {
            free(temp);
            printf("Memory allocation failed!\n");
            return OTHER_ERROR;
        }

        newParameter->name = malloc(strlen(token) + 1);
        newParameter->value = malloc(strlen(equalSign + 1) + 1);

        if (newParameter->name == NULL || newParameter->value == NULL) {
            free(newParameter->name);
            free(newParameter->value);
            free(newParameter);
            free(temp);
            printf("Memory allocation failed!\n");
            return OTHER_ERROR;
        }

        strcpy(newParameter->name, token);
        strcpy(newParameter->value, equalSign + 1);
        insertBack(parameters, newParameter);

        token = strtok(NULL, ";");
    }

    free(temp);
    return OK;
}

/* this function is used to build the optional properties list for the vCard object.
    it calls the parse group, parse property name, parse parameters and parse values functions
    to build the optional properties list which is a list containing multiple structs of type Property
*/

VCardErrorCode buildOptionalProperties (char* newLine, List* optionalProperties){

    char* groupName = NULL;
    char* propName = NULL;
    Property* newProperty = NULL;
    List* parameters;
    List* values;
    VCardErrorCode error;
    char* Str;

    //parse the group name
    error = parseGroupName(newLine, &groupName);
    if(error != OK){
        return error;
    }
    //parse the property name
    error = parsePropName(newLine, &propName);
    //printf("%s\n", propName);
    if(error != OK){
        free(groupName);
        Str = errorToString(error);
        printf("%s", Str);
        return error;
    }

    //allocate memory for parameter list 
    parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    if(parameters == NULL){
        free(groupName);
        return OTHER_ERROR;
    }

    //allocate memory for the values list
    values = initializeList(&valueToString, &deleteValue, &compareValues);
    if(values == NULL){
        free(groupName);
        freeList(parameters);
        return OTHER_ERROR;
    }


    //parse the parameters
    error = parseParameters(newLine, parameters);
    if(error != OK){
        free(groupName);
        free(propName);
        freeList(parameters);
        freeList(values);
        return error;
    }

    //parse the values
    error = parseValues(newLine, values);
    if(error != OK){
        free(groupName);
        free(propName);
        freeList(parameters);
        freeList(values);
        return error;
    }

    //allocate memory for the new property
    newProperty = malloc(sizeof(Property));
    if(newProperty == NULL){
        free(groupName);
        free(propName);
        freeList(parameters);
        freeList(values);
        return OTHER_ERROR;
    }


    //copy the group name into the new property
    newProperty->group = groupName;
    //copy the property name into the new property
    newProperty->name = propName;
    //copy the parameters list into the new property
    newProperty->parameters = parameters;
    //copy the values list into the new property
    newProperty->values = values;

    //add the new property to the optional properties list
    insertBack(optionalProperties, newProperty);

    //deleteProperty(newProperty);

    return OK;
}

VCardErrorCode extractFN (char* newLine, Card* obj){

    char* result = NULL;
    int indexColon = 0;
    char* fnValue = NULL;

    // check if the line is empty
    if(newLine == NULL){
        printf("Empty Line!");
        return INV_PROP;
    }

    // check if property name exists in line, it should be before the colon
    result = strchr(newLine, ':');
    if(result == NULL){
        printf("No property in line found!\n");
        return INV_PROP;
    }

    // get the index of the colon
    indexColon = result - newLine;

    // check if the property name is FN
    if(strncmp(newLine, "FN", 2) != 0){
        printf("No FN property found!\nThe thirdline has to be a FN property!\n");
        return INV_FILE;
    }

    // allocate memory for the property name
    fnValue = malloc(sizeof(char)*256);

    if(fnValue == NULL){
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }

    // copy the property value (everything after the :'') into the fn variable
    strcpy(fnValue, newLine + indexColon + 1);
    // remove the line break and add null terminator
    fnValue[strcspn(fnValue, "\r\n")] = '\0'; // Remove CRLF safely


    // allocate memory for the fn property
    obj->fn = malloc(sizeof(Property));
    if(obj->fn == NULL){
        free(fnValue);
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }

    // initialize the fn property
    //obj->fn->name = strdup("FN");
    //obj->fn->group = strdup("");
    obj->fn->name = malloc(strlen("FN") + 1);
    obj->fn->group = malloc(1); // Empty string, just '\0'

    if (!obj->fn->name || !obj->fn->group) {
        free(obj->fn->name);
        free(obj->fn->group);
        free(obj->fn);
        return OTHER_ERROR;
    }

    strcpy(obj->fn->name, "FN");
    obj->fn->group[0] = '\0'; // Empty string

    obj->fn->parameters = initializeList(&parameterToString, &deleteParameter, &compareParameters);
    obj->fn->values = initializeList(&valueToString, &deleteValue, &compareValues);

    // validate fn property
    if (obj->fn->parameters == NULL || obj->fn->values == NULL) {
        printf("Memory allocation for FN lists failed!\n");
        free(obj->fn->name);
        free(obj->fn->group);
        free(obj->fn);
        free(fnValue);
        return OTHER_ERROR;
    }

    insertBack(obj->fn->values, fnValue);


    return OK;
}

VCardErrorCode dateAndTime(char* dtValue, DateTime* dt) {
    if (dtValue == NULL || dt == NULL) {
        printf("Invalid input parameters\n");
        return INV_PROP;
    }

    // Initialize all members
    dt->UTC = false;
    dt->isText = false;
    dt->date = "";
    dt->time = "";
    dt->text = "";

    int dtLen = strlen(dtValue);
    if (dtLen == 0) {
        printf("Empty date value\n");
        return INV_PROP;
    }

    // Check for UTC flag (Z or z at the end)
    if (dtLen > 0 && (dtValue[dtLen - 1] == 'Z' || dtValue[dtLen - 1] == 'z')) {
        dt->UTC = true;
        dtValue[dtLen - 1] = '\0'; // Remove trailing 'Z'
    }

    if (dtValue[0] == 'T') {
        dt->time = malloc(strlen(dtValue)); // No need for `T`
        if (dt->time == NULL) {
            return OTHER_ERROR;
        }
        strcpy(dt->time, dtValue + 1); // Skip 'T'
        printf("Parsed Time Only: %s\n", dt->time); // Debugging output
        return OK;
    }
    // Check if the value is text (e.g., "today", "unknown")
    if (isalpha(dtValue[0])) {
        dt->isText = true;
        dt->text = malloc(strlen(dtValue) + 1);
        if (dt->text == NULL) {
            return OTHER_ERROR;
        }
        strcpy(dt->text, dtValue);
        return OK;
    }

    // Handle partial dates (e.g., --MMDD or --MM-DD)
    if (dtValue[0] == '-' && dtValue[1] == '-') {
        dt->date = malloc(strlen(dtValue) + 1);
        if (dt->date == NULL) {
            return OTHER_ERROR;
        }
        strcpy(dt->date, dtValue);
        return OK;
    }

    // Handle full date-time (e.g., YYYYMMDDTHHMMSS)
    char* timeSeparator = strchr(dtValue, 'T');
    if (timeSeparator == NULL) {
        // No 'T', so it's a date-only value
        dt->date = malloc(strlen(dtValue) + 1);
        if (dt->date == NULL) {
            return OTHER_ERROR;
        }
        strcpy(dt->date, dtValue);
        return OK;
    }

    // Handle date-time values
    int datePartLength = timeSeparator - dtValue;

    // Allocate memory for date
    dt->date = malloc(datePartLength + 1);
    if (dt->date == NULL) {
        return OTHER_ERROR;
    }
    strncpy(dt->date, dtValue, datePartLength);
    dt->date[datePartLength] = '\0';
    printf("%s", dt->date);
    // Allocate memory for time
    dt->time = malloc(strlen(timeSeparator + 1) + 1);
    if (dt->time == NULL) {
        free(dt->date);
        return OTHER_ERROR;
    }
    strcpy(dt->time, timeSeparator + 1);

    return OK;
}

