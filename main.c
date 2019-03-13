// Aviv David 208732933
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#define MAX_JOBS_SIZE 128
#define MAX_INPUT_LENGTH 1024
#define MAX_ARGS_LENGTH 512

// info about a running command
typedef struct Command {
    char name[32];
    pid_t pid;
} Command;

/**
 * Array of all running commands info, and a index for its
 * current size(number of running commands)
 */
static Command Jobs[MAX_JOBS_SIZE];
unsigned int NumOfJobs = 0;

/**
 * Add the command info to the jobs array
 * @param argsSize size of args
 * @param args arguments array
 * @param pid pid of the process that runs the command
 */
void add_command(unsigned argsSize, char **args, pid_t pid) {
    Command c;
    c.pid = pid;
    // build the command name with its arguments:
    for (unsigned int j = 0; j < argsSize; ++j) {
        strcat(c.name, args[j]);
        if (j != argsSize - 1) strcat(c.name, " ");
    }
    // add process to array of Jobs
    Jobs[NumOfJobs] = c;
    ++NumOfJobs;
}

/**
 * Print all running commands info
 */
void print_running_commands() {
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        printf("%d %s\n", Jobs[i].pid, Jobs[i].name);
    }
}

/**
 * Remove command p info from jobs array
 * @param p running command
 */
void remove_command(Command p) {
    unsigned int location = 0;
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        if (p.pid == Jobs[i].pid && !strcmp(p.name, Jobs[i].name)) break;
        ++location;
    }
    for (; location < NumOfJobs; ++location)
        Jobs[location] = Jobs[location + 1];
    if (NumOfJobs)--NumOfJobs;
}
/**
 * Read user input
 * @return user input
 */
char* read_input(){
    printf(">");
    char input[MAX_INPUT_LENGTH] = {0};
    fgets(input, MAX_INPUT_LENGTH, stdin); // input
    char * line = input;
    return line;
}
/**
 * Split user input by spaces and create arguments array
 * @param input user input
 * @return array of arguments
 */
char** create_args(char *input){
    char *args[MAX_ARGS_LENGTH] ; // args array
    strtok(input, "\n"); // trim \n from input
    char *token = strtok(input, " ");
    unsigned int argsSize = 0; // number of arguments
    while (token && argsSize < MAX_ARGS_LENGTH) {
        args[argsSize] = token;
        token = strtok(NULL, " ");
        ++argsSize;
    }
    args[argsSize] = NULL;
    char** arr = args;
    return arr;
}
/**
 * @param args arguments array
 * @return number of arguments
 */
unsigned int number_of_args(char **args){
    unsigned int i = 0;
    while (args[i]) ++i;
    return i;
}
/**
 * Run the command
 * @param argsSize number of arguments
 * @param args argument array
 * @param shouldWait 1 if father should wait for the run to finish
 * @param pid pid of the process that runs the command
 */
void run_command(unsigned int argsSize, char** args, int shouldWait, pid_t pid){
    if (!strcmp(args[0], "jobs")) {
        print_running_commands();
        return;
    }
    char bin[32] = "/bin/";
    char *path = strcat(bin, args[0]);
    printf("%d\n", getpid()); // print PID of new process
    if (shouldWait)execv(path, args); // just execute, father will wait
    else {
        ++NumOfJobs;
        add_command(argsSize, args, pid);
        execv(path, args);
    }
}
//TODO : Fix Jobs array
int run_shell() {
    while (1) {
        char* input = read_input();
        char** args = create_args(input);
        unsigned int argsSize = number_of_args(args);
        int status;
        pid_t pid;
        // father will have to wait if there is no & in end of args
        int shouldWait = strcmp(args[argsSize - 1], "&");
        if (!shouldWait) { // to skip the last arg, which is &
            args[argsSize - 1] = NULL;
            --argsSize;
        }
        if ((pid = fork() == 0)) {
            run_command(argsSize, args, shouldWait, pid);
        } else {
            if (shouldWait) {
                wait(&status);
            } else {
                //print_running_commands();
            }
        }
    }
}

int main() {
    return run_shell();
}