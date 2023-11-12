#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "token.h"
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>

void executeCommand(char** result);
void redirection(char** result);

int main(int argc, char **argv) {
  // Prints welcome message
  printf("Welcome to mini-shell.\n");

  char* user = getenv("USER");
  int sizeOfDir = strlen(getenv("HOME"));
  char *homeDir =(char*)malloc((sizeOfDir + 1) * sizeof(char));
  strcpy(homeDir, getenv("HOME"));
  if(homeDir != NULL) {
  } else {
     printf("HOME environment variable is not set.\n");
  }
  char previousCommand[255] = "";

  int run = 1;
  while (run == 1) {
        // prompt user
        printf("shell $ ");
        // shell $, tokenize input
        char userInput[255];
	char* line = fgets(userInput, sizeof(userInput), stdin);
	fflush(stdout);
	
	// user presses an empty line + enter
        if (line == NULL)  {
           printf("\nBye bye.\n");
           return 0;
        }

       	char** resultArray = tokenizeInput(userInput);
       
	// count pipes
	int pipeCount = 0;
	int i = 0;
	while(resultArray[i] != NULL) {
	   if (strcmp(resultArray[i], "|") == 0) {
	      pipeCount++;
	   }
	   i++;
	}

	// sequencing
	int semiIndex[255];
        int semiCount = -1;
        int f = 0;

        while(resultArray[f] != NULL) {
	   if(strcmp(resultArray[f], ";") == 0) {
 	      semiCount++;
              semiIndex[semiCount] = f;
           }
           f++;
        }
        char*** RESULT = (char***)malloc((semiCount + 2) * sizeof(char**));
        if(semiCount == -1) {
	   RESULT[0] = resultArray;
	}
       	else {
	   int lastIndex = 0;
	   for (int i = 0; i <= semiCount; i++) {
              int segmentSize = semiIndex[i] - lastIndex;
	      RESULT[i] = (char**)malloc((segmentSize + 1) * sizeof(char*));
	      for (int h = lastIndex, anotherCount = 0; h < semiIndex[i]; h++) {
                  RESULT[i][anotherCount] = resultArray[h];
                  anotherCount++;
	      }
	      RESULT[i][segmentSize] = NULL;
	      lastIndex = semiIndex[i] + 1;
	   }
	   // Handle the last segment
	   int lastSegmentSize = f - lastIndex;
	   RESULT[semiCount + 1] = (char**)malloc((lastSegmentSize + 1) * sizeof(char*));
	   for (int h = lastIndex, anotherCount = 0; h < f; h++) {
	       RESULT[semiCount + 1][anotherCount] = resultArray[h];
               anotherCount++;
	   }
	   RESULT[semiCount + 1][lastSegmentSize] = NULL;
	}

	for(int i = 0; i <= (semiCount + 1); i++) {
            char** result;
            result = RESULT[i];
	 if(result[0] == NULL) {

            } else {
            int count = 0;
            while(result[count] != NULL) {
                count = count + 1;
            }
            if (result[count - 1][strlen(result[count - 1]) - 1] == '\n') {
            // Replace the newline character with a null terminator
                result[count - 1][strlen(result[count - 1]) - 1] = '\0';
            }
	while(result[count] != NULL) {
		count++;
	}
	if (result[count - 1][strlen(result[count - 1]) - 1] == '\n') {
           // Replace the newline character with a null terminator
         result[count - 1][strlen(result[count - 1]) - 1] = '\0';
        }

	// exit conditions met --> exits the program
        if (strcmp(result[0], "exit") == 0) {
                printf("Bye bye.\n");
                run = 0;
                exit(1);
                return 0;
        }

	// cd command
	else if (strcmp(result[0], "cd") == 0) {
            if (result[1] == NULL) {
                chdir(homeDir);
            } else if (strcmp(result[1], "~") == 0) {
                chdir(homeDir);
            } else if (strcmp(result[1], "..") == 0) {
                chdir("..");
            } else {
                if (chdir(result[1]) != 0) {
                    perror("chdir");
                }
            }
            continue;
	}

	// prev: prints the previous command line and executes it again
	else if (strcmp(result[0], "prev") == 0) {
	   if (strlen(previousCommand) > 0) {
                printf("Previous command: %s\n", previousCommand);
                // Tokenize and execute the previous command
	       	char** args = tokenizeInput(previousCommand);
                redirection(args);     
	   }
	   else {
              printf("No previous command.\n");
           }
	}
		
	// source: executes a script, takes filename as arg and processes each line of file as a command
	else if (strcmp(result[0], "source") == 0) {
	   char command[255];
	   FILE* file = fopen(result[1], "r"); // open the file for reading
	   if (file == NULL) {
	      perror("Unable to open file");
	      exit(1);
	   }
	   else {
	      while (fgets(command, sizeof(command), file) != NULL) {
	          // Remove newline character from line
	          if (command[strlen(command) - 1] == '\n') {
	              command[strlen(command) -1] = '\0';
	          }
	          char** args = tokenizeInput(command);  
	          pid_t childPid = fork();
                  if (childPid == -1) {
                    perror("fork");
                    exit(1);
                  }
                  if (childPid == 0) {
                    // Child process
                    execvp(args[0], args);
                    perror("execvp"); // If execvp fails
                    exit(1);
                  } else {
                    // Parent process
                    wait(NULL);
                  }
		  freeTokens(args);
	      }
	      fclose(file); // close the file
	   }
	}
	
	// help: explains all the built-in commands available in shell
	else if (strcmp(result[0], "help") == 0) {
	   printf("Available commands:\n"
		   "\tcd: Changes the current working directory of the shell to the path specified as the argument\n"
		   "\tsource: Executes a script; takes a filename as an argument and processes each line of the file as a command\n"
		   "\tprev: Prints the previous command line and executes it again\n"
		   "\thelp: Explains all built-in commands available in the shell \n");
	}			    
       
        // handles piping	
	else if (pipeCount > 0) {
	   int pipeIndex = 0;
   	   int input_fd = 0;
   	   int prev_pipe_fds[2];
    	   while (pipeIndex <= pipeCount) {
              // Find the command and arguments before the pipe symbol
              int segmentSize = 0;
              while (result[input_fd + segmentSize] != NULL && strcmp(result[input_fd + segmentSize], "|") != 0) {
                 segmentSize++;
              }

              // Create pipe
              int pipe_fds[2];
              if (pipe(pipe_fds) == -1) {
                 perror("Error creating pipe");
                 exit(1);
              }
	      // Fork a child process to handle the command
       	      pid_t childPid = fork();
              if (childPid == -1) {
                  perror("fork");
                  exit(1);
              }

	      // Child process
             if (childPid == 0) {
                // Redirect input from the previous pipe or stdin
                if (input_fd > 0) {
                   dup2(prev_pipe_fds[0], 0);
                   close(prev_pipe_fds[0]);
		   close(prev_pipe_fds[1]);
                }
                // Redirect output to the pipe or stdout
                if (pipeIndex < pipeCount) {
                   dup2(pipe_fds[1], 1);
                   close(pipe_fds[0]);
		   close(pipe_fds[1]);
                }
                // Execute the command
                result[input_fd + segmentSize] = NULL;  // Null-terminate the argument list
                redirection(result + input_fd);
               // Error occurred if reaching this point
               exit(1);
            } 
	    // Parent Process
	    else {
	       // Close the previous pipe ends not used by this child
               if (input_fd > 0) {
                   close(prev_pipe_fds[0]);
                   close(prev_pipe_fds[1]);
               }
               // Save the current pipe ends for the next child
               prev_pipe_fds[0] = pipe_fds[0];
               prev_pipe_fds[1] = pipe_fds[1];
               // Move to the next command in the input array
               input_fd += segmentSize + 1;  // skip the pipe symbol
               pipeIndex++;
	    }
          }
          // Wait for all child processes to finish
          for (int i = 0; i <= pipeCount; i++) {
              wait(NULL);
          }
       }

	else {       
	   redirection(result);
	}

	// Update previous command
        strncpy(previousCommand, userInput, sizeof(previousCommand));
        previousCommand[sizeof(previousCommand) - 1] = '\0';
      }
     }
     // Free tokens
     for (int i = 0; i <= (semiCount + 1); i++) {
         freeTokens(RESULT[i]);
     }
     free(RESULT);
     } 
  free(homeDir);
  return 0;
} 

// handles executing a basic command
void executeCommand(char** result) {
    pid_t childPid = fork();
    if (childPid == -1) {
       perror("fork");
       exit(1);
    }
    // executes child process and searches for path
    if (childPid == 0) {
       if (execvp(result[0], result) == -1) {
          fprintf(stderr, "%s: command not found\n", result[0]);
          exit(1);
       }
    }
    else {
       wait(NULL);
    }
}


// handles redirection
void redirection(char** result) {
    int input_redirection_index = -1;
    int output_redirection_index = -1;

    // Check for input and output redirection
    for (int i = 0; result[i] != NULL; i++) {
        if (strcmp(result[i], "<") == 0) {
            // Input redirection detected
            input_redirection_index = i;
        } else if (strcmp(result[i], ">") == 0) {
            // Output redirection detected
            output_redirection_index = i;
        }
    }

    // Fork a child process
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }

    // Child process
    if (child_pid == 0) {
        // Process input redirection
        if (input_redirection_index > 0) {
            char* input_file = result[input_redirection_index + 1];
            int input_fd = open(input_file, O_RDONLY);
            if (input_fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(input_fd, 0);
            close(input_fd);
            result[input_redirection_index] = NULL;
        }

        // Process output redirection
        if (output_redirection_index > 0) {
            char* output_file = result[output_redirection_index + 1];
            int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("open");
                exit(1);
            }
            dup2(output_fd, 1);
            close(output_fd);
            result[output_redirection_index] = NULL;
        }

        // Execute the command
        execvp(result[0], result);
        perror("execvp");
        exit(1); // Exit child process if execvp fails
	exit(0);
    } else {
        // Parent process
        int status;
        waitpid(child_pid, &status, 0);
    }
}
