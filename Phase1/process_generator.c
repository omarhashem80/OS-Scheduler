#include "headers.h"
#include<string.h>
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
    strcpy((char *)shmaddr, "1"); 
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    msgctl(q_id, IPC_RMID, (struct msqid_ds *)0);
    destroyQueue(&processQueue);
    kill(0, SIGINT);
    exit(0);
}

// Function to read the input file and populate the queue
void readInputFile(const char *filename, struct Queue *queue) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    // Read and discard the first line (header)
    fgets(line, sizeof(line), file);
    // Read process information from the file and enqueue them into the queue
    while (fgets(line, sizeof(line), file) != NULL) {
        struct Process *p = malloc(sizeof(struct Process));
        if (p == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        //i++;
        sscanf(line, "%d %d %d %d", &p->id, &p->arrival, &p->runtime, &p->priority);
        enqueue(queue, p);
    }
    // Close the file
    fclose(file);
}

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);

    // for(int i=1; i< argc; i++)
    //     printf("generator: %s\n",argv[i]);

    int algorithmNO, timeSlice = -1;  
    int schedulerID;  
    char * path = argv[2];
    
    // TODO Initialization
    // 1. Read the input files.
    initializeQueue(&processQueue);
    readInputFile(path, &processQueue);
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    algorithmNO = atoi(argv[1]);
    timeSlice = atoi(argv[3]);
    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        //printf("child1 to scheduler\n");
        char algorithmArg[16], timeSliceArg[16]; // Assuming algorithmNO and timeSlice won't exceed 15 digits
        sprintf(algorithmArg, "%d", algorithmNO);
        sprintf(timeSliceArg, "%d", timeSlice);
        char *schedulerARGS[] = {"scheduler.out", algorithmArg, timeSliceArg, NULL}; 
        execv(realpath("scheduler.out", NULL),schedulerARGS);
        perror("Scheduler failed\n");
        exit(EXIT_FAILURE);
    } else{
        schedulerID = pid;
        pid = fork();
        if(pid==-1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }else if (pid==0){
            char * clockARGS [] = {"clk.out", NULL};
            //printf("child2 to clk\n");
            execv(realpath("clk.out", NULL),clockARGS);
            perror("Child failed\n");
            exit(EXIT_FAILURE);
        }
    }
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int time = getClk();
    //printf("current time is %d\n", time);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    int q_key = 22;
    q_id = msgget(q_key, 0666 | IPC_CREAT);
     if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
    while(!isEmpty(&processQueue)){
        struct Process *p = peek(&processQueue);
        if(p->arrival== getClk()){
            struct msgbuff message;
            message.process = *p;
            message.mtype = 5;
            int send_val = msgsnd(q_id,  &message, sizeof(message.process), !IPC_NOWAIT);
            if (send_val == -1)
                perror("Errror in send");

            dequeue(&processQueue);
        }
        
    }
    //send singal to inform the scheduler that there is no incoming processes
    kill(schedulerID, SIGUSR2);
    //wait the scduler to finish
    waitpid(schedulerID, NULL, 0);
    //printf("sbye\n");
    // 7. Clear queue resources
    destroyQueue(&processQueue);
    key_t key = 50;  // Generate a key for the shared memory segment
    shmid = shmget(key, 4096, IPC_CREAT | 0666);
    // Create a shared memory segment
    if (shmid < 0 ){
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    
    writer(shmid);  // Write to shared memory
    //end();
    // 8. Clear clock resources
    destroyClk(true);
    return 0;
}