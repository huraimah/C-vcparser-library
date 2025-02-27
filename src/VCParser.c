/*
Name : Huraimah Fatima
Email : huraimah@uoguelph.ca
*/

#include "LinkedListAPI.h"
#include "VCParser.h"
#include "VCParsingHelper.h"
#include "VCValidate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
    Card* object;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    VCardErrorCode error = createCard(argv[1], &object);
    if (error != OK) {
        fprintf(stderr, "Error: %s\n", errorToString(error));
        return EXIT_FAILURE;
    }

    printf("Ok!");

    // Cleanup memory
    //deleteCard(object);
    
    return 0;
}


// This function create alloctes soace for the VCard 
VCardErrorCode createCard(char* fileName, Card** obj){

    FILE* fp; //file pointer to read the file
    char* file = NULL; //variable to store the file contents
    char* line; //variable to store each line read from vcard
    int memoryAllocated = 300; //memory allocated for each line read from vcard
    int length; //variable to store length of each line read from vcard temporarily
    int fileSize = 0; // variable to store the size of the file after reading all the contents
    int fileIterator = 0; //variable to iterate through the file contents
    char* newLine = NULL; //variable to parse the file contents into lines
    char* birthday = NULL; //variable to store the group name
    char* anniversary = NULL; //variable to store the property name
    char* newMem; //variable to store the new memory allocated
    VCardErrorCode abort;
    


    //-------------------------------------------------------****validate file name****-------------------------------------------------------
    if(fileName == NULL){
        printf("file name not provided!");
        return INV_FILE;
    }
    //validate if file name provided has vcf or vcard at the end 
    if (strcmp(&(fileName[strlen(fileName)-3]), "vcf") !=0 && strcmp(&(fileName[strlen(fileName)-5]), "vcard") != 0){

        printf("Invalid file type!\n Please provide a file with a '.vcf' or '.vcard' extension");
        return INV_FILE;
    }
    
    //open file reader
    fp = fopen(fileName, "r+");

    if(fp == NULL){
        printf("File not found!");
        return INV_FILE;
    }
    //-------------------------------------------------------****validate file name end****-------------------------------------------------------

    //-------------------------------------------------------****initialize the card object****-------------------------------------------------------
    *obj = malloc(sizeof(Card));
    //
    if(*obj == NULL){
        printf("Memory allocation failed!");
        return OTHER_ERROR; 
    }
    (*obj)->fn = NULL;
    (*obj)->optionalProperties = initializeList(&propertyToString, &deleteProperty, &compareProperties);
    (*obj)->birthday = NULL;
    (*obj)->anniversary = NULL;

    // initialize date time property 
    //memset((*obj)->birthday, 0, sizeof(DateTime));
    //memset((*obj)->anniversary, 0, sizeof(DateTime));

    //optional properties cannot be NULL
    if((*obj)->optionalProperties == NULL){
        deleteCard(*obj); //deallocate memory for the card object 
        printf("Memory allocation failed!");
        return OTHER_ERROR; 
    }
    //-------------------------------------------------------****initialize the card object end****-------------------------------------------------------
    //---------------------------------------------------------*******file reader*********---------------------------------------------------------
    //read Vcard file contants line by line 
    line = malloc(sizeof(char)*memoryAllocated);
    //check if memory allocation was successful
    if(line == NULL){
        deleteCard(*obj); //deallocate memory for the card object
        *obj = NULL;
        printf("Memory allocation failed!");
        fclose(fp);
        return OTHER_ERROR; 
    }
    //acclocate memory for the array containing all the file contents
    file = malloc(sizeof(char)*memoryAllocated);
    //check if memory allocation was successful
    if(file == NULL){
        deleteCard(*obj); //deallocate memory for the card object
        free(line);
        fclose(fp);
        printf("Memory allocation failed!");
        return OTHER_ERROR; 
    }

    //read file contents line by line
    while(fgets(line, 256, fp) != NULL){

        length = strlen(line); //get the length of the line read

        //reallocate memory if the current line's length added to the current file size is less than original memory allocated.
        if((length + fileSize) >= memoryAllocated){
            memoryAllocated += 300;
            newMem = realloc(file, memoryAllocated);
            //check if memory allocation was successful
            if(newMem == NULL){
                free(line); //deallocate memory for the line
                free(file); //deallocate memory for the file contents
                deleteCard(*obj); //deallocate memory for the card object
                fclose(fp);
                printf("Memory allocation failed!");
                return OTHER_ERROR; 
            }
            file = newMem; //update the file contents to the new memory allocated
        }
       
        //concatenate the line read to the end of the file contents
        strcat(file, line);
        //update the file size after adding the new line
        fileSize += length;
        free(line); // free the old line 
        line = malloc(sizeof(char)*memoryAllocated); //allocate memory for the new line
        //check if memory allocation was successful
        if(line == NULL){
            deleteCard(*obj); //deallocate memory for the card object
            fclose(fp);
            printf("Memory allocation failed!");
            return OTHER_ERROR; 
        }

    }
    //free the memory allocated for the line after finishing the while loop 
    if(line != NULL){
        free(line);
    }

    //shrink fileSize to the actual size of the file contents
    newMem = realloc(file, (fileSize + 1));
    //check if memory allocation was successful
    if(newMem == NULL){
        free(file); //deallocate memory for the file contents
        deleteCard(*obj); //deallocate memory for the card object
        fclose(fp);
        printf("Memory allocation failed!");
        return OTHER_ERROR; 
    }
    file = newMem; //update the file contents to the new memory allocated   

    // print the file contents and fileSize for testing purposes
    printf("File Size: %d\n", fileSize);
    printf("File Contents: %s", file);

    //close the file reader
    //fclose(fp);
    //---------------------------------------------------------*******file reader end*********-----------------------------------------------------------------
    //---------------------------------------------------------*******split file contents into lines*********---------------------------------------------------------
    // split the file contents into lines

    //---------------------------------------------------------*******parse the file contents*********---------------------------------------------------------
    //validate the file Contents
    //1. make sure BEGIN is the first line
    if (iterateAndValidate(file, fileSize, &newLine, &fileIterator) != OK) {
        deleteCard(*obj);
        return INV_CARD;
    }

    if(strcmp(newLine, "BEGIN:VCARD\r\n") != 0){
        deleteCard(*obj); //deallocate memory for the card object
        printf("Invalid file format!\nBEGIN:VCARD not found!\n");
        return INV_CARD;
    }

    //2. make sure VERSION is the second line
    iterateAndValidate(file, fileSize, &newLine, &fileIterator);
    if(strcmp(newLine, "VERSION:4.0\r\n") != 0){
        deleteCard(*obj); //deallocate memory for the card object
        printf("Invalid file format!\nVERSION:4.0 not found!\n");
        return INV_CARD;
    }

        // Allocate memory for tempLine
    char* tempLine = malloc(sizeof(char) * 20); // 11 characters + 1 for null terminator
    if (tempLine == NULL) {
        deleteCard(*obj); // Deallocate memory for the card object
        printf("Memory allocation failed!");
        return OTHER_ERROR;
    }

    // Copy the last 11 characters of the file contents to tempLine
    strncpy(tempLine, file + fileSize - 11, 11);
    tempLine[11] = '\0'; // Null-terminate the string

    // Debug: Print the last 20 characters of the file
    printf("Last 20 characters of the file:\n");
    for (int i = fileSize - 20; i < fileSize; i++) {
        printf("%c", file[i]);
    }
    printf("\n");

    // Validate the last line
    if (strcmp(tempLine, "END:VCARD\r\n") != 0) {
        deleteCard(*obj); // Deallocate memory for the card object
        free(tempLine);
        printf("Invalid file format!\nEND:VCARD not found!\n");
        return INV_CARD;
    }

    // Free tempLine after validation
    free(tempLine);
    //3. makes sure the last line is END:VCARD
    /*char* tempLine = malloc(sizeof(char)*256);
    if(tempLine == NULL){
        deleteCard(*obj); //deallocate memory for the card object
        printf("Memory allocation failed!");
        return OTHER_ERROR; 
    }
    // copy the last line of the file contents to the tempLine
    strncpy(tempLine, file + fileSize - 11, 11);
    if(strcmp(tempLine, "END:VCARD\r\n") != 0){
        deleteCard(*obj); //deallocate memory for the card object
        free(tempLine);
        printf("Invalid file format!\nEND:VCARD not found!\n");
        return INV_CARD;
    }*/

    //free(tempLine);

    //4. check for fn property
    if(iterateAndValidate(file, fileSize, &newLine, &fileIterator) != OK){
        deleteCard(*obj); //deallocate memory for the card object
        printf("Invalid file format!\nNo FN property found!\n");
        return INV_CARD;
    }

    //extract the FN property and adding it's value to the card object's fn member.
    if(extractFN(newLine, *obj) != OK){
        deleteCard(*obj); //deallocate memory for the card object
        printf("Invalid file format!\nNo FN property found!\n");
        return INV_CARD;
    }


    while(newLine != NULL){
        //groupName = NULL;
        //propName = NULL;
        iterateAndValidate(file, fileSize, &newLine, &fileIterator);
        //printf("\n%s\n", newLine);
        if(strcmp(newLine, "END:VCARD\r\n") == 0){
            break;
        }
        if(strcmp(newLine, "BEGIN:VCARD\r\n") == 0){
            printf("invalid card!\nTwo begins found!\n");
            return INV_CARD;
        }
        if(strcmp(newLine, "VERSION:4.0\r\n") == 0){
            printf("invalid card!\nTwo versions found!\n");
            return INV_CARD;
        }

        abort = buildOptionalProperties(newLine, (*obj)->optionalProperties);
        if(abort != OK){
            deleteCard(*obj); //deallocate memory for the card object
            printf("\nInvalid file format!\nError building optional properties!\n");
            return abort;
        }


    }

    //print the optional properties for testing purposes
    ListIterator iter = createIterator((*obj)->optionalProperties);
    void* elem;
    while((elem = nextElement(&iter)) != NULL){
        Property* prop = (Property*)elem;
        //prop->parameters = initializeList(&parameterToString,&deleteParameter,&compareParameters);
        //prop->values = initializeList(&valueToString,&deleteValue,&compareValues);

        if (strcmp(prop->name, "BDAY") == 0) {
            char* bdayValue = (char*)prop->values->head->data;
        
            // Allocate only if BDAY exists
            if ((*obj)->birthday == NULL) {
                (*obj)->birthday = malloc(sizeof(DateTime));
                if ((*obj)->birthday == NULL) {
                    printf("Memory allocation failed!\n");
                    return OTHER_ERROR;
                }
            }
        
            // Parse date
            if (dateAndTime(bdayValue, (*obj)->birthday) != OK) {
                free((*obj)->birthday);
                (*obj)->birthday = NULL; // Keep it NULL if invalid
            }
            deleteDataFromList((*obj)->optionalProperties, prop);
            continue;
        }

        if (strcmp(prop->name, "ANNIVERSARY") == 0){
            char* anniversaryValue = (char*)prop->values->head->data;
            printf("Anniversary Raw Value: %s\n", anniversaryValue);  // Debugging output
        
            // Ensure anniversary is allocated
            if ((*obj)->anniversary == NULL) {
                (*obj)->anniversary = malloc(sizeof(DateTime));
                if ((*obj)->anniversary == NULL) {
                    printf("Memory allocation failed for anniversary!\n");
                    return OTHER_ERROR;
                }
            }
        
            VCardErrorCode dtError = dateAndTime(anniversaryValue, (*obj)->anniversary);
            if (dtError != OK) {
                printf("Error parsing anniversary date!\n");
            }
        
            deleteDataFromList((*obj)->optionalProperties, prop);
            continue;
        }
        

    }

    // print date and time 
    //birthday = dateToString((*obj)->birthday);
    //anniversary = dateToString((*obj)->anniversary);
   //printf("%s\n", birthday);
    //printf("%s\n", anniversary);
    if ((*obj)->birthday == NULL) {
        printf("Birthday is NULL\n");
    } else {
        printf("Birthday is NOT NULL\n");
    }    

    char* cardString = cardToString(*obj);
    printf("%s\n", cardString);

    //---------------------------------------------------------*******parse the file contents end*********---------------------------------------------------------
    //free(file);
    free(newLine);
    free(birthday);
    free(anniversary);
    free(newMem);
    //deleteCard(*obj);
    //free(cardString);
    return OK;
}

//This function deletes the whole card object
void deleteCard(Card* obj){
    if (obj == NULL) return;

    // Free FN property
    if (obj->fn != NULL) {
        if (obj->fn->name) free(obj->fn->name);
        if (obj->fn->group) free(obj->fn->group);
        freeList(obj->fn->parameters);
        freeList(obj->fn->values);
        free(obj->fn);
    }

    // Free optional properties
    if (obj->optionalProperties != NULL) {
        freeList(obj->optionalProperties);
    }

    // Free birthday
    if (obj->birthday != NULL) {
        free(obj->birthday->date);
        free(obj->birthday->time);
        free(obj->birthday->text);
        free(obj->birthday);
    }

    // Free anniversary
    if (obj->anniversary != NULL) {
        free(obj->anniversary->date);
        free(obj->anniversary->time);
        free(obj->anniversary->text);
        free(obj->anniversary);
    }

    free(obj); // Free main object
}


/*void deleteCard(Card* obj){

    if (obj == NULL){
        return;
    }


    if (obj->fn != NULL) {
        if (obj->fn->name) free(obj->fn->name);
        if (obj->fn->group) free(obj->fn->group);
        freeList(obj->fn->parameters);
        freeList(obj->fn->values);
        free(obj->fn);
    }
    if(obj->fn != NULL){
        deleteProperty(obj->fn);
        return;
    }

    if(obj->optionalProperties != NULL){
        freeList(obj->optionalProperties);
    }

    if(obj->birthday != NULL){
        deleteDate(obj->birthday);
    }

    if(obj->anniversary != NULL){
        deleteDate(obj->anniversary);
    }

    free(obj);

}*/


char* cardToString(const Card* obj) {
    if (obj == NULL) {
        return "Object is empty";
    }

    // Allocate a buffer large enough to hold the entire card string
    char* cardStr = malloc(4096); // Adjust size as needed
    if (cardStr == NULL) {
        return NULL;
    }

    // Initialize the buffer
    cardStr[0] = '\0';

    // Add the FN property
    if (obj->fn != NULL) {
        char* fnStr = propertyToString(obj->fn);
        strcat(cardStr, "FN: ");
        strcat(cardStr, fnStr ? fnStr : "N/A");
        strcat(cardStr, "\n");
        free(fnStr);
    } else {
        strcat(cardStr, "FN: N/A\n");
    }

    // Add the Birthday property
    if (obj->birthday != NULL) {
        char* bdayStr = dateToString(obj->birthday);
        strcat(cardStr, "Birthday: ");
        strcat(cardStr, bdayStr ? bdayStr : "N/A");
        strcat(cardStr, "\n");
        free(bdayStr);
    } else {
        strcat(cardStr, "Birthday: N/A\n");
    }

    // Add the Anniversary property
    if (obj->anniversary != NULL) {
        char* anniversaryStr = dateToString(obj->anniversary);
        strcat(cardStr, "Anniversary: ");
        strcat(cardStr, anniversaryStr ? anniversaryStr : "N/A");
        strcat(cardStr, "\n");
        free(anniversaryStr);
    } else {
        strcat(cardStr, "Anniversary: N/A\n");
    }

    // Add the Optional Properties
    if (obj->optionalProperties != NULL && obj->optionalProperties->head != NULL) {
        strcat(cardStr, "Optional Properties:\n");

        ListIterator iter = createIterator(obj->optionalProperties);
        void* elem;
        while ((elem = nextElement(&iter)) != NULL) {
            Property* prop = (Property*)elem;

            // Add the group name (if it exists)
            if (prop->group != NULL && strlen(prop->group) > 0) {
                strcat(cardStr, "  Group: ");
                strcat(cardStr, prop->group);
                strcat(cardStr, "\n");
            }

            // Add the property name
            strcat(cardStr, "  Name: ");
            strcat(cardStr, prop->name ? prop->name : "N/A");
            strcat(cardStr, "\n");

            // Add the parameters (if they exist)
            if (prop->parameters != NULL && prop->parameters->head != NULL) {
                strcat(cardStr, "    Parameters:\n");

                ListIterator paramIter = createIterator(prop->parameters);
                void* paramElem;
                while ((paramElem = nextElement(&paramIter)) != NULL) {
                    Parameter* param = (Parameter*)paramElem;
                    strcat(cardStr, "      ");
                    strcat(cardStr, param->name ? param->name : "N/A");
                    strcat(cardStr, " = ");
                    strcat(cardStr, param->value ? param->value : "N/A");
                    strcat(cardStr, "\n");
                }
            } else {
                strcat(cardStr, "    Parameters: N/A\n");
            }
            if(prop->parameters == NULL){
                printf("parameters are NULL");
            }
            // Add the values (if they exist)
            if (prop->values != NULL && prop->values->head != NULL) {
                strcat(cardStr, "    Values:\n");

                ListIterator valueIter = createIterator(prop->values);
                void* valueElem;
                while ((valueElem = nextElement(&valueIter)) != NULL) {
                    char* value = (char*)valueElem;
                    strcat(cardStr, "      ");
                    strcat(cardStr, value ? value : "N/A");
                    strcat(cardStr, "\n");
                }
            } else {
                strcat(cardStr, "    Values: N/A\n");
            }
        }
    } else {
        strcat(cardStr, "Optional Properties: N/A\n");
    }

    return cardStr;
}



char* errorToString(VCardErrorCode err){
    
    char *error;
    error = NULL;

    if (err == OK)
    {
    error = malloc(3);
    strcpy(error, "OK");
    }
    else if (err == INV_FILE)
    {
        error = malloc(9);
        strcpy(error, "INV_FILE");
    }
    else if (err == INV_CARD)
    {
        error = malloc(9);
        strcpy(error, "INV_CARD");
    }
    else if (err == INV_PROP)
    {
        error = malloc(9);
        strcpy(error, "INV_PROP");
    }
   
    else if (err == OTHER_ERROR)
    {
        error = malloc(12);
        strcpy(error, "OTHER_ERROR");
    }
    else if (err == INV_DT)
    {
        error = malloc(7);
        strcpy(error, "INV_DT");
    }
    else
    {
        error = malloc(19);
        strcpy(error, "Invalid error code");
    }

  return error;


}

// *************************************************************************

// ************* List helper functions - MUST be implemented *************** 

/*void deleteProperty(void* toBeDeleted){
    if(toBeDeleted == NULL){
        return;
    }

    Property* deleted = (Property*)toBeDeleted;

    // Free the group name
    if(deleted->group != NULL){
        free(deleted->group);
        deleted->group = NULL;  
    }

    // Free the property name
    if(deleted->name != NULL){
        free(deleted->name);
        deleted->name = NULL;  
    }

    // Free the parameters list
    if(deleted->parameters != NULL){
        freeList(deleted->parameters);
        deleted->parameters = NULL;  
    }

    // Free the values list 
    if(deleted->values != NULL){
        freeList(deleted->values);
        deleted->values = NULL;  
    }

    free(deleted);
    deleted = NULL;  
}*/
void deleteProperty(void* toBeDeleted)
{
  if (toBeDeleted == NULL)
    return;

  Property *deleteProp;
  deleteProp = (Property*)toBeDeleted;

  if (deleteProp != NULL)
  {
    if (deleteProp->name != NULL)
      free(deleteProp->name);

    if (deleteProp->group != NULL)
      free(deleteProp->group);

    if (deleteProp->parameters != NULL)
      freeList(deleteProp->parameters);

    if (deleteProp->values != NULL)
      freeList(deleteProp->values);

    free(deleteProp);
  }
}

/*
this function returns -1 if properties are not the same

*/

int compareProperties(const void* first,const void* second){

    if(first == NULL || second == NULL){
        return 0;
    }

    const Property* firstC = (const Property*)first;
    const Property* secondC = (const Property*)second;
    int result;
    char* firstStr = NULL;
    char* secondStr = NULL;

    firstStr = propertyToString((void*)firstC);
    secondStr = propertyToString((void*)secondC);

    if (firstStr == NULL || secondStr == NULL) {
        free(firstStr);
        free(secondStr);
        return -1;  // Indicate failure
    }   

    result = strcmp(first, second);

    free(firstStr);
    free(secondStr);

    return result;
}

char* propertyToString(void* prop) {
    if (prop == NULL) {
        return NULL;  // Return NULL if the input is invalid
    }

    Property* property = (Property*)prop;  // Cast the void pointer to Property*

    // Calculate the required length for the string
    int length = snprintf(NULL, 0,"Group: %s\n""Name: %s\n",property->group ? property->group : "N/A",property->name ? property->name : "N/A");
    // Add length for parameters
    if (property->parameters != NULL) {
        ListIterator iter = createIterator(property->parameters);
        void* elem;
        while ((elem = nextElement(&iter)) != NULL) {
            Parameter* param = (Parameter*)elem;
            length += snprintf(NULL, 0,"Parameter: %s = %s\n",param->name ? param->name : "N/A",param->value ? param->value : "N/A");
        }
    } else {
        length += snprintf(NULL, 0, "Parameters: N/A\n");
    }

    // Add length for values
    if (property->values != NULL) {
        ListIterator iter = createIterator(property->values);
        void* elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char* value = (char*)elem;
            length += snprintf(NULL, 0, "Value: %s\n", value ? value : "N/A");
        }
    } else {
        length += snprintf(NULL, 0, "Values: N/A\n");
    }

    // Allocate memory for the string
    char* result = malloc((length + 1) * sizeof(char));  // +1 for the null terminator
    if (result == NULL) {
        return NULL;  // Return NULL if memory allocation fails
    }

    // Format the string
    int offset = snprintf(result, length + 1,"Group: %s\n""Name: %s\n",property->group ? property->group : "N/A",property->name ? property->name : "N/A");

    // Append parameters
    if (property->parameters != NULL) {
        ListIterator iter = createIterator(property->parameters);
        void* elem;
        while ((elem = nextElement(&iter)) != NULL) {
            Parameter* param = (Parameter*)elem;
            offset += snprintf(result + offset, length + 1 - offset,"Parameter: %s = %s\n",param->name ? param->name : "N/A",param->value ? param->value : "N/A");
        }
    } else {
        offset += snprintf(result + offset, length + 1 - offset, "Parameters: N/A\n");
    }

    // Append values
    if (property->values != NULL) {
        ListIterator iter = createIterator(property->values);
        void* elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char* value = (char*)elem;
            offset += snprintf(result + offset, length + 1 - offset, "Value: %s\n", value ? value : "N/A");
        }
    } else {
        offset += snprintf(result + offset, length + 1 - offset, "Values: N/A\n");
    }

    return result;
}

void deleteParameter(void* toBeDeleted){

    if(toBeDeleted == NULL){
        return;
    }

    Parameter* deleted = (Parameter*)toBeDeleted;

    // Free the group name
    if(deleted->name != NULL){
        free(deleted->name);
    }

    // Free the property name
    if(deleted->value != NULL){
        free(deleted->value);
    }

    free(deleted);

}

char* parameterToString(void* param){
    if (param == NULL) {
        return NULL;  // Return NULL if the input is invalid
    }

    Parameter* parameter = (Parameter*)param;  

    // Calculate the required length for the string
    int length = snprintf(NULL, 0, "Parameter: %s = %s\n",parameter->name ? parameter->name : "N/A",parameter->value ? parameter->value : "N/A");

    // Allocate memory for the string
    char* result = malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        return NULL; 
    }

    // Format the string
    snprintf(result, length + 1, "Parameter: %s = %s\n",parameter->name ? parameter->name : "N/A",parameter->value ? parameter->value : "N/A");

    return result;
}

int compareParameters(const void* first,const void* second){

    if(first == NULL || second == NULL){
        return 0;
    }

    const Parameter* firstC = (const Parameter*)first;
    const Parameter* secondC = (const Parameter*)second;
    int result;
    char* firstStr = NULL;
    char* secondStr = NULL;

    firstStr = parameterToString((void*)firstC);
    secondStr = parameterToString((void*)secondC);

    result = strcmp(firstStr, secondStr);

    free(firstStr);
    free(secondStr);

    return result;

}




void deleteValue(void* toBeDeleted){

    if(toBeDeleted != NULL)
    free((char*)toBeDeleted);

}


int compareValues(const void* first,const void* second){

    char* firstStr;
    char* secondStr;

    firstStr = (char*)first;
    secondStr = (char*)second;

  return strcmp(firstStr, secondStr);

}


char* valueToString(void* val){

    char* valString;
    valString = malloc(sizeof(char)*strlen((char*)val));
    strcpy(valString, (char*)val);
    return valString;


}

void deleteDate(void* toBeDeleted){

    if(toBeDeleted == NULL){
        return;
    }

    DateTime* deleted = (DateTime*)toBeDeleted;
    if (deleted->date) free(deleted->date);
    if (deleted->time) free(deleted->time);
    if (deleted->text) free(deleted->text);
    free(deleted);

}

// to be fleshed out later :)
int compareDates(const void* first,const void* second){

    return 1;

}

/*char* dateToString(void* date){

    if (date == NULL) {
        return NULL;  // Return NULL if the input is invalid
    }

    DateTime* dt = (DateTime*)date;  // Cast the void pointer to DateTime*

    // Calculate the required length for the string
    int length = snprintf(NULL, 0, "Date: %s\nTime: %s\nText: %s\nUTC: %s\nisText: %s\n",
        dt->date ? dt->date : "N/A",
        dt->time ? dt->time : "N/A",
        dt->text ? dt->text : "N/A",
        dt->UTC ? "Yes" : "No",
        dt->isText ? "Yes" : "No");

    // Allocate memory for the string
    char* result = malloc((length + 1) * sizeof(char));  // +1 for the null terminator
    if (result == NULL) {
        return NULL;  // Return NULL if memory allocation fails
    }

    // Format the string
    snprintf(result, length + 1,"Date: %s\n""Time: %s\n""Text: %s\n""UTC: %s\n""isText: %s\n",dt->date ? dt->date : "N/A",
    dt->time ? dt->time : "N/A",dt->text ? dt->text : "N/A",dt->UTC ? "Yes" : "No",dt->isText ? "Yes" : "No");

    return result;
    
}*/

/*char* dateToString(void* date) {
    if (date == NULL) {
        return "NULL DateTime";  // Ensure non-NULL return
    }

    DateTime* dt = (DateTime*)date;

    const char* dateStr = dt->date ? dt->date : "N/A";
    const char* textStr = dt->text ? dt->text : "N/A";
    const char* timeStr = dt->time ? dt->time : "N/A";
    const char* utcStr = dt->UTC ? "Yes" : "No";
    const char* isTextStr = dt->isText ? "Yes" : "No";

    size_t bufferSize = 256;
    char* result = malloc(bufferSize);
    if (result == NULL) {
        return "Memory Allocation Failed";
    }


    strcpy(result, "Date: ");
    strcat(result, dateStr);
    strcat(result, "\nText: ");
    strcat(result, textStr);
    strcat(result, "\nTime: ");
    strcat(result, timeStr);
    strcat(result, "\nUTC: ");
    strcat(result, utcStr);
    strcat(result, "\nisText: ");
    strcat(result, isTextStr);
    strcat(result, "\n");

    return result;  // Caller must free() this
}*/
char* dateToString(void* date) {
    if (date == NULL) {
        char* nullMsg = malloc(15); // "NULL DateTime" + null terminator
        if (nullMsg != NULL) {
            strcpy(nullMsg, "NULL DateTime");
        }
        return nullMsg; 
    }

    DateTime* dt = (DateTime*)date;

    // Ensure dt members are properly initialized
    const char* dateStr = (dt->date && dt->date[0] != '\0') ? dt->date : "N/A";
    const char* textStr = (dt->text && dt->text[0] != '\0') ? dt->text : "N/A";
    const char* timeStr = (dt->time && dt->time[0] != '\0') ? dt->time : "N/A";
    const char* utcStr = dt->UTC ? "Yes" : "No";
    const char* isTextStr = dt->isText ? "Yes" : "No";

    // Precompute required buffer size
    size_t bufferSize = strlen(dateStr) + strlen(textStr) + strlen(timeStr) + 50;

    char* result = malloc(bufferSize);
    if (result == NULL) {
        char* errorMsg = malloc(26); // "Memory Allocation Failed" + null terminator
        if (errorMsg != NULL) {
            strcpy(errorMsg, "Memory Allocation Failed");
        }
        return errorMsg;
    }

    // Manually copy each component using `strcpy()` and `strcat()`
    result[0] = '\0';  // Start with an empty string

    strcpy(result, "Date: ");
    strcat(result, dateStr);
    strcat(result, "\nText: ");
    strcat(result, textStr);
    strcat(result, "\nTime: ");
    strcat(result, timeStr);
    strcat(result, "\nUTC: ");
    strcat(result, utcStr);
    strcat(result, "\nisText: ");
    strcat(result, isTextStr);
    strcat(result, "\n");

    return result;
}



