#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

int main()
{
    signal(SIGCHLD, SIG_IGN);
    while (1)
    {
        int andFlag = 0;
        int inputFlag = 0;
        int outputFlag = 0;
        int pipeFlag = 0;
        // Read command
        string cmd;
        char *arg;
        char *args[1000];
        char *args2[1000]; // for pipe
        char *inputFile;
        char *outputFile;
        cout << ">";
        getline(cin, cmd);

        // Parse string
        char *cmdline = new char[cmd.length() + 1];
        strcpy(cmdline, cmd.c_str());
        arg = strtok(cmdline, " ");
        int i = 0;
        while (arg != nullptr)
        {
            if (strcmp(arg, "&") == 0)
                andFlag = 1;
            else
            {
                if (pipeFlag == 1)
                    args2[i++] = arg;
                else if (inputFlag == 1)
                    inputFile = arg;
                else if (outputFlag == 1)
                    outputFile = arg;
                else
                {
                    if (strcmp(arg, "|") == 0)
                    {
                        pipeFlag = 1;
                        args[i] = NULL;
                        i = 0;
                    }
                    else if (strcmp(arg, "<") == 0)
                        inputFlag = 1;
                    else if (strcmp(arg, ">") == 0)
                        outputFlag = 1;
                    else
                        args[i++] = arg;
                }
            }
            arg = strtok(nullptr, " ");
        }
        if (pipeFlag == 0)
            args[i] = NULL;
        else
            args2[i] = NULL;

        // Execute
        pid_t pid, pid2;

        // conditon: <
        if (inputFlag == 1)
        {
            /* fork another process */
            pid = fork();
            if (pid < 0)
            { /*error occurred*/
                fprintf(stderr, "Fork Failed");
                exit(-1);
            }
            else if (pid == 0)
            { /* child process */
                if (andFlag == 0)
                {
                    int fd = open(inputFile, O_RDONLY);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    execvp(args[0], args);
                }
                else
                {
                    pid2 = fork(); // Double fork
                    if (pid2 < 0)
                    { /*error occurred*/
                        fprintf(stderr, "Fork Failed");
                        exit(-1);
                    }
                    else if (pid2 == 0)
                    {
                        int fd = open(inputFile, O_RDONLY);
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        execvp(args[0], args);
                    }
                    else
                        exit(0);
                }
            }
            else
            { /* parent process */
                /* parent will wait for the child to complete */
                wait(NULL);
            }
        }

        // conditon: >
        if (outputFlag == 1)
        {
            /* fork another process */
            pid = fork();
            if (pid < 0)
            { /*error occurred*/
                fprintf(stderr, "Fork Failed");
                exit(-1);
            }
            else if (pid == 0)
            { /* child process */
                if (andFlag == 0)
                {
                    int fd = open(outputFile, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execvp(args[0], args);
                }
                else
                {
                    pid2 = fork(); // Double fork
                    if (pid2 < 0)
                    { /*error occurred*/
                        fprintf(stderr, "Fork Failed");
                        exit(-1);
                    }
                    else if (pid2 == 0)
                    {
                        int fd = open(outputFile, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        execvp(args[0], args);
                    }
                    else
                        exit(0);
                }
            }
            else
            { /* parent process */
                /* parent will wait for the child to complete */
                wait(NULL);
            }
        }

        // condition: |
        if (pipeFlag == 1)
        {
            int p[2]; // p[0] = read side, p[1] = write side
            pipe(p);
            /* fork another process */
            pid = fork();
            if (pid < 0)
            { /*error occurred*/
                fprintf(stderr, "First Fork Failed");
                exit(-1);
            }
            else if (pid == 0)
            { /* 1st child process */
                close(p[0]);
                dup2(p[1], STDOUT_FILENO);
                close(p[1]);
                execvp(args[0], args);
            }
            pid2 = fork();
            if (pid2 < 0)
            { /*error occurred*/
                fprintf(stderr, "Second Fork Failed");
                exit(-1);
            }
            else if (pid2 == 0)
            { /* 2nd child process */
                close(p[1]);
                dup2(p[0], STDIN_FILENO);
                close(p[0]);
                execvp(args2[0], args2);
                exit(0);
            }
            int status, status2;
            close(p[0]);
            close(p[1]);
            waitpid(pid, &status, 0);
            waitpid(pid2, &status2, 0);
        }

        // Normal
        if (inputFlag == 0 && outputFlag == 0 && pipeFlag == 0)
        {
            /* fork another process */
            pid = fork();
            if (pid < 0)
            { /*error occurred*/
                fprintf(stderr, "Fork Failed");
                exit(-1);
            }
            else if (pid == 0)
            { /* child process */
                if (andFlag == 0)
                    execvp(args[0], args);
                else
                {
                    pid2 = fork(); // Double fork
                    if (pid2 < 0)
                    { /*error occurred*/
                        fprintf(stderr, "Fork Failed");
                        exit(-1);
                    }
                    else if (pid2 == 0)
                        execvp(args[0], args);
                    else
                        exit(0);
                }
            }
            else
            { /* parent process */
                /* parent will wait for the child to complete */
                wait(NULL);
            }
        }
    }
    return 0;
}
