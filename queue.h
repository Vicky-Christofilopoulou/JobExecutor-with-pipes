#define MAXLEN 500      //For buffers

struct node
{
	char * jobId;                  // The 3 characteristics that we get from issueJob
	char * job;
    int queuePosition;
	pid_t pid;
	struct node * next;          //next in the queque
};

// Struct for job information - For QUEUE_Return function
struct JobInfo {
    char *jobId;
    char *job;
    int queuePosition;
    pid_t pid;
};

typedef struct JobInfo JOB_INFO;
typedef struct node QUEUE_NODE;
typedef struct node *QUEUE_PTR;


extern QUEUE_PTR waitingQueue; // To store the jobs that are waiting
extern QUEUE_PTR runningQueue; // To store the jobs that are running

void QUEUE_init(QUEUE_PTR *head);       
void QUEUE_destroy(QUEUE_PTR *head);
int QUEUE_empty(QUEUE_PTR head);
void QUEUE_print(QUEUE_PTR queue);
int QUEUE_count (QUEUE_PTR head);
QUEUE_PTR QUEUE_add(QUEUE_PTR head, char* JobId, char* job, int queuePosition, pid_t pid);
void QUEUE_delete(QUEUE_PTR *head, pid_t pid);
int QUEUE_deletewaiting(QUEUE_PTR *head, char *jobId);
int QUEUE_deleterunning(QUEUE_PTR *head, char *jobId);
JOB_INFO QUEUE_getfirst(QUEUE_PTR head);
char *QUEUE_Return(QUEUE_PTR head);
