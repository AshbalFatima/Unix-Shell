//include and define

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXCOM 1000 //maximum number of letters to be supported
#define MAXLIST 100 //maximum number of commands to be supported

//Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

//Greeting the shell during startup
void init_shell()
{
	clear();
	printf("\n\n\n\n*********************************");
	printf("\n\n\n\t****MY SHELL****");
	printf("\n\n\t-USE AT YOUR OWN RISK-");
	printf("\n\n\n\n*********************************");
	char* username = getenv("USER");//get username of current user
	printf("\n\n\nUSER is: @%s", username);
	printf("\n");
	sleep(1);
	clear();
}

//Function to take input
int takeInput(char* str)
{
	char* buf;

	buf = readline("\n>>> "); //display the shell prompt and read input
	if (strlen(buf) !=0)
	{
		add_history(buf); // Add input to history
		strcpy(str, buf); // Copy the input to str
		return 0;

		
	}
	else
	{
		return 1; // Return 1 if no input is given
	}
}

// Function to print Current Directory
void printDir(void)
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd)); // Get the current working directory
	printf("\nDir: %s", cwd); // Print the current working directory
}

// Function where the system command is executed
void execArgs(char** parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1)
	{
		printf("\nFailed to fork a child.");
		return;
	}
	else if (pid == 0)
	{
		if (execvp(parsed[0], parsed) < 0) 
		{
			// Execute the command
			printf("\nCould not execute Command");
		}
		exit(0);
	}
	else
	{
		//waiting for child to terminate
		wait(NULL);
		return;
	}
}

// Function where the piped system commands are executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
	// 0 is read end, 1 is write end
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd)<0)
	{
		printf("\nPipe could not be initialized");
		return;
	}

	p1=fork();
	if(p1<0)
	{
		printf("\nCould not fork");
		return;
	}

	if (p1==0)
	{
		// Child 1 executing
		// It only needs to write at write end
		close(pipefd[0]);
		// Redirect stdout to pipe write end
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[1]);
		
		// Execute the first command
		if (execvp(parsed[0], parsed) < 0)
		{
			printf("\nCould not execute Command 1");
			exit(0);
		}
	}
	else
	{
		// Parent executing
		p2 = fork();

		if (p2<0)
		{
			printf("\nCould not fork");
			return;
		}

		if (p2 == 0)
		{
			// Child 2 executing
			// It only needs to read at the read end
			close(pipefd[1]);
			// Redirect sdtin to pipe read end
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);

			// Execute the second command
			if (execvp(parsedpipe[0], parsedpipe) < 0) 
			{
				printf("\nCould not execute command 2");
				exit(0);
			}				
		}
		else
		{
			// Parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
}

// HELP command BuiltIn
void openHelp()
{
	puts("\n***WELCOME TO MY SHELL HELP***"
		"\nList of commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>All other general commands available in UNIX shell"
		"\n>Pipe Handling"
		"\n>Improper Space Handling");

	return;
}
 
// Function to execute BuiltIn commands
int ownCmdHandler(char** parsed)
{
	int NoOfOwnCmds =4, i, switchOwnArg=0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* username;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "help";
	ListOfOwnCmds[3] = "hello";

	if (parsed[0] == NULL)
		return 0; // No command provided

	for (i=0;i<NoOfOwnCmds; i++)
	{
		if (parsed[0] !=NULL && strcmp(parsed[0], ListOfOwnCmds[i])==0)
		{
			switchOwnArg = i+1;
			break;
		}
	}

	switch (switchOwnArg)
	{
	case 1:
		printf("\nGoodbye\n");
		exit(0); // Exit the shell
	case 2:
		if(parsed[1] != NULL)
		{
			chdir(parsed[1]); // Change directory
		}
		else
		{
			printf("\ncd: missing arguments\n");
		}
		return 1;
	case 3:
		openHelp(); // Display help
		return 1;
	case 4:
		username = getenv("USER");
		printf("\nHello %s. \n Mind that this"
		" is not a place to play around."
		"\nUse help to know more\n", username);
		return 1;
	default:
		break;
	}

	return 0;
}

// Function for finding pipe
int parsePipe(char* str, char** strpiped)
{
	int i;
	for (i=0;i<2;i++)
	{
		strpiped[i] = strsep(&str, "|"); // Split the string by pipe
		if (strpiped[i] == NULL)
			break;
	}

	if (strpiped[1] == NULL)
		return 0; // Returns zero if no pipe is found
	else
	{
		return 1;
	}
}

// Function for parsing command words
void parseSpace(char* str, char** parsed)
{
	int i;
	char* token;

	while ((token = strsep(&str," ")) != NULL && i < MAXLIST -1)
	{
		if (strlen(token)>0) // Avoid adding empty strings
		{
			parsed[i] = token;
			i++;
		}
	}
	parsed[i] = NULL; // NULL terminate the array

	//for (i=0;i<MAXLIST; i++)
	//{
	//	parsed[i] = strsep(&str, " "); // Split the string by spaces

	//	if (parsed[i] == NULL)
	//		break;
	//	if (strlen(parsed[i])==0)
	//		i--;
	//}
}

int processString(char* str, char** parsed, char** parsedpipe)
{
	char* strpiped[2];
	int piped = 0;

	piped = parsePipe(str, strpiped); // Check if there's a pipe
					  
	if (piped) 
	{
		// Parse the first part of the pipe
		parseSpace(strpiped[0], parsed); 
		// Parse the second part of the pipe
		parseSpace(strpiped[1], parsedpipe);
	}
	else
	{
		// Parse the input string if no pipe
		parseSpace(str,parsed);
	}

	if (ownCmdHandler(parsed))
		return 0;
	else
		return 1 + piped;
		// Return 1 if simple command, 2 if piped command
}

int main (void)
{
	char inputString[MAXCOM];
        char* parsedArgs[MAXLIST]={0}; // Initialize all to NULL
	char* parsedArgsPiped [MAXLIST]; // Initialize all to NULL
	int execFlag = 0;

	init_shell(); // Initialize the shell

	
	while(1) 
	{
		// Print shell line
		printDir();

		// Take input
		if (takeInput(inputString))
			continue;
	
		// Ensure all elements are initialized to NULL
        	for (int i=0; i<MAXLIST; i++)
        	{	
               		parsedArgs[i] = NULL;
                	parsedArgsPiped[i] = NULL;
	        }
			
		// Process
		execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
		// execFlag return zero if there is no command
		// or it is a builtin command,
		// 1 if it is a simple command
		// 2 if it is including a pipe

		// Execute
		if (execFlag == 1)
			execArgs(parsedArgs);

		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
	}

	return 0;
}


