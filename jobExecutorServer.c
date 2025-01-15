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
#include <sys/wait.h>
#include <semaphore.h>

//commanderfifo  	--> Commander writes - Server reads
//serverfifo 		--> Server writes - commander reads

void JobRequest() 
{

}

QUEUE_PTR waitingQueue = NULL;      //To store the jobs that are waiting
QUEUE_PTR runningQueue = NULL;      //To store the jobs that are running

int n = 1;

void jobExecutorServer()
{
    int compd;
	int serpd;
    char readbuf[MAXLEN];
    char response[MAXLEN];
    int i;
    char *instr;
    char jobpid[MAXLEN];
    int index = 1;

    // Open the pipe to read data
	if( (compd=open(commanderfifo, O_RDONLY)) < 0) //Commander writes - Server reads
    {
		printf("Error opening the fifo.\n");
		perror ("open failed") ;
        exit(1);
	}

	if( (serpd=open(serverfifo, O_WRONLY)) < 0) 	//Server writes - commander reads
    {
		printf("Error opening the fifo.\n");
		perror ("open failed") ;
        exit(1);
	}

    for (; ;)
    {
        pause();
        memset(readbuf,0,MAXLEN);

        if (read(compd, readbuf, MAXLEN + 1) < 0)          //read from the pipe to determine job issued by jobCommander
        {
            perror("problem in first reading ");
            exit(5);
        }
        
        //Now the message has this form: issueJob ptr
        //So the first word is what kind of instruction is.

        //printf("readbuf: %s\n", readbuf);
        instr = strtok(readbuf, " ");

        //Based on the instruction i do what i have to do
        if (strcmp(instr, "issueJob") == 0)
        {
            char *tok = strtok(NULL, "\n");
            sprintf(jobpid, "job_%d", index);
            index ++;
            int position = QUEUE_count(runningQueue) + 1;
            my_issueJob(jobpid,tok,position,0,0);
        }
        else if(strcmp(instr, "setConcurrency") == 0)
        {
            int ConcFlag = 0;                           //ConcFlag is 0 when concurrency was not increased
            char * con = strtok(NULL, " ");
            if (atoi(con) > Concurrency)                //if Concurrency was increased raise ConcFlag
            {
                ConcFlag = 1;
            }                
            Concurrency = atoi(con);

            //Now i have to check if i need to add or remove jobs
            if (ConcFlag == 1)
            {
                Update();
            }

            sprintf(response, "%s %d", "Concurrency changed to :", Concurrency);
            sendResponse(response);
        }
        else if(strcmp(instr, "stop") == 0)
        {
            
            char * tok = strtok(NULL, " ");
            int check = QUEUE_deleterunning(&runningQueue, tok);     //remove from running queue
            
            if (check == 1)     //The item was in the running queue
            {
                sprintf(response, " %s terminated.", tok);
                sendResponse(response);
                Update();
            }
            else if (check = QUEUE_deletewaiting(&waitingQueue, tok) == 1)
            {
                sprintf(response, " %s  removed.", tok);
                sendResponse(response);
            }
            else 
            {
                sprintf(response, "We coud not found the %s.", tok);
                sendResponse(response);
            }
        }
        else if(strcmp(instr, "poll") == 0)
        {
            char *  mode = strtok(NULL, " ");
            if (strcmp(mode, "running") == 0) 
            {
                char *results;                          
                results = QUEUE_Return(runningQueue);
                sendResponse(results);
            }
            else //It is queued
            {
                char *results;                          
                results = QUEUE_Return(waitingQueue);
                sendResponse(results);
            }
        }
        else  //We know it is exit because we have validate the form of instruction from main
        {
            sprintf(response, "%s", "jobExecutorServer terminated");
            sendResponse(response);
            terminateRunningJobs();
            unlink(Server);           
            exit(0);
        }
    }
    close(compd);
    close(serpd);
}


// -------------------------------FUNCTIONS---------------------------------------------
void Update() 
{
    int running = QUEUE_count(runningQueue);
    int waiting = QUEUE_count(waitingQueue);
    QUEUE_PTR temp;
    char *jobId;
    char *job;
    int queuePosition;
    pid_t pid;

    if (running < Concurrency) 
    { // Check if running jobs are less than Concurrency
        while (running < Concurrency ) 
        { 
            if(QUEUE_count(waitingQueue) == 0)
            {
                break;
            }

            JOB_INFO jobInfo = QUEUE_getfirst(waitingQueue);

            jobId = jobInfo.jobId;
            job = jobInfo.job;
            queuePosition = QUEUE_count(runningQueue);
            pid = jobInfo.pid;

            QUEUE_deletewaiting(&waitingQueue, jobId);
            printf("Job added to running queue: %s\n", jobId);
            my_issueJob(jobId, job, queuePosition,1,pid);
            running = QUEUE_count(runningQueue);
        }
    }
}

void my_issueJob(char * jobId, char *command, int queuePosition, int flag, pid_t p) 
{
    int running = QUEUE_count(runningQueue);
    int waiting = QUEUE_count(waitingQueue);
    char response[MAXLEN];
    int i;
    char *tok;
    pid_t pid;
    char *args[MAX_ARGS];
    char job[strlen(command)];
    strcpy(job, command);

    // Parse the command string
    for (int i =0; i< MAX_ARGS; i++)
    {
        args[i] = NULL;
    }

    i = 0;
    tok = strtok(command, " ");
    while (tok != NULL) {
        args[i] = strdup(tok);
        tok = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;

    if (flag == 1) 
    {
        queuePosition =  QUEUE_count(runningQueue) + 1;
        runningQueue = QUEUE_add(runningQueue, jobId, job, queuePosition, p); // Update running queue
    }
    else 
   {
        if (running < Concurrency) 
        {
            // Create another process for the job
            if ((pid = fork()) < 0) 
            {
                perror("fork");
                exit(1);
            }
            if (pid == 0) 
            {
                /*execute the command*/
                execvp(args[0], args);
                perror(args[0]);
                exit(1);
            } else
            {
                queuePosition = QUEUE_count(runningQueue) + 1;
                sprintf(response, "<Added to the running queue: %s, %s, %d>", jobId, job, queuePosition);
                runningQueue = QUEUE_add(runningQueue, jobId, job, queuePosition, pid); // Update running queue
                //QUEUE_print(runningQueue);
                sendResponse(response);
            }
        } else 
        {
            printf("Concurrency limit reached.\n");
            queuePosition = QUEUE_count(waitingQueue) + 1;
            waitingQueue = QUEUE_add(waitingQueue, jobId, job, queuePosition, pid); // Update waiting queue
            sprintf(response, "<Added to the waiting queue: %s, %s, %d>", jobId, job, queuePosition);
            sendResponse(response);
        }
   }
}

void sendResponse(char *args) 
{
    int serpd, i;
    char response[MAXLEN];
    response[0] = '\0'; 

    // printf("Running\n");
    // QUEUE_print(runningQueue);

    // printf("waiting\n");
    // QUEUE_print(waitingQueue);

    if ((serpd = open(serverfifo, O_WRONLY)) < 0) 
    { // Server writes - commander reads
        perror("Error opening the fifo");
        exit(1);
    }

    // Copy the contents of args to response
    strncpy(response, args, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0'; // Ensure null-termination
    
    // Write the complete response to the FIFO
    if (write(serpd, response, strlen(response)) == -1) {
        perror("Error while writing the fifo");
        exit(1);
    }
}

void JobDone(int signal)
{
    if (signal == SIGCHLD)
    {
        printf("\nA job has ended.\n");
        int waiting = QUEUE_count(waitingQueue);
        QUEUE_PTR temp;
        char* jobId;                  
        char * job;
        int queuePosition;
        
        //find all the children
        int status;
        pid_t pid, new;
        while ((pid = waitpid((pid_t)(-1), &status, WNOHANG)) > 0)
        {
            // printf("Running queue BEFORE deletion:\n");
            // QUEUE_print(runningQueue); 
            
            // printf("Received signal from child process with PID: %d\n", pid);
            // QUEUE_delete(&runningQueue, pid);

            //See how many are now running and see if we can add more
            int running = QUEUE_count(runningQueue);
            // printf("Running queue after deletion:\n");
            // QUEUE_print(runningQueue); 

            running = QUEUE_count(runningQueue);
            waiting = QUEUE_count(waitingQueue);

            if (running < Concurrency  && waiting > 0)
            {
                if(waiting > 0)
                {

                    JOB_INFO jobInfo = QUEUE_getfirst(waitingQueue);

                    jobId = jobInfo.jobId;
                    job = jobInfo.job;
                    queuePosition = QUEUE_count(runningQueue);
                    pid = jobInfo.pid;
                    QUEUE_deletewaiting(&waitingQueue, jobId);
                    printf("Job added to running queue: %s\n", jobId);
                    my_issueJob(jobId, job, queuePosition, 1,pid);
                }
            }
        }
    } 
    sleep(1); 
}

void terminateRunningJobs() 
{
    // Iterate through the running queue and kill each process
    QUEUE_PTR temp = runningQueue;
    while (temp != NULL) {
        kill(temp->pid, SIGTERM); 
        temp = temp->next;
    }
}