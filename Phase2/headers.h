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
#define MAXSIZE 1000

//***********************************************************************************//
// Define a structure for the process
struct Process {
    int id, arrival, runtime, priority,actual_id,remaining_time,waiting_time,turnaround_time;
    float WTA;
    char state[20];
    /*
    enum {
        RUNNING,
        TERMINATED,
        WAITING
    } state
    */
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

// struct PriorityQueue_Ali {
//     struct Node **heap;
//     int capacity;
//     int size;
// };

// struct PriorityQueue_Ali* createPriorityQueue(int capacity) {
//     struct PriorityQueue_Ali *pq_Ali = (struct PriorityQueue_Ali*)malloc(sizeof(struct PriorityQueue_Ali));
//     pq_Ali->capacity = capacity;
//     pq_Ali->size = 0;
//     pq_Ali->heap = (struct Node**)malloc(capacity * sizeof(struct Node*));
//     return pq_Ali;
// }

// void swap(struct Node **a, struct Node **b) {
//     struct Node *temp = *a;
//     *a = *b;
//     *b = temp;
// }

// void heapify(struct PriorityQueue_Ali *pq_Ali, int idx) {
//     int smallest = idx;
//     int left = 2 * idx + 1;
//     int right = 2 * idx + 2;

//     if (left < pq_Ali->size && pq_Ali->heap[left]->data->remaining_time < pq_Ali->heap[smallest]->data->remaining_time)
//         smallest = left;

//     if (right < pq_Ali->size && pq_Ali->heap[right]->data->remaining_time < pq_Ali->heap[smallest]->data->remaining_time)
//         smallest = right;

//     if (smallest != idx) {
//         swap(&pq_Ali->heap[idx], &pq_Ali->heap[smallest]);
//         heapify(pq_Ali, smallest);
//     }
// }

// void insert(struct PriorityQueue_Ali *pq_Ali, struct Process* p) {
//     if (pq_Ali->size == pq_Ali->capacity) {
//         printf("Priority queue is full.\n");
//         return;
//     }
//     int i = pq_Ali->size;
//     pq_Ali->size++;
//     pq_Ali->heap[i] = (struct Node*)malloc(sizeof(struct Node));
//     pq_Ali->heap[i]->data = p;

//     while (i != 0 && pq_Ali->heap[(i - 1) / 2]->data->remaining_time > pq_Ali->heap[i]->data->remaining_time) {
//         swap(&pq_Ali->heap[i], &pq_Ali->heap[(i - 1) / 2]);
//         i = (i - 1) / 2;
//     }
// }

// struct Node* extractMin(struct PriorityQueue_Ali *pq_Ali) {
//     if (pq_Ali->size <= 0) {
//         return NULL;
//     }

//     if (pq_Ali->size == 1) {
//         pq_Ali->size--;
//         return pq_Ali->heap[0];
//     }

//     struct Node* root = pq_Ali->heap[0];
//     pq_Ali->heap[0] = pq_Ali->heap[pq_Ali->size - 1];
//     pq_Ali->size--;
//     heapify(pq_Ali, 0);
//     return root;
// }

// int isEmpty_Ali(struct PriorityQueue_Ali *pq_Ali) {
//     return pq_Ali->size == 0;
// }

// void freeMemory_Ali(struct PriorityQueue_Ali *pq_Ali) {
//     // Free memory allocated to the nodes in the priority queue
//     for (int i = 0; i < pq_Ali->size; i++) {
//         free(pq_Ali->heap[i]->data);
//         free(pq_Ali->heap[i]);
//     }

//     // Free memory allocated to the heap array and the priority queue itself
//     free(pq_Ali->heap);
//     free(pq_Ali);
// }
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
    return p;
}

// Function to check if the queue is empty
int isEmpty(struct Queue *queue) {
    return queue->size == 0;
}

// Function to destroy the queue and deallocate memory
void destroyQueue(struct Queue *queue) {
    // Traverse the queue and deallocate memory for each node
    struct Node *current = queue->front;
    while (current != NULL) {
        struct Node *temp = current;
        current = current->next;
        
        // Check if process data exists before deallocating
        if (temp->data != NULL) {
            free(temp->data); // Deallocate memory for the process data
        }
        
        free(temp);       // Deallocate memory for the node
    }

    // Reset queue attributes
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

//***********************************************************************************//

struct msgbuff
{
	long mtype;
	struct Process process;
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


// -----------------------------------------------------------------------------------------

struct PNode {
    struct Process* P;
    int pr;
};
struct PriorityQueue {
    struct PNode** A;
    int heap_size;
};

// Function to initialize a priority queue
void InitPriorityQueue(struct PriorityQueue* pq) {
    pq->A = (struct PNode**)malloc((MAXSIZE + 1) * sizeof(struct PNode*));
    for (int i = 0; i <= MAXSIZE; i++) {
        pq->A[i] = (struct PNode*)malloc(sizeof(struct PNode));
    }
    pq->heap_size = 0;
}


// Utility functions for heap operations
int left(int i) {
    return 2 * i;
}

int right(int i) {
    return 2 * i + 1;
}

int parent(int i) {
    return i / 2;
}

// Function to maintain MinHeap property
void MIN_HEAPIFY(struct PriorityQueue* pq, int i) {
    int l = left(i);
    int r = right(i);
    int smallest = i;
    if (l <= pq->heap_size && pq->A[l]->pr < pq->A[i]->pr)
        smallest = l;
    if (r <= pq->heap_size && pq->A[r]->pr < pq->A[smallest]->pr)
        smallest = r;
    if (smallest != i) {
        struct PNode* temp = pq->A[i];
        pq->A[i] = pq->A[smallest];
        pq->A[smallest] = temp;
        MIN_HEAPIFY(pq, smallest);
    }
}


// Function to decrease key of a node in MinHeap
void DECREASE_KEY(struct PriorityQueue* pq, int i, int k) {
    if (k > pq->A[i]->pr) {
        printf("Error: New key is greater than current key\n");
        return;
    }
    pq->A[i]->pr = k;
    while (i > 1 && pq->A[parent(i)]->pr > pq->A[i]->pr) {
        struct PNode* temp = pq->A[i];
        pq->A[i] = pq->A[parent(i)];
        pq->A[parent(i)] = temp;
        i = parent(i);
    }
}

void Insert(struct PriorityQueue* pq, struct Process* p) {
    struct PNode* pnode = (struct PNode*)malloc(sizeof(struct PNode)); // Allocate memory for pnode
    if (pnode == NULL) {
        printf("Error: Memory allocation failed\n");
        return;
    }
    pnode->P = p;
    pnode->pr = p->priority;
    pq->heap_size++;
    pq->A[pq->heap_size] = pnode;
    DECREASE_KEY(pq, pq->heap_size, pnode->pr);
}

struct Process* Extract_Min(struct PriorityQueue* pq) {
    if (pq->heap_size < 1) {
        printf("Error: Heap underflow\n");
    }
    struct PNode* MIN = pq->A[1];
    pq->A[1] = pq->A[pq->heap_size];
    pq->A[pq->heap_size] = MIN;
    pq->heap_size--;
    MIN_HEAPIFY(pq, 1);
    return MIN->P;
}
bool IsEmpty(struct PriorityQueue* pq) {
    return pq->heap_size == 0;
}
void DestroyPriorityQueue(struct PriorityQueue* pq) {
    for(int i=0;i<pq->heap_size;i++) {
        free(pq->A[i]->P);
        free(pq->A[i]);
    }
    free(pq);
}
