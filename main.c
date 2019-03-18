#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#define MAX_JOBS_SIZE 128
#define MAX_INPUT_LENGTH 1024
#define MAX_ARGS_LENGTH 512

// info about a running command
typedef struct Command {
    char name[MAX_INPUT_LENGTH];
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
    // add command info to array of Jobs
    Jobs[NumOfJobs] = c;
    ++NumOfJobs;
}

/**
 * Remove command with the given pid from jobs array
 * @param pid running process pid
 */
void remove_command(pid_t pid) {
    unsigned int location = 0;
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        if (pid == Jobs[i].pid) break;
        ++location;
    }
    for (; location < NumOfJobs; ++location)
        Jobs[location] = Jobs[location + 1];
    if (NumOfJobs)--NumOfJobs;
}

/**
 * Print all running commands info
 * @return 1, in order to tell the program to keep running
 */
int print_running_commands() {
    for (unsigned int i = 0; i < NumOfJobs; ++i) {
        printf("%d %s\n", Jobs[i].pid, Jobs[i].name);
    }
    return 1;
}

/**
 * check if the command is special(either cd, exit, or jobs)
 * @param args arguments
 * @return 2 if no command requires special treatment, else returns command required finish status
 */
int special_commands(char **args) {
    if (!strcmp(args[0], "jobs")) return print_running_commands();
    else if (!strcmp(args[0], "cd")) {
        if (chdir(args[1]) == -1)fprintf(stderr, "Error in system call\n");
        return 1;
    } else if (!strcmp(args[0], "exit")) {
        _exit(0);
    }
    return 2;
}

/**
 * execute the user's request
 * @param argsSize number of arguments
 * @param args arguments
 * @param shouldWait 1 if father needs to wait for forked child, else 0
 * @return > 0 if shell should continue running, else 0
 */
int execute(unsigned int argsSize, char **args, int shouldWait) {
    int returnVal;
    pid_t pid;
    // collect all PID's of dead childes and remove theme from Jobs array:
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) remove_command(pid);
    if ((returnVal = special_commands(args)) == 2) {
        if (!strcmp(args[0], "man")) shouldWait = 1;
        if ((pid = fork()) == -1) {
            fprintf(stderr, "Error in system call\n");
            return 1;
        }
        if (!pid) { // child will execute the command
            execvp(args[0], args);
            fprintf(stderr, "Error in system call\n");// gets here if error occured on execute
            return 1;
        }
        if (shouldWait) wait(NULL); // parent will wait
        else add_command(argsSize, args, pid); // parent will add the given command to Jobs and continue
    }
    return returnVal;
}

/**
 * Read user input
 * @return user input
 */
char *read_input() {
    printf(">");
    char input[MAX_INPUT_LENGTH] = {0};
    fgets(input, MAX_INPUT_LENGTH, stdin); // input
    char *line = input;
    return line;
}

/**
 * Split user input by spaces and create arguments array
 * @param input user input
 * @return array of arguments
 */
char **create_args(char *input) {
    char *args[MAX_ARGS_LENGTH]; // args array
    strtok(input, "\n"); // trim \n from input
    char *token = strtok(input, " ");
    unsigned int argsSize = 0; // number of arguments
    while (token && argsSize < MAX_ARGS_LENGTH) {
        args[argsSize] = token;
        token = strtok(NULL, " ");
        ++argsSize;
    } // ^ inside loop : continue splitting by space, and add arguments to args array
    args[argsSize] = NULL;
    char **arr = args;
    return arr;
}

/**
 * @param args arguments array
 * @return number of arguments
 */
unsigned int number_of_args(char **args) {
    unsigned int i = 0;
    while (args[i]) ++i;
    return i;
}

int run_shell() {
    int status = 1;
    while (status) {
        char *input = read_input();
        char **args = create_args(input);
        if (**args == '\n') continue; // empty input
        unsigned int argsSize = number_of_args(args);
        // father will have to wait if there is no & in end of args
        int shouldWait = (*args[argsSize - 1] != '&');
        if (!shouldWait) { // to remove the last arg, which is &
            args[argsSize - 1] = NULL;
            --argsSize;
        }
        status = execute(argsSize, args, shouldWait);
    }
    return 0;
}

int main() {
    return run_shell();
}