#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#define true 1
#define false 0

#define SHKEY 300

// Define a structure for the process
struct Process {
    int id, arrival, runtime, priority;
};

// Define a structure for the node in the queue
struct Node {
    struct Process *data;
    struct Node *next;
};

// Define a structure for the queue
struct Queue {
    struct Node *front;
    struct Node *rear;
    int size;
};

// Function to initialize the queue
void initializeQueue(struct Queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

// Function to enqueue a process
void enqueue(struct Queue *queue, struct Process *p) {
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    newNode->data = p;
    newNode->next = NULL;

    if (!queue->front) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

// Function to dequeue a process
struct Process* dequeue(struct Queue *queue) {
    if (queue->front == NULL) return NULL;

    struct Node *temp = queue->front;
    struct Process *p = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) queue->rear = NULL;

    free(temp);
    queue->size--;
    return p;
}

struct Process* peek(struct Queue *queue) {
    if (queue->front == NULL) return NULL;
    struct Node *temp = queue->front;
    struct Process *p = temp->data;
    free(temp);
    return p;
}

// Function to check if the queue is empty
int isEmpty(struct Queue *queue) {
    return queue->size == 0;
}

struct msgbuff
{
	long mtype;
	struct Process* process;
};

///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}
