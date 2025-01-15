#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "myheaders.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

int Concurrency = 1;
char * Server = "jobExecutorServer.txt";


int main(int argc, char **argv)
{

    pid_t server_pid;
    pid_t childpid;
	FILE *server_file;
	FILE *commander_file;
    int pid;


	server_file = fopen(Server, "r"); 
	if (server_file == NULL) {
		server_pid = fork();
		if (server_pid == -1) {
			perror("Fork failed");
			exit(1);
		}

		if (server_pid == 0) 
        { /*server : write pid to file*/
			server_file = fopen(Server, "w");
            
			fprintf(server_file, "%d\n", getpid());
			fclose(server_file);

            signal(SIGCHLD, JobDone);       //Install signal handler for when a process is finished
	        signal(SIGCONT, JobRequest);	//Install signal handler for when jobCommander sends request

            //Redirect standard output and standard error to /dev/null
            int devNull = open("/dev/null", O_WRONLY);
            if(devNull == -1){
                perror("Failed to open:");
                exit(1);
            }

            if(dup2(devNull, STDOUT_FILENO) == -1 || dup2(devNull, STDERR_FILENO) == -1){
                perror("Failed to redirect:");
                close(devNull);
                exit(1);
            }
            close(devNull);

			jobExecutorServer();
		}
		else 
        {
            pid = server_pid;
            if (mkfifo(serverfifo, 0666) == -1)                       //This means it is not created, so create it
            {
                if ( errno != EEXIST)
                {
                    printf("Could not create fifo file.\n");
                    perror("perror");
                    return 1;
                }
            }
            if (mkfifo(commanderfifo, 0666) == -1)                    //This means it is not created, so create it
            {
                if ( errno != EEXIST)
                {
                    printf("Could not create fifo file.\n");
                    perror("perror");
                    return 1;
                }
            }
		}
	}

    //Check the argyments that i got from the user to define the action
    //printf("Argument 1: %s\n", argv[1]);

    if (argc == 0)
	{
        printf("You have not entered any command.\n");
        return 0;        
    }
    // We have this form jobCommander ..
    else if (strcmp (argv[1], "issueJob") == 0)
    {
        if (argc == 2)
        {
            printf("Wrong input of argyments for intrunction issueJob\n.");
            return 1; 
        }
    }
    else if (strcmp (argv[1], "setConcurrency") == 0)
    {
        if (argc == 2)
        {
            printf("Wrong input of argyments for intrunction setConcurrency\n.");
            return 1; 
        }
        else if (argc > 3)
        {
            printf("Too many argyments for intrunction setConcurrency. We need only 2\n.");
            return 1; 
        }
        else    // Check the concurrency is valid
        {
            int con = atoi(argv[2]);
            if (con < 1)
            {
                printf("Not acceptable concurrency. Must be higher that 1\n.");
                return 1;
            }
        }
    }
    else if (strcmp (argv[1], "stop") == 0)
    {
        if (argc == 2)
        {
            printf("Wrong input of argyments for intrunction stop.\n");
            return 1; 
        }
        else if (argc > 3)
        {
            printf("Too many argyments for intrunction stop. We need only 2.\n");
            return 1; 
        }
    }
    else if (strcmp (argv[1], "poll") == 0)
    {
        if (argc == 2)
        {
            printf("Wrong input of argyments for intrunction poll.\n");
            return 1; 
        }
        else if (argc > 3)
		{
			printf("Too many argyments for intrunction poll. It must be running OR queued.\n");
            return 1;
		}
		else
		{
            if (((strcmp(argv[2], "running")) != 0) && ((strcmp(argv[2], "queued")) != 0))
			{
                printf("Wrong argyments for intrunction poll. It must be running OR queued.\n");
                return 1;
			}
        }
    }
    else if (strcmp (argv[1], "exit") == 0)
    {
        if (argc > 2)
        {
            printf("Wrong input of argyments for intrunction exit.\n");
            return 1; 
        }
    }
    else
    {
        printf("You have not entered any valid command (issueJob,setConcurrency, stop, poll, exit).\n");
        return 0;
    }

    // Here we have a valid form of our instruction so i call the commander.
	jobCommander(argc, argv, server_pid);
}
    

    