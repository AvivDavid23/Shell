// Aviv David 208732933
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define JOBS_SIZE 128
#define INPUT_LENGTH 1024
#define ARGS_LENGTH 512

// info about a running command
typedef struct Command {
    char name[32];
    pid_t pid;
} Command;

/**
 * Array of all running commands info, and a index for its
 * current size(number of running commands)
 */
static Command Jobs[JOBS_SIZE];
unsigned int NumOfJobs = 0;

/**
 * Add the command info to the jobs array
 * @param argsSize size of args
 * @param args arguments array
 * @param pid pid of the process that runs the command
 */
void addCommand(unsigned argsSize, char *args[argsSize], pid_t pid) {
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
void printRunningCommands() {
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        printf("%d %s\n", Jobs[i].pid, Jobs[i].name);
    }
}

/**
 * Remove command p info from jobs array
 * @param p running command
 */
void removeCommand(Command p) {
    unsigned int location = 0;
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        if (p.pid == Jobs[i].pid && !strcmp(p.name, Jobs[i].name)) break;
        ++location;
    }
    for (; location < NumOfJobs; ++location)
        Jobs[location] = Jobs[location + 1];
    if (NumOfJobs)--NumOfJobs;
}

int run_shell() {
    while (1) {
        printf(">");
        char input[INPUT_LENGTH] = {0};
        fgets(input, INPUT_LENGTH, stdin); // input
        char *args[ARGS_LENGTH] = {0}; // args array
        strtok(input, "\n"); // trim \n from input
        /**
         * split by space:
         */
        char *token = strtok(input, " ");
        unsigned int argsSize = 0; // number of arguments
        while (token && argsSize < ARGS_LENGTH) {
            args[argsSize] = token;
            token = strtok(NULL, " ");
            ++argsSize;
        }

        int status;
        pid_t pid;
        // father will have to wait if there is no & in end of args
        int shouldWait = strcmp(args[argsSize - 1], "&");
        if (!shouldWait) { // to skip the last arg, which is &
            args[argsSize - 1] = NULL;
            --argsSize;
        }
        if ((pid = fork() == 0)) {
            if (!strcmp(args[0], "jobs")) {
                printRunningCommands();
                continue;
            }
            char bin[32] = "/bin/";
            char *path = strcat(bin, args[0]);
            printf("%d\n", getpid()); // print PID of new process
            if (shouldWait)execv(path, args); // just execute, father will wait
            else {
                ++NumOfJobs;
                addCommand(argsSize, args, pid);
                execv(path, args);
            }
        } else {
            if (shouldWait) {
                wait(&status);
            } else {
                //printRunningCommands();
            }
        }
    }
}

int main() {
    return run_shell();
}