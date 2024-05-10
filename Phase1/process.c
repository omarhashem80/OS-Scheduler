#include "headers.h"

int remainingTime;
int startTime;
int runTime;
int processNo;

void resume_process(int signum) {
    // Resume process by updating start time
    printf("FROM PROCESS FILE :: process with number %d got continue signal\n",processNo);
    startTime = getClk();
    signal(SIGCONT, resume_process);
}

void stop_process(int signum) {
    // Stop process by updating runTime and raising SIGSTOP signal
    printf("FROM PROCESS FILE :: process with number %d go stop signal\n",processNo);
    runTime = remainingTime;
    signal(SIGTSTP, stop_process);
    raise(SIGSTOP);
}

int main(int argc, char *argv[]) {
    // Register signal handlers
    signal(SIGCONT, resume_process);
    signal(SIGTSTP, stop_process);

    // Check if the correct number of command-line arguments is provided
    printf("FROM PROCESS FILE :: file name %s\n",argv[0]);
    printf("FROM PROCESS FILE :: runtime %s\n",argv[1]);
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <remaining_time> <process_number>\n", argv[0]);
        return 1;
    }

    printf("FROM PROCESS FILE :: process id:%d\n",getpid());
    // Initialize clock
    initClk();

    // Retrieve remaining time and process number from command-line arguments
    runTime = atoi(argv[1]);
    processNo = atoi(argv[2]);

    // Initialize start time and remaining time
    startTime = getClk();
    remainingTime = runTime;
    printf("FROM PROCESS FILE :: %d\n",processNo);
    // Loop until remaining time becomes zero
    while (remainingTime > 0) {
        int elapsed = getClk() - startTime;
        remainingTime = runTime - elapsed;
    }
    
    // Cleanup
    destroyClk(false);
    // Notify parent process
    kill(getppid(), SIGUSR1); 
    exit(processNo);
}
