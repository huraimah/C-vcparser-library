/*
Name : Huraimah Fatima
Email : huraimah@uoguelph.ca
*/

#include "VCValidate.h"

VCardErrorCode iterateAndValidate (char* fileString, int size, char** newLine, int* iterator){

    //initialize variables
    char* copyLine = NULL; //variable to store the temporary line to trim
    int startIndex = *iterator;
    int endIndex = startIndex;
    int lineFound = 0;
    int memory = 256;
    char* tempMem = NULL;


    //validate length of the file
    if(size < MIN_FILE_SIZE){
        printf("File is too short!\n");
        return INV_FILE;
    }

    //iterate through each file contents
    if(*iterator > strlen(fileString)){
        return OK;
    }

    // allocate memory to new line
    *newLine = malloc(sizeof(char)*256); //allocate memory for the new line
    if(*newLine == NULL){
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }
    strcpy(*newLine, "");
    copyLine = malloc(sizeof(char)*256); //allocate memory for the new line
    if(copyLine == NULL){
        free(*newLine);
        printf("Memory allocation failed!\n");
        return OTHER_ERROR;
    }
    if(*iterator ==0){
        endIndex++;
    }
    while(lineFound != 1 && *iterator < size){
        //check if no line break is found
        if(fileString[endIndex -1 ] == '\r' && fileString[endIndex] != '\n'){
            free(*newLine);
            printf("Invalid file format!\nNo line break found!\n");
            return INV_FILE;
        }
        if(fileString[endIndex] == '\n' && fileString[endIndex -1] != '\r'){
            free(*newLine);
            printf("Invalid file format!\nNo carriage return found!\n");
            return INV_FILE;
        }
        if(fileString[endIndex] == '\n' && fileString[endIndex -1] == '\r'){
            
            // check for line folding
            if(fileString[endIndex + 1] == ' ' || fileString[endIndex + 1] == '\t'){

                //copy the line into the new line
                if (strlen(*newLine) + (endIndex - startIndex + 1) > memory) {  // Ensure we don't overflow
                    tempMem = realloc(*newLine, (strlen(*newLine)) + (endIndex - startIndex -1 + 2));
                    if (tempMem == NULL) {
                    printf("Memory reallocation failed!\n");
                    return OTHER_ERROR;
                    }
                    *newLine = tempMem;
                }
                strncpy(*newLine,fileString + startIndex, endIndex - startIndex - 1);

                startIndex = endIndex + 2;
                endIndex = startIndex;
                while (endIndex < size && !(fileString[endIndex] == '\n' && fileString[endIndex - 1] == '\r')) {
                    endIndex++;
                }

                 int nextLineLength = endIndex - startIndex - 1;
                if (nextLineLength > 0) {
                    strncpy(copyLine, fileString + startIndex, nextLineLength);
                    copyLine[nextLineLength] = '\0'; // Null-terminate copyLine
                }

                //concatenate the line to the new line
                if(strlen(copyLine)+strlen(*newLine)>250){
                    memory += strlen(copyLine) + strlen(*newLine) +1;
                    tempMem = realloc(*newLine, memory);
                    if(tempMem == NULL){
                        free(*newLine);
                        printf("Memory allocation failed!\n");
                        return OTHER_ERROR;
                    }
                    *newLine = tempMem;
                    memory += 200;
                }

                //concatenate the folded line to the new line
                strcat(*newLine, copyLine);

                //update the iterator
                *iterator = endIndex + 1;
                lineFound = 1;
                free(copyLine);
                copyLine = malloc(sizeof(char)*256); //allocate memory for the new line
                if(copyLine == NULL){
                    free(*newLine);
                    printf("Memory allocation failed!\n");
                    return OTHER_ERROR;
                }
            }
            else{
                //copy the line into the new line
                strncpy(copyLine, fileString + startIndex, endIndex - startIndex +1);
                //remove the line folding
                copyLine[endIndex - startIndex + 1] = '\0';
                //update the iterator
                *iterator = endIndex + 1;
                //concatenate the line to the new line
                strcat(*newLine, copyLine);
                lineFound = 1;
                free(copyLine);
                copyLine = NULL;
            }
            
        }
        endIndex++;

    }// while loop ends 

    if(endIndex == size && copyLine != NULL){
        //copy the line into the new line
        strncpy(copyLine, fileString + startIndex, endIndex - startIndex + 1);
        //remove the line folding
        copyLine[endIndex - startIndex + 1] = '\0';
        //update the iterator
        *iterator = endIndex + 1;
        lineFound = 1;
        //concatenate the line to the new line
        strcat(*newLine, copyLine);
        free(copyLine);
        copyLine = NULL;
    }

    return OK;

}
