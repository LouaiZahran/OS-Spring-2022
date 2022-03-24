#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "list.c"

#define MAX_COMMAND_SIZE 1000
#define MAX_PROCESSES_NUM 1000

int pids[MAX_PROCESSES_NUM];
bool reaped[MAX_PROCESSES_NUM];
int pidsCount = 0;
struct list* list;
FILE* logfile;

void write_to_log_file(int pid){
    fprintf(logfile, "Child process [%d] has terminated\n", pid);
    fflush(logfile);
}

void reap_child_zombie(){
    for(int i=0; i<pidsCount; i++){
        if(reaped[i])
            continue;
        if(waitpid(pids[i], NULL, WNOHANG) != 0)
            reaped[i] = true, write_to_log_file(pids[i]);
    }
}

void on_child_exit(int sig){
    reap_child_zombie();
}

void printWorkingDirectory(){
    char directory[1000];
    getcwd(directory, 1000);
    printf("> %s: ", directory);
}

void setup_environment(){
    list = createList();

    char* directory = (char*) malloc(1000 * sizeof(char));
    char* logfileDirectory = (char*) malloc(1010 * sizeof(char));
    getcwd(directory, 1000);
    chdir(directory);
    strcpy(logfileDirectory, directory);
    strcat(logfileDirectory, "/logs.txt");
    logfile = fopen(logfileDirectory, "w+");

    free(directory);
    free(logfileDirectory);
}

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

void tokenize(char* src, char** dst){
    if(!*src){
        *dst = NULL;
        return;
    }
    int length = getFirstWord(src, *dst);
    tokenize(src + length, dst + 1);
}

void replaceVariables(char* src){
    char* ret = (char*) malloc(MAX_COMMAND_SIZE);
    char* retStart = ret;
    char* srcStart = src;
    char* temp = (char*) malloc(100);
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

void execute_shell_builtin(char* firstWord, char* command){
    int length = (int)strlen(command);

    if(strcmp(firstWord, "cd") == 0){
        if(strlen(command) == 2)    //cd command without a directory
            return;
        char* directory = malloc(1000);
        *directory = '\0';
        strncpy(directory, command + 3, length - 3);    //Skip the cd and the space after it
        chdir(directory);
        free(directory);
    }else if(strcmp(firstWord, "echo") == 0){
        char** tokens = (char**) malloc(10 * sizeof(char*));
        for(int i=0; i<10; i++)
            tokens[i] = (char*) malloc(100 * sizeof(char)), tokens[i][0] = '\0';
        tokenize(command, tokens);
        for(int i=1; i<10 && tokens[i] && tokens[i][0]; i++)
            printf("%s ", tokens[i]);
        printf("\n");
        for(int i=0; i<10; i++)
            free(tokens[i]);
        free(tokens);
    }else if(strcmp(firstWord, "export") == 0){
        char* name = (char*) malloc(100);
        char* value = (char*) malloc(100);
        char* temp = (char*) malloc(100);
        *name = '\0';
        *value = '\0';
        sscanf(command, "export %[^=]%[^\"]\"%[^\"]\"", name, temp, value);
        getFirstWord(name, name);
        setenv(name, value, true);
        free(name);
        free(value);
        free(temp);
    }else{
        printf("Unsupported Command\n");
    }
}

void execute_command(char* firstWord, char* command, bool background){
    int pid = fork();
    if(pid == 0){   //Child
        char** tokens = (char**) malloc(10 * sizeof(char*));
        for(int i=0; i<10; i++)
            tokens[i] = malloc(100 * sizeof(char));
        tokenize(command, tokens);

        execvp(firstWord, tokens);
        printf("Unsupported Command\n");
        for(int i=0; i<10; i++)
            free(tokens[i]);
        free(tokens);
        exit(1);
    }else{  //Parent
        pids[pidsCount++] = pid;
        if(!background)
            waitpid(pid, NULL, 0), reaped[pidsCount - 1] = true;
    }
}

void shell(){
    while(true){
        bool bg = false;                                   //Assumption until proven otherwise
        char* command = malloc(MAX_COMMAND_SIZE);
        char* commandStart = command;

        printWorkingDirectory();

        fgets(command, 1000, stdin);
        command[strlen(command) - 1] = '\0';            //To ignore the newline character from fgets

        while(command[0] == ' ' || command[0] == '\t')     //To ignore leading spaces
            command++;

        if(strlen(command) != 0 && command[strlen(command) - 1] == '&'){
            command[strlen(command) - 1] = '\0';        //We don't need the '&' anymore
            bg = true;
            while(command[strlen(command) - 1] == ' ' || command[strlen(command) - 1] == '\t')
                command[strlen(command) - 1] = '\0';    //To ignore trailing spaces
        }

        if(strlen(command) == 0)                        //Nothing to execute
            continue;

        char firstWord[20];
        replaceVariables(command);
        getFirstWord(command, firstWord);
        if(strcmp(firstWord, "exit") == 0)
            exit(0);
        if(strcmp(firstWord, "cd") == 0
        || strcmp(firstWord, "echo") == 0
        || strcmp(firstWord, "export") == 0)
            execute_shell_builtin(firstWord, command);
        else
            execute_command(firstWord, command, bg);

        free(commandStart); //To free the whole allocated array from the memory, even with ignored spaces
    }
}

int main() {
    signal(SIGCHLD, &on_child_exit);
    setup_environment();
    shell();

    return 0;
}
