#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "util.c"

#define MAX_COMMAND_LENGTH 1000
#define MAX_DIRECTORY_LENGTH 1000
#define MAX_PROCESSES_NUM 1000
#define MAX_TOKEN_NUM 10
#define MAX_TOKEN_LENGTH 100

int pids[MAX_PROCESSES_NUM];
bool reaped[MAX_PROCESSES_NUM];
int pidsCount = 0;
struct list* list;
FILE* logfile;

//Updates log whenever a child terminates
void write_to_log_file(int pid){
    fprintf(logfile, "Child process [%d] has terminated\n", pid);
    fflush(logfile);
}

//Loops over all child processes to reap its zombies
void reap_child_zombie(){
    for(int i=0; i<pidsCount; i++){
        if(reaped[i])
            continue;
        if(waitpid(pids[i], NULL, WNOHANG) != 0)
            reaped[i] = true, write_to_log_file(pids[i]);
    }
}

//Reaps zombie processes
void on_child_exit(int sig){
    reap_child_zombie();
}

//Prints the working directory before each line so that the user knows in what directory he is in
void printWorkingDirectory(){
    char directory[MAX_DIRECTORY_LENGTH];
    getcwd(directory, MAX_DIRECTORY_LENGTH);
    printf("> %s: ", directory);
}

//Sets up the initial directory to run in and open the file stream for the logs
void setup_environment(){
    char* directory = (char*) malloc(MAX_DIRECTORY_LENGTH);
    char* logfileDirectory = (char*) malloc(1010 * sizeof(char));
    getcwd(directory, MAX_DIRECTORY_LENGTH);
    chdir(directory);
    strcpy(logfileDirectory, directory);
    strcat(logfileDirectory, "/logs.txt");
    logfile = fopen(logfileDirectory, "w+");

    free(directory);
    free(logfileDirectory);
}

//Changes directory to the specified directory in the command
void execute_cd(char* command){
    int length = (int)strlen(command);
    if(length <= 3)    //cd command without a directory
        return;
    char* directory = (char*) malloc(MAX_COMMAND_LENGTH);
    memset(directory, 0, MAX_COMMAND_LENGTH);
    strncpy(directory, command + 3, length - 3);    //Skip the cd and the space after it
    chdir(directory);
    free(directory);
}

//Prints the given string to the screen
void execute_echo(char* command){
    char** tokens = (char**) malloc(MAX_TOKEN_NUM * sizeof(char*));
    for(int i=0; i<10; i++)
        tokens[i] = (char*) malloc(MAX_TOKEN_LENGTH * sizeof(char));

    tokenize(command, tokens);
    for(int i=1; i<10 && tokens[i] && tokens[i][0]; i++)
        printf("%s ", tokens[i]);
    printf("\n");

    for(int i=0; i<10; i++)
        free(tokens[i]);
    free(tokens);
}

//Stores the (name, value) combination given in the command to the environment variables
void execute_export(char* command){
    char* name = (char*) malloc(MAX_TOKEN_LENGTH * sizeof(char));
    char* value = (char*) malloc(MAX_TOKEN_LENGTH * sizeof(char));
    char* temp = (char*) malloc(MAX_TOKEN_LENGTH * sizeof(char));   //used to store any additional spaces around '=' in export
    *name = '\0';
    *value = '\0';

    sscanf(command, "export %[^=]%[^\"]\"%[^\"]\"", name, temp, value);
    getFirstWord(name, name);
    setenv(name, value, true);

    free(name);
    free(value);
    free(temp);
}

//Executes the only 3 built-in commands (cd, echo, export)
void execute_shell_builtin(char* firstWord, char* command){

    if(strcmp(firstWord, "cd") == 0){
        execute_cd(command);
    }else if(strcmp(firstWord, "echo") == 0){
        execute_echo(command);
    }else if(strcmp(firstWord, "export") == 0){
        execute_export(command);
    }else{
        printf("Unsupported Command\n");
    }
}

//Executes the external commands (non built-in)
void execute_command(char* firstWord, char* command, bool background){
    int pid = fork();
    if(pid == 0){   //Child
        char** tokens = (char**) malloc(MAX_TOKEN_NUM * sizeof(char*));
        for(int i=0; i<10; i++)
            tokens[i] = malloc(MAX_TOKEN_LENGTH * sizeof(char));
        tokenize(command, tokens);

        execvp(firstWord, tokens);

        printf("Unsupported Command\n");        //if execvp is not successful
        for(int i=0; i<10; i++)
            free(tokens[i]);
        free(tokens);
        exit(1);
    }else{  //Parent
        pids[pidsCount++] = pid;
        if(!background)
            waitpid(pid, NULL, 0),      //So that the terminal "gets stuck" at foreground
            reaped[pidsCount - 1] = true;
    }
}

//Main shell method
void shell(){
    while(true){
        bool bg = false;                                   //Assumption until proven otherwise
        char* command = (char*) malloc(MAX_COMMAND_LENGTH);
        char* commandStart = command;

        printWorkingDirectory();

        fgets(command, MAX_COMMAND_LENGTH, stdin);
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

        char firstWord[MAX_TOKEN_LENGTH];
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

//Driver Code
int main() {
    signal(SIGCHLD, &on_child_exit);
    setup_environment();
    shell();

    return 0;
}
