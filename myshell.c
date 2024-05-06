#include <stdio.h> // input output lib
#include <stdlib.h> // standard lib for exit()
#include <string.h> // string lib
#include <unistd.h> // fork() & execvp()
#include <sys/wait.h> // wait() & waitpid()
#include <ctype.h>

#define MAX_LEN 512 // for character input
#define MAX_ARGS 20 // amount of words per command input
#define DELIM ";\n" // anywhere in the code there's a DELIM, a ; will replace it
#define MAX_COMMANDS 5 // max amount of commands

int should_quit = 0; // quit flag

void execute_command(char *cmds) { 
	char *commands[MAX_COMMANDS];
	char *args[MAX_ARGS];
	pid_t pids[MAX_COMMANDS]; // array to store child PIDS
	int cmd_count = 0;
	
	// splitting up the commands by ; 
	char *cmd_token = strtok(cmds, ";");
	while (cmd_token != NULL && cmd_count <= MAX_COMMANDS) { 
		commands[cmd_count++] = cmd_token; // each token AKA command is placed in the arg array
		cmd_token = strtok(NULL, ";"); // continues where the strtok left off reading
	}
	
	// command count checker
	if (cmd_count > MAX_COMMANDS) {
		fprintf(stderr, "\nCommand limit exceeded. %d is the command cap.\n", MAX_COMMANDS);
		exit(EXIT_FAILURE);
	}
	
	// starting each command concurrently
	for (int j = 0; j < cmd_count; j ++) {
		// trimming excess whitespaces
		while (isspace((unsigned char)*commands[j])) commands[j] ++;
		
		char *token = strtok(commands[j], " ");
		int i = 0;
		
		// tokenize the command into arguments
		while (token != NULL && i <= MAX_ARGS) {
			args[i++] = token;
			token = strtok(NULL, " ");
		}
		
		// args counter check
		if (i >= MAX_ARGS) {
			fprintf(stderr, "\nToo many arguments for a command, the argument limit is %d.\n", MAX_ARGS);
			exit(EXIT_FAILURE);
		}
		
		args[i] = NULL; // Null-terminate the arguments list i.e. list is done
	
		if (args[0] == NULL) {
			// an empty command was entered
			pids[j] = -1; // mark as no process to wait for 
			continue;
		}	
		
		// quit command checker
		if (strcmp(args[0], "quit") == 0) {
			should_quit = 1; // quit will execute last
			pids[j] = -1; // no process to wait for
			continue;
		}
		
		pid_t pid = fork(); 
		if (pid == -1) {
			perror("fork failed");
			exit(EXIT_FAILURE);
		} 
		else if (pid == 0) {
			// child process 
			if (execvp(args[0], args) == -1) { // execvp replaces the current process image with a new process (args)
				fprintf(stderr, "Command not found or cannot be executed: %s\n", args[0]);										
				_exit(EXIT_FAILURE);
			}
	 	}
	 	else {
	 		// storing the PID of the child process
	 		pids[j] = pid;
	 	}
	}
	
	// waiting for all child processes to finish
	for (int j = 0; j < cmd_count; j ++) {
		if (pids[j] > 0) {
			waitpid(pids[j], NULL, 0);
		}
	}
}

int main() {

	printf("Welcome to Eric Castorina's Linux Shell:\n");
	
	char line[MAX_LEN + 2]; // + 2 for the newline & null terminator
	
	while (!should_quit) {
		printf("prompt> ");
		fflush(stdout);
		
		if (!fgets(line, sizeof(line), stdin)) {
			if (feof(stdin)) {
				// Ctr + D was punched
				printf("\n Leaving the Shell");
				exit(EXIT_SUCCESS);
			}
			else {
				perror("fgets error");
				exit(EXIT_FAILURE);
			}
		}
		
		if (strlen(line) > MAX_LEN) {
			fprintf(stderr, "Command line's too long. Input must be less than %d characters.\n", MAX_LEN);
			continue;
		}
		// Remove newline character at the end of input
		line[strcspn(line, "\n")] = 0;
		
		// execute commands from the input line 
		execute_command(line);	
	}
	printf("\n Leaving the Shell\n");
	return 0;
}
