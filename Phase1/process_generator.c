#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>

#define FILENAME "file.txt"
#define SHM_SIZE 1024

int q_id;
struct Queue processQueue;
int shmid;

void writer(int shmid) {
    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("Error in attach in writer");
        exit(EXIT_FAILURE);
    }
    printf("\nGenerator: Shared memory attached at address %p\n", shmaddr);
    strcpy((char *)shmaddr, "1");
    printf("%s", (char *)shmaddr); 
    shmdt(shmaddr);
}

void clearResources(int signum)
{
    msgctl(q_id, IPC_RMID, NULL);
    destroyQueue(&processQueue);
    //shmctl(shmid, IPC_RMID, NULL);
    kill(0, SIGINT);
    exit(0);
}

void readInputFile(const char *filename, struct Queue *queue) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    fgets(line, sizeof(line), file); // Read and discard the first line (header)
    while (fgets(line, sizeof(line), file) != NULL) {
        struct Process *p = malloc(sizeof(struct Process));
        if (p == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        sscanf(line, "%d %d %d %d", &p->id, &p->arrival, &p->runtime, &p->priority);
        enqueue(queue, p);
    }
    fclose(file);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);

    int algorithmNO, timeSlice = -1;
    int schedulerID;
    char *path = argv[2];
    
    initializeQueue(&processQueue);
    readInputFile(path, &processQueue);

    algorithmNO = atoi(argv[1]);
    timeSlice = atoi(argv[3]);

    int pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        char algorithmArg[16], timeSliceArg[16];
        sprintf(algorithmArg, "%d", algorithmNO);
        sprintf(timeSliceArg, "%d", timeSlice);
        char *schedulerARGS[] = {"scheduler.out", algorithmArg, timeSliceArg, NULL}; 
        execv(realpath("scheduler.out", NULL), schedulerARGS);
        perror("Scheduler failed\n");
        exit(EXIT_FAILURE);
    } else {
        schedulerID = pid;
        pid = fork();
        if(pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            char *clockARGS[] = {"clk.out", NULL};
            execv(realpath("clk.out", NULL), clockARGS);
            perror("Child failed\n");
            exit(EXIT_FAILURE);
        }
    }

    initClk();
    int time = getClk();
    printf("current time is %d\n", time);

    int q_key = 22;
    q_id = msgget(q_key, 0666 | IPC_CREAT);
    if (q_id == -1) {
        perror("Error in create up queue");
        exit(-1);
    }

    while (!isEmpty(&processQueue)) {
        struct Process *p = peek(&processQueue);
        if (p->arrival == getClk()) {
            struct msgbuff message;
            message.process = *p;
            message.mtype = 5;
            int send_val = msgsnd(q_id, &message, sizeof(message.process), 0);
            if (send_val == -1)
                perror("Error in send");

            dequeue(&processQueue);
        }
    }

    kill(schedulerID, SIGUSR2);
    waitpid(schedulerID, NULL, 0);
    printf("sbye\n");

    destroyQueue(&processQueue);
    key_t key = 50;
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0 ){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory created with id %d\n", shmid);
    writer(shmid);
    destroyClk(true);
    return 0;
}
