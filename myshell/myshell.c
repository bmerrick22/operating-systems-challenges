#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>


/* PRINT DIRECTORY */
void printDir(){
    char cwd[BUFSIZ];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        printf("myshell: Unable to read current directory: %s\n", strerror(errno));
        return;
    }
    printf("%s\n", cwd); 
}


/* CHANGE DIRECTORY */
void changeDir(char *newDir){
    if(chdir(newDir) < 0){
        printf("myshell: Unable to change directory: %s: %s\n", newDir, strerror(errno));
        return;
    }
}


/* START */
void start(char *command[], int commandCount){
    char *final[commandCount-1];
    for(int i = 0; i < commandCount-1; i++)
        final[i] = command[i+1];

    int status = EXIT_SUCCESS;

    pid_t pid = fork();
    if(pid < 0){
        printf("Unable to fork: %s\n", strerror(errno));
        return;

    }else if(pid == 0){
        printf("myshell: procces %i started\n", getpid());
        if(execvp(final[0], final) < 0){
            printf("Unable to start program: %s: %s\n", command[1], strerror(errno));
            return;
        }
    }
}


/* WAIT */
void waitFunc(){
    int status;
    pid_t pid = wait(&status);
    if(pid < 0){
        printf("myshell: No children\n");
        return;
    }

    if(WIFEXITED(status)){
        int s = WEXITSTATUS(status);
        printf("myshell: process %i exited normally with status%i\n",pid,s);
    }
    if(WIFSIGNALED(status)){
        int s = WTERMSIG(status);
        printf("myshell: process %i exited abnormall with status %s: %s\n", pid, s, strerror(errno));
    }
}


/* WAITFOR */
void waitfor(char *PID){
    pid_t pid = atoi(PID);
    int status;

    pid_t cpid = waitpid(pid, &status, 0);

    if(cpid < 0){
        printf("myshell: No children\n");
        return;
    }

    if(WIFEXITED(status)){
        printf("myshell: process %d exited normally with status %d\n", cpid, WEXITSTATUS(status));
        return;
    } else if(WIFSIGNALED(status)){
        printf("myshell: process %i exited abnormally with signal %d: %s\n", cpid, WTERMSIG(status), strerror(errno));
        return;

    }
}


/* RUN */
void run(char *command[], int commandCount){
    char *final[commandCount-1];
    for(int i = 0; i < commandCount-1; i++)
        final[i] = command[i+1];

    int status;

    pid_t pid = fork();
    if(pid < 0){
        printf("Unable to fork: %s\n", strerror(errno));
        return;

    }else if(pid == 0){
        if(execvp(final[0], final) < 0){
            printf("Unable to start program: %s: %s\n", command[1], strerror(errno));
            return;
        }
    }

    pid_t cpid = waitpid(pid, &status, 0);

    if(cpid < 0){
        printf("myshell: No children\n");
        return;
    }

    if(WIFEXITED(status)){
        printf("myshell: process %d exited normally with status %d\n", cpid, WEXITSTATUS(status));
        return;
    } else if(WIFSIGNALED(status)){
        printf("myshell: process %i exited abnormally with signal %d: %s\n", cpid, WTERMSIG(status), strerror(errno));
        return;

    }
}



/* Signal Handler */
void sigchld_handler(int sig){}


/* WATCHDOG */
void watchdog(char *time, char *command[], int commandCount){

    char *final[commandCount-2];
    for(int i = 0; i < commandCount-2; i++){
        final[i] = command[i+2];
    }

    int status;
    signal(SIGCHLD, sigchld_handler);
    pid_t pid = fork();

    if(pid < 0){
        printf("Unable to fork: %s\n", strerror(errno));
        return;
    }

    if(pid == 0){
        if(execvp(final[0], final) < 0){
            printf("Unable to start program: %s: %s\n", command[1], strerror(errno));
            return;
        }
    }
    else{
        int sleepTime = atoi(time);
        if(sleep(sleepTime) == 0){
            printf("myshell: process %i exceeded the time limit, killing it...\n", pid);

            if(kill(pid, SIGKILL) < 0){
                printf("myshell: Unable to kill process %i\n", pid);
                return;
            }
        }
        wait(&status);
    
        if(WIFEXITED(status)){
            int s = WEXITSTATUS(status);
            printf("myshell: process %i exited normally with status %i\n",pid, s);
            return;
        }
        if(WIFSIGNALED(status)){
            int n = WTERMSIG(status);
            printf("myshell: process %i exited abnormally with signal %i: %s\n", pid, n, strerror(errno)); 
            return;
        }
    }
}


/* PARSE ARGUMENTS */
void parseArgs(char *args[], int argCount){
    for(int i = 0; i < argCount; i++){
        if((strcmp(args[i], "pwd") == 0) && (argCount ==1)){
            printDir();
            break;
        }
        else if((strcmp(args[i], "chdir") == 0)){
            if(argCount != 2){
                printf("myshell: Incorrect number of arguments for chdir\n");
                break;
             }
            i++;
            changeDir(args[i]);
            break;
        }
        else if((strcmp(args[i], "start") == 0)){
            start(args, argCount);
            break;
        }
        else if((strcmp(args[i], "wait") == 0)){
            waitFunc(args);
            break;
        }
        else if((strcmp(args[i], "waitfor") == 0)){
            if(argCount != 2){
                printf("myshell: Incorrect number of arguments for waitfor\n");
                break;
            }
            i++;
            waitfor(args[i]);
            break;
        }
        else if((strcmp(args[i], "run") ==0)){
            run(args, argCount);
            break;
        }
        else if((strcmp(args[i], "watchdog") == 0)){
            i++;
            watchdog(args[i], args, argCount);
            break;
        }
        else if((strcmp(args[i], "exit") == 0))
            exit(EXIT_SUCCESS);
        else if((strcmp(args[i], "quit") == 0))
            exit(EXIT_SUCCESS);
        else{
            printf("Unknown command: %s\n", args[i]);
            return;
        }
    }
}


/* STORE ARGUMENTS */
void storeArgs(char *input){
    char *words[100];
    char *currArg = strtok(input, " \t\n");
    int i = 0;
    int nWords = 0;

    while(currArg != NULL && nWords != 100){
        words[i]  = currArg;
        nWords++;
        i++;

        currArg = strtok(0, " \t\n");
    }
    words[nWords] = 0;
    parseArgs(words, nWords);
}


/* MAIN DRIVER */
int main(int args, char *argv[]){
    if(args > 1){
        printf("myshell: Incorrect number of arguments\n");
        printf("Usage:  ./myshell\n");
        exit(EXIT_FAILURE);
    }

    char currentLine[1000];
    bool canRead = true;
    fprintf(stdout, "myshell> " );
    while(fgets(currentLine, BUFSIZ, stdin)){
        storeArgs(currentLine);
        fprintf(stdout, "myshell> ");
        fflush(stdout);
    }
    return 0;
}

