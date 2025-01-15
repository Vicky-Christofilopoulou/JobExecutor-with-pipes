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


//commanderfifo  	--> Commander writes - Server reads
//serverfifo 		--> Server writes - commander reads


void jobCommander(int argc, char **argv, int server_pid)
{
	int compd;
	int serpd;
    int i;
	char buffer[250];
	FILE *server_file;

	if( (compd=open(commanderfifo, O_WRONLY )) < 0) 	//Commander writes - Server reads
    {
		printf("Error opening the fifo.\n");
		perror ("open failed");
		exit(1);
	}

	if( (serpd=open(serverfifo, O_RDONLY)) < 0) 	//Server writes - commander reads
    {
		printf("Error opening the fifo.\n");
		perror ("open failed");
		exit(1);
	}


	//Step 1: Read the server pid
	server_file = fopen(Server, "r");
	if (server_file == NULL)
	{
		perror("Failed to open server file for reading");
		exit(EXIT_FAILURE);
	}

    char pid_str[MAXLEN];
    if (fgets(pid_str, MAXLEN, server_file) == NULL)
    {
        perror("Failed to read PID from server file");
        exit(EXIT_FAILURE);
    }
	
    // Convert the string PID to integer
    server_pid = atoi(pid_str);
    fclose(server_file);
    
	// Step 2: Make all the argyments a string
	char writebuf[MAXLEN];              //Keep the arguments passed in order to write them in pipe
	writebuf[0] = '\0';                 //Empty string

    for (i = 1; i < argc - 1; i++)
	{
		strcat(writebuf, argv[i]);      //Ignore the first arg (programm name) and concatenate everything else
		strcat(writebuf, " ");
	}

	strcat(writebuf, argv[argc - 1]);
	//printf("writebuf: %s\n", writebuf);

	//Step 3: Write the string into the pipe.
	kill(server_pid, SIGCONT);
	if ((write(compd, writebuf, MAXLEN + 1)) == -1) //write to pipe
	{
		printf("Error while writing the fifo.\n");
		exit(1);
	}

	//Step 4: wait for a reply
	char results[MAXLEN];
	memset(results,0, MAXLEN);
	if (read(serpd, results, MAXLEN + 1) < 0) {
		perror("commander : problem in reading");
		exit(5);
	}
	
	printf("\nAnswer from server:\n");
	printf("%s\n", results);
	close(compd);
}