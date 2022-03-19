#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

int pids[10000];
bool reaped[10000];
int pidsCount;

//function parent_main()
//  register_child_signal(on_child_exit())
//  setup_environment()
//  shell()
//



//function on_child_exit()
//  reap_child_zombie()
//  write_to_log_file("Child terminated")

void write_to_log_file(int pid){
    printf("Child process [%d] has terminated\n", pid);
    fflush(stdin);
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

//
//function setup_environment()
//  cd(Current_Working_Directory)
//

void printWorkingDirectory(){
    char directory[1000];
    getcwd(directory, 1000);
    printf("> %s: ", directory);
}

void setup_environment(){
    char directory[1000];
    getcwd(directory, 1000);
    chdir(directory);
}

//function shell()
//  do
//      parse_input(read_input())
//      evaluate_expression():
//      switch(input_type):
//          case shell_builtin:
//              execute_shell_builtin();
//          case executable_or_error:
//              execute_command():
//
//  while command_is_not_exit
//

//Returns the number of characters processed
int getFirstWord(char* src, char* dst){
    char* srcStart = src, *dstStart = dst;
    while(*src == ' '){
        src++;
        continue;
    }
    while(*src != ' ' && *src != '\0')
        *dst = *src, dst++, src++;
    *dst = '\0';
    return src - srcStart;
}

void tokenize(char* src, char** dst){
    if(!*src){
        *dst = NULL;
        return;
    }
    int length = getFirstWord(src, *dst);
    tokenize(src + length, dst+1);
}

//function execute_shell_builtin()
//  switch(command_type):
//      case cd:
//      case echo:
//      case export:
//

void execute_shell_builtin(char* firstWord, char* command){
    int length = (int)strlen(command);
    if(strcmp(firstWord, "cd") == 0){
        char* directory = malloc(1000);
        strncpy(directory, command + 3, length - 3);
        chdir(directory);
    }else if(strcmp(firstWord, "echo") == 0){
        printf("ECHO: %s", command + 4);
    }else{
        exit(1);
    }
}

//function execute_command()
//  child_id = fork()
//  if child:
//      execvp(command parsed)
//      print("Error)
//      exit()
//  else if parent and foreground:
//      waitpid(child)

void execute_command(char* firstWord, char* command, bool background){
    int pid = fork();
    if(pid == 0){   //Child
        char** tokens = (char**) malloc(10 * sizeof(char*));
        for(int i=0; i<10; i++)
            tokens[i] = malloc(100 * sizeof(char));
        tokenize(command, tokens);

        char *path = getenv("PATH");
        char pathenv[strlen(path) + sizeof("PATH=")];
        sprintf(pathenv, "PATH=%s", path);
        char *env[] = {pathenv, "x=-l", NULL};
        printf("%s\n", pathenv);
        execve("/bin/ls", tokens, env);
        printf("ERRORRR!!!\n");
        exit(2);
    }else{  //Parent
        pids[pidsCount++] = pid;
        if(!background)
            waitpid(pid, NULL, 0), reaped[pidsCount - 1] = true;
    }
}

void shell(){
    while(true){
        bool bg = false;
        char command[1000];
        printWorkingDirectory();
        fgets(command, 1000, stdin);
        command[strlen(command) - 1] = '\0'; //To ignore the newline character from fgets
        if(command[strlen(command) - 1] == '&'){
            command[strlen(command) - 1] = '\0';
            bg = true;
            while(command[strlen(command) - 1] == ' ')
                command[strlen(command) - 1] = '\0';
        }
        if(strlen(command) == 0)
            continue;
        char firstWord[20];
        getFirstWord(command, firstWord);
        if(strcmp(firstWord, "exit") == 0)
            exit(0);
        if(strcmp(firstWord, "cd") == 0
        || strcmp(firstWord, "echo") == 0
        || strcmp(firstWord, "export") == 0)
            execute_shell_builtin(firstWord, command);
        else
            execute_command(firstWord, command, bg);
    }
}

int main() {
    signal(SIGCHLD, &on_child_exit);
    setup_environment();
    shell();

//    char** buffer = (char**) malloc(10 * sizeof(char*));
//    for(int i=0; i<10; i++)
//        buffer[i] = malloc(100 * sizeof(char));
//    tokenize("HELLO, WORLD! I'm Louai", buffer);

    return 0;
}
