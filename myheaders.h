#define MAX_ARGS 8
#include <semaphore.h>

#define commanderfifo "/tmp/200216mypipe"    //Commander writes - Server reads
#define serverfifo "/tmp/200216mypipe2"    //Server writes - commander reads

extern char * Server;
extern int Concurrency;

void jobExecutorServer();
void jobCommander(int argc, char **argv, int server_pid);
void sendResponse(char *args) ;
void JobDone(int signal);

void Update();
void my_issueJob(char * jobId, char *command, int queuePosition, int flag, pid_t p); 
void JobRequest();
void terminateRunningJobs();