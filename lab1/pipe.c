#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	// Check if no command arguments are provided
	if (argc <= 1)
	{
		// Exit with an error status indicating invalid argument
		exit(EINVAL);
	}

	// If only one command argument is provided
	if (argc == 2)
	{
		// Create a child process
		pid_t cpid = fork();

		// If fork fails
		if (cpid < 0)
		{
			perror("Fork failed");
			exit(EXIT_FAILURE);
		}

		// If inside child process
		if (cpid == 0)
		{
			// Execute the command provided as argument
			if (execlp(argv[1], argv[1], NULL) < 0)
			{
				perror("Executing command failed");
				exit(EXIT_FAILURE);
			}
		}
		// If inside parent process
		else
		{
			int status = 0;
			// Wait for child process to terminate and get its status
			waitpid(cpid, &status, 0);
			// Exit with the child's exit status
			exit(WEXITSTATUS(status));
		}
	}
	// If more than one command argument is provided
	else
	{
		int fds[2];		  // File descriptors for the pipe
		int prevEnd = -1; // Previous pipe's read end

		// Loop through all provided commands
		for (int i = 1; i < argc; i++)
		{

			// Create a pipe
			if (pipe(fds) < 0)
			{
				perror("Pipe creation failed");
				exit(EXIT_FAILURE);
			}

			// Create a child process
			int pid = fork();

			if (pid < 0)
			{
				perror("Fork failed");
				exit(EXIT_FAILURE);
			}
			// If inside child process
			else if (pid == 0)
			{
				// If there was a previous command, set its output as the current command's input
				if (prevEnd != -1)
				{
					dup2(prevEnd, STDIN_FILENO);
					close(prevEnd);
				}

				// If not the last command, set the output to the write end of the pipe
				if (i < argc - 1)
				{
					dup2(fds[1], STDOUT_FILENO);
				}

				// Close unused pipe ends
				close(fds[0]);
				close(fds[1]);

				// Execute the current command
				execlp(argv[i], argv[i], NULL);
				exit(EXIT_FAILURE);
			}
			// If inside parent process
			else
			{
				close(fds[1]);	  // Close write end of the pipe
				prevEnd = fds[0]; // Store read end for next iteration

				int status = 0;
				// Wait for child process to terminate and get its status
				waitpid(pid, &status, 0);

				// If child did not terminate normally, exit with failure
				if (!WIFEXITED(status))
				{
					exit(EXIT_FAILURE);
				}

				// If child exited with a non-zero status, exit with the same status
				int exitStatus = WEXITSTATUS(status);
				if (exitStatus != 0)
				{
					exit(exitStatus);
				}
			}
		}
	}
	return 0;
}
