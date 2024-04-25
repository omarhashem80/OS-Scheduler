#include "headers.h"
int remainingTime;
int startTime;
int runTime;
int processNo;

void resume_process(int signum) {
    // Resume process by updating start time
    startTime = getClk();
}

void stop_process(int signum) {
    // Stop process by updating runTime and raising SIGSTOP signal
    runTime = remainingTime;
    raise(SIGSTOP);
}

int main(int argc, char *argv[]) {
     // Check if the correct number of command-line arguments is provided
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <remaining_time> <process_number>\n", argv[0]);
        return 1;
    }

    // Initialize clock
    initClk();

    // Register signal handlers
    signal(SIGCONT, resume_process);
    signal(SIGTSTP, stop_process);

    // Retrieve remaining time and process number from command-line arguments
    runTime = atoi(argv[1]);
    processNo = atoi(argv[2]);

    // Initialize start time and remaining time
    startTime = getClk();
    remainingTime = runTime;

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

