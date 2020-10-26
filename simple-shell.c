/*
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


#define MAX_LINE		80 /* 80 chars per line, per command */

char *trim(char *value) /*trims the blank spaces from user input*/
{
	char *result = value;
	while(result[0] == ' ')
	{
		result++;
	}

	return result;
}

void tokenize(char *input, int capacity, char **args)
{
	int argc = 0;
	char *p;
	const char delim[2] = " ";

	p = strtok(input, delim);
	while( p != NULL)
	{
		if(argc == capacity)
		{
			printf("Too many arguments");
			exit(-1);
		}
		args[argc++] = p;
		p = strtok(NULL, delim);
	}

	args[argc] = NULL;
}
int main(void)
{
	char input[1024];
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    	int should_run = 1;
	int pid;
	int childpid;
	int argc = 0;
	char *p;
	int i = 0;
	int should_wait;
	char *lessthan;
	char *greathan;
	char *pipesearch;
	char *filename;
	char *pipeA;
	char *pipeB;
	int fd;
	int pipefd[2];
	char history[1024] = {0};
	int pipeFlag = 0;

    	while (should_run)
	{
		pipeFlag = 0;
		should_wait = 1;
        	printf("osh>");
		fgets(input, 1024, stdin); /* using fgets adds \n  when user hits enter */
        	input[strlen(input) - 1] = '\0'; /* removes \n from fgets user input */

		if (strcmp(input, "!!") != 0) /* only copy if input IS NOT !! */
		{
			strcpy(history, input); /* copies input into history*/
		}
		if (strcmp(input, "!!") == 0) /* if user input is !! */
		{
			strcpy(input, history); /* copies history into input */
			printf("%s\n", history);
		}
		if (history[0] == '\0')
		{
			printf("No commands in history.\n");
		}
		if (input[strlen(input) - 1] == '&') /* checks for & in user input */
		{
			input[strlen(input) - 1] = '\0'; /* removes & */
			should_wait = 0; /* a flag to tell parent to not wait */
		}

		lessthan = strchr(input, '<'); /* checks user input for < and > characters*/
		greathan = strchr(input, '>'); /* stores them in appropriate variable */
		pipesearch = strchr(input, '|');

		fflush(stdout);

		if(pipesearch != NULL)
		{
			pipesearch[0] = '\0'; /* removes | character from input */
			pipesearch++; /* moves pointer to next element */
			pipeA = trim (input); /* trims spaces and stores in new variable */
			pipeB = trim (pipesearch); /* trims spaces and stores in new variable */
			/* printf("%s\n", pipeA); */
			/* printf("%s\n", pipeB); */

			pipeFlag = 1;
		}

		pid = fork();

		if ( pid < 0 ) /* Creation of child unsuccessful */
		{
			printf("Creation failed.\n");
			continue;
		}

		if ( pid == 0 ) /* Returned to the newly created child process */
		{
			/* printf("Currently in child process.\n"); */
			if(pipeFlag == 1) /* checks for pipe flag */
			{
   				/*while(args[i] != NULL)
				{
					printf("%s\n", args[i++]);
				}
				*/
				pipe(pipefd);

				childpid = fork();
				if ( childpid < 0 )
				{
					printf("Creation pipe child failed.\n");
					continue;
				}
				if ( childpid == 0)
				{
					/* printf("Currently in pipe child process.\n"); */
					dup2(pipefd[0], 0);
					close(pipefd[1]);
					tokenize(pipeB, MAX_LINE/2 + 1, args);

					execvp(args[0], args);
				}
				if  ( childpid > 0 )
				{
					/* printf("Currently in parent pipe process. Pipe Child process ID: %d\n", childpid); */
					dup2(pipefd[1], 1);
					close(pipefd[0]);
					tokenize(pipeA, MAX_LINE/2 + 1, args);

					execvp(args[0], args);
				}
			}
			if(lessthan != NULL) /* checks if < was entered */
			{
				lessthan[0] = '\0'; /* removes < character from input */
				lessthan++; /* moves pointer to next element */
				filename = trim (lessthan); /* trims spaces and stores in new variable */
				/* printf("%s\n", filename); */
				fd = open(filename, O_RDONLY);
				if(dup2(fd, 0) == -1) /* verifies that there was no error */
					printf("failed\n");
				close(fd);
			}
			else if(greathan != NULL) /* checks if > was entered */
			{
				greathan[0] = '\0'; /* removes > character from input */
				greathan++; /* moves pointer to next element */
				filename = trim (greathan); /* trims spaces and stores in new variable*/
				/* printf("%s\n", filename); */
				fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
				dup2(fd, 1);
				close(fd);

			}

			if(pipeFlag == 0)
			{
				tokenize (input, MAX_LINE/2 + 1, args);
				execvp(args[0], args);
			}

			exit(0);
		}

		if ( pid > 0 ) /* Returned to parent or caller. */
		{
			/* printf("Currently in parent process. Child process ID: %d\n", pid); */
			if(should_wait == 1) /* checks if the wait flag triggered */
			{
				waitpid(pid, NULL, 0);
			}

		}
        	/**
          	 * After reading user input, the steps are:
         	 * (1) fork a child process
         	 * (2) the child process will invoke execvp()
         	 * (3) if command included &, parent will invoke wait()
         	 */
    	}
	return 0;
}
