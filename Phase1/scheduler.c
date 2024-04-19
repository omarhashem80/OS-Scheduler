#include "headers.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include<string.h>
struct Queue process_queue;
struct Process * running_process;
struct Process * terminated_process;
FILE *outputFilePointer; // Declare a file pointer
void process_terminated(int signum);
void update_PCB(struct Process * p,int finished){
    //waiting time =current time- arrival time-time taken in cpu uptil now
    p->waiting_time=getClk()-p->arrival-(p->runtime-p->remaining_time);
    fprintf(outputFilePointer, "At time\t%d\tprocess\t%d\t%s\tarrive\t%d\ttotal\t%d\tremain\t%d\twait\t%d",getClk(),p->id,p->state,p->arrival,p->runtime,p->remaining_time,p->waiting_time);
    if(finished){
        fprintf(outputFilePointer,"\tTA\t%d\tWTA\t%.2f\n",p->turnaround_time,p->turnaround_time*1.0/p->runtime);
    }else
        fprintf(outputFilePointer,"\n");
}
int main(int argc, char *argv[]) {
    initClk();
    signal(SIGUSR1, process_terminated);
    printf("FROM SCHDULER FILE :: Hello from scheduler\n");
    printf("FROM SCHDULER FILE :: Scheduler\n");
    printf("FROM SCHDULER FILE :: algo: %s\n", argv[1]); 
    printf("FROM SCHDULER FILE :: timeSlice: %s\n", argv[2]); 
    outputFilePointer = fopen("./outputs/scheduler.log", "w");
    if (outputFilePointer == NULL) {
        printf("Error opening the file.\n");
        return 1; // Exit with an error code
    }
    int q_key = 22;
    int quantum;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
    if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
    initializeQueue(&process_queue);
    struct msgbuff message;
    if(atoi(argv[1])==3){ //round robin
        terminated_process=NULL;
        quantum=atoi(argv[2]);
        int pick_new_process=quantum;
        int next_time=getClk();
        running_process=NULL;
        int g=20;
        while (1)
        {  
            if((next_time+1)==getClk()){
                printf("next time:%d clk:%d\n",next_time,getClk());
                //new time unit
                //you should update process 
                 next_time++;
                 if(running_process!=NULL){
                    running_process->remaining_time--;
                    printf("remaing time of process:%d is %d\n",running_process->id,running_process->remaining_time);
                    if(running_process->remaining_time==0){
                        pick_new_process=quantum;
                    }else
                    pick_new_process++;
                 }
            }   
            int rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
            //printf("FROM SCHDULER FILE :: HI:%d \n",message.process->id);
            //printf("FROM SCHDULER FILE :: shceduler recieved val%d \n",rec_val);
            int f=0;
            if(rec_val!=-1){//new process arrives
                printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);                
                // Convert the integer to a string
                char runtime[20]; 
                sprintf(runtime, "%d", message.process.runtime);
                char id [20]; 
                sprintf(id, "%d", message.process.id);
                int pid=fork();
                if(pid==0){
                   // printf("FROM SCHDULER FILE :: process id :%d\n",getpid());
                   // execl("./process.out",runtime,message.process.id,NULL);
                   char* processAgruments[]={"process.out", runtime,id, NULL};
                    execv(realpath("process.out", NULL),processAgruments);
                }
                struct Process * p=malloc(sizeof(struct Process));
                p->actual_id=pid;
                p->runtime=message.process.runtime;
                p->arrival=message.process.arrival;
                p->id=message.process.id;
                p->remaining_time=message.process.runtime;
                strcpy(p->state,"stopped\0");
                printf("FROM SCHDULER FILE :: schulder id :%d\n",getpid());
                p->priority=message.process.priority;
                enqueue(&process_queue,p);
                kill(pid,SIGTSTP);
            }

            if((pick_new_process==quantum)){
                if(running_process!=NULL){
                    int f=0;
                    printf("**********************************");
                    printf("singal should be passed to %d",running_process->actual_id);
                    printf("**********************************\n");
                    printf("remaining time should be zero:%d\n",running_process->remaining_time);
                    if(running_process->remaining_time==0){
                        //todo: wait the process for exit code
                        f=1;
                        printf("/////////////////////////////////\n");
                        printf("Process should terminats if side\n");
                        printf("/////////////////////////////////\n");
                        strcpy(running_process->state,"finish");
                        running_process->turnaround_time=getClk()-running_process->arrival;
                        //Waiting Time = Turnaround Time - running time
                        running_process->waiting_time=running_process->turnaround_time-running_process->runtime;
                        terminated_process=running_process;                        
                    }else{
                        strcpy(running_process->state,"stopped");
                        kill(running_process->actual_id,SIGTSTP);
                        printf(" proces ID:%d entered the queue again with remainig time %d queue size :%d\n",running_process->id,running_process->remaining_time,process_queue.size);
                
                        enqueue(&process_queue,running_process);
                    }
                    
                    update_PCB(running_process,f);
                    if(!(g--)){
                        fclose(outputFilePointer);
                        return 0;
                        
                    }
                }
                running_process=NULL;
                //pick new process
                if(!isEmpty(&process_queue)){
                    printf("**********************************");
                    printf("number of processes in the queue:%d",process_queue.size);
                    printf("**********************************\n");
                    running_process= dequeue(&process_queue);
                    pick_new_process=0;
                    strcpy(running_process->state,"resumed");
                    if(running_process->remaining_time==running_process->runtime){
                        strcpy(running_process->state,"started");
                    }
                    update_PCB(running_process,0);
                    kill(running_process->actual_id,SIGCONT);
                }
            }
        }
    }
    fclose(outputFilePointer);
    sleep(5);// remove it
    destroyClk(false);
    return 0;
}
void process_terminated(int signum){
//     printf("\n");
    // while (terminated_process==NULL)
    // {}

   if(terminated_process!=NULL){
        //printf("&&&&&*ERROR IN TERMINATING THE PROCESS*&&&&&");
    printf("\n Process %d WAS TERMINATED at %d ",terminated_process->id,terminated_process->turnaround_time);
    free(terminated_process);  
    terminated_process=NULL;
   }else 
        printf("TERMINATION NOT DONT YET\n");
}

