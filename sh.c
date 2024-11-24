#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#define clear() printf("\033[H\033[J")

void init() { printf("\n\nUse at own risk\n\n"); }

void getPrompt(char *prompt) {
    char *user = getenv("USER");
    char host[1024];
    char cwd[1024];
    char *at = "@";
    char *suffix = "$";
    char *space = " ";
    memset(prompt, 0, sizeof(*prompt));

    getcwd(cwd, sizeof(cwd));
    gethostname(host, sizeof(host));
    printf("%s", host);
    strcat(prompt, user);
    strcat(prompt, at);
    strcat(prompt, host);
    strcat(prompt, space);
    strcat(prompt, cwd);
    strcat(prompt, space);
    strcat(prompt, suffix);
}

int getInput(char *str, char *prompt) {
    char *buf;
    buf = readline(prompt);
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

int parseStr(char *input, char args[20][50], int *argCount) {
    int len = strlen(input);
    char* builtins[] = {"ls", "mkdir"};
    for (int i = 0; i < len; i++) {
        if (input[i] == ' ') {
            if (strlen(args[*argCount]) != 0) {
                (*argCount)++;
            }
            args[*argCount][0] = ' ';
            args[*argCount][1] = '\0';
            (*argCount)++;
        } else if (input[i] == '|') {
            if (strlen(args[*argCount]) != 0) {
                (*argCount)++;
            }
            args[*argCount][0] = '|';
            args[*argCount][1] = '\0';
            (*argCount)++;
        } else {
            int arglen = strlen(args[*argCount]);
            args[*argCount][arglen] = input[i];
            args[*argCount][arglen + 1] = '\0';
        }
    }
    if (strcmp(args[0], "!!") == 0) {
        return 0;
    }
    if (strcmp(args[*argCount], "ECHO") == 0) {
        return 1;
    }
    if (strcmp(args[0], "help") == 0) {
        return 2;
    }
    for (int i = 0; i < 2; i++) {
        if (strcmp(args[0], builtins[i]) == 0) {
            return 3;
        }
    }
    if (strcmp(args[0], "exit") == 0) {
        return 4;
    }
    if (strcmp(args[0], "cd") == 0) {
        return 5;
    }

    return -1;
}

void doEcho(char args[20][50], int *argCount) {
    for (int i = 0; i <= *argCount; i++) {
        if (strcmp(args[i], " ") == 0) {
            printf("SPACE\n");
        } else if (strcmp(args[i], "|") == 0) {
            printf("PIPE\n");
        } else {
            printf("%s\n", args[i]);
        }
    }
}

void help() {
    puts("\nShell Help"
         "\nList of Commands:"
         "\ncd"
         "\nls"
         "\nexit"
         "");
}

void cd(char** cmd, int argCount) {
    if (argCount < 2) {
        char* home = getenv("HOME");
        if (chdir(home) != 0) {
            perror("cd");
        } else if (chdir(cmd[1]) != 0) {
            perror("cd");
        }
    }
}

void execBuiltIn(char **cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("\nFiled forking child...");
        return;
    } else if (pid == 0) {
        if (execvp(cmd[0], cmd) < 0) {
            printf("\nCould not execute command...");
        }
    } else {
        wait(NULL);
        return;
    }
}

void execFlag(int flag, char args[20][50], int *argCount) {
    if (flag == 1) {
        doEcho(args, argCount);
    } else if (flag == 2) {
        help();
    } else if (flag == 3) {
        char* cmd[20];
        for (int i = 0; i <= *argCount; i++) {
            cmd[i] = args[i];
        }
        cmd[*argCount + 1] = NULL;
        execBuiltIn(cmd);
    } else if (flag == 4) {
        exit(0);
    } else if (flag == 5) {
        char* cmd[*argCount];
        for (int i = 0; i <= *argCount; i++) {
            cmd[i] = args[i];
        }
        cd(cmd, *argCount);
    }
}

int main() {
    char input[1000];
    char args[20][50] = {0};
    int argCount = 0;
    char *pipedArgs[10];
    char *prompt;


    init();

    while (1) {
        getPrompt(prompt);
        if (getInput(input, prompt) == 1) {
            continue;
        }
        int flag = parseStr(input, args, &argCount);
        if (flag == 0) {
            strcpy(input, previous_history()->line);
            flag = parseStr(input, args, &argCount);
            execFlag(flag, args, &argCount);
            memset(args, 0, sizeof(args));
            argCount = 0;
            continue;
        }
        execFlag(flag, args, &argCount);
        memset(args, 0, sizeof(args));
        argCount = 0;
    }
}
