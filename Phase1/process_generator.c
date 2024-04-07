#include "headers.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    struct Queue queue;
    initializeQueue(&queue);

    // Example usage
    struct Process p1 = {1, 0, 10, 1};
    struct Process p2 = {2, 2, 8, 2};
    struct Process p3 = {3, 4, 6, 3};

    enqueue(&queue, &p1);
    enqueue(&queue, &p2);
    enqueue(&queue, &p3);

    while (!isEmpty(&queue)) {
        struct Process *p = dequeue(&queue);
        printf("Dequeued Process: ID %d\n", p->id);
    }
    
    // signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption

}

void readInputFiles(){
   
}

void getUserInput(){

}

