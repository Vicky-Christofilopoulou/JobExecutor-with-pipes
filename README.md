# JobExecutor-with-pipes
## 1. jobExecutor

At the heart of this system lies the **jobExecutor**, a powerful tool designed to manage and execute user-submitted jobs with precision and controlled concurrency. The system efficiently handles tasks using the following key concepts:

### Core Concepts:
- **Concurrency:** Dynamically manages the number of concurrently executed jobs. For instance, with a concurrency level set to 4, the system processes the first four jobs while queuing the rest.
- **Job Queue:** Jobs are assigned unique identifiers (jobID) and queued, executing as space becomes available based on the current concurrency limit.

### System Components:
- **jobCommander:** A command-line interface (CLI) that empowers users to interact with the system, issuing commands and receiving feedback.
- **jobExecutorServer:** The backbone of the system, this server is responsible for handling job execution, ensuring tasks are processed according to user commands and concurrency settings.

### jobExecutor Commands & Features:
- **issueJob <job>:** Submit a job for execution, receiving a unique jobID (e.g., job_01, job_02) and its position in the queue.
- **setConcurrency <N>:** Adjust the maximum number of concurrently executing jobs. The default is 1, but this can be changed on the fly without halting running jobs.
- **stop <jobID>:** Terminate a running job or remove one from the queue, with a feedback message (e.g., `job_XX terminated`).
- **poll [running, queued]:** View the list of jobs in various states: running or queued.
  - **poll running:** Displays jobs actively being executed.
  - **poll queued:** Shows jobs waiting in the queue.
- **exit:** Gracefully terminates the jobExecutorServer and notifies the jobCommander.

### Detailed Job Flow:
- Jobs enter the system via **issueJob** and are assigned a unique jobID and queue position.
- The system adheres to the concurrency level, running up to a specified number of jobs simultaneously.
- As jobs finish (via the SIGCHLD signal), the next job in the queue is started.
- The **jobExecutorServer** uses **fork()** to create child processes, executing the tasks through system calls like **exec*()**.

### System Communication:
- Communication between **jobCommander** and **jobExecutorServer** happens through **named pipes (FIFO)**.
- The jobExecutorServer generates a **jobExecutorServer.txt** file, storing its process ID (PID), which the jobCommander uses to verify the server's status.
- Named pipes facilitate command transmission, and the SIGCHLD signal triggers the restart of jobs upon completion.

---

## 2. Bash Scripting

Elevate the **jobExecutor**'s functionality with these bash scripts designed for seamless user interaction:

- **multijob.sh <file1> <file2> ... <fileN>:** This script processes multiple job files, each containing a list of jobs, and submits them to the system for execution.
- **allJobsStop.sh:** Instantly halts all active jobs, whether running or queued, offering users complete control over job management.
