#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#define MAX_COMMAND_SIZE 1000
#define MAX_TOKEN_LENGTH 100

//Returns the number of characters processed
int getFirstWord(char* src, char* dst){
    char* srcStart = src;
    while(*src == ' ' || *src == '\t')
        src++;
    while(*src != ' ' && *src != '\t' && *src != '\0')
        *dst = *src, dst++, src++;
    *dst = '\0';
    return (int)(src - srcStart);
}

//Tokenizes the given command into an arrays of strings that were seperated by spaces or tabs
void tokenize(char* src, char** dst){
    if(!*src){
        *dst = NULL;
        return;
    }
    int length = getFirstWord(src, *dst);
    tokenize(src + length, dst + 1);
}

//Replaces every environment variable in the command with its value before tokenizing
void replaceVariables(char* src){
    char* ret = (char*) malloc(MAX_COMMAND_SIZE);
    char* retStart = ret;
    char* srcStart = src;
    char* temp = (char*) malloc(MAX_TOKEN_LENGTH);
    char* name;
    char* value;
    bool firstWordAppended = false;

    while(*src != '\0'){
        if(firstWordAppended)
            sprintf(ret, " "), ret += 1;
        else
            firstWordAppended = true;

        int len = getFirstWord(src, temp);
        src += len;
        if(temp[0] != '$') {
            sprintf(ret, "%s", temp);
            ret += strlen(temp);
            continue;
        }

        name = strtok(temp, "$");
        value = getenv(name);
        if(value)
            sprintf(ret, "%s", value), ret += strlen(value);
    }
    strcpy(srcStart, retStart);
    free(retStart);
    free(temp);
}