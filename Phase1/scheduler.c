#include "headers.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("FROM SCHDULER FILE :: Hello from scheduler\n");
    printf("FROM SCHDULER FILE :: Scheduler\n");
    printf("FROM SCHDULER FILE :: algo: %s\n", argv[1]); 
    printf("FROM SCHDULER FILE :: timeSlice: %s\n", argv[2]); 
    initClk();

    int q_key = 22;
    int quantum;
    int q_id = msgget(q_key, 0666 | IPC_CREAT);
    if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
    struct Queue process_queue;
    initializeQueue(&process_queue);
    struct msgbuff message;
    // message.process=malloc(sizeof(struct Process));
    // message.process->id=-1;
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    if(atoi(argv[1])==3){ //round robin
        quantum=atoi(argv[2]);
        int pick_new_process=quantum;
        int next_time=getClk();
        struct Process * running_process=NULL;

        while (1)
        {  
            if((next_time+1)==getClk()){
            printf("next time:%d clk:%d\n",next_time,getClk());
                //new time unit
                //you should update process 
                 next_time++;
                 if(running_process!=NULL){
                    running_process->remaining_time--;
                    pick_new_process++;
                 }
            }   
            int rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
            //printf("FROM SCHDULER FILE :: HI:%d \n",message.process->id);
            //printf("FROM SCHDULER FILE :: shceduler recieved val%d \n",rec_val);
            
            if(rec_val!=-1){//new process arrives
                printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);
                printf("FROM SCHDULER FILE :: schulder id :%d\n",getpid());
                
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
                kill(pid,SIGTSTP);
                printf("FROM SCHDULER FILE :: schulder id :%d\n",getpid());
                message.process.actual_id=pid;
                message.process.remaining_time=message.process.runtime;
                enqueue(&process_queue,&message.process);
                //go into while loop each time send the new remaining time to this process

            }

            if((pick_new_process==quantum)){
                printf("SCHDEULER ENTERED \n");
                //stop running process 
                if(running_process!=NULL){
                    kill(running_process->actual_id,SIGTSTP);
                    running_process->state=WAITING;
                    if(running_process->remaining_time==0){
                        //todo: wait the process for exit code 
                        printf("FROM SCHDULER FILE :: process with id:%d terminated \n",running_process->id);
                        running_process->state=TERMINATED;
                        running_process->turnaround_time=getClk()-running_process->arrival;
                        //Waiting Time = Turnaround Time - running time
                        running_process->waiting_time=running_process->turnaround_time-running_process->runtime;
                    }else{
                        printf("new wating time proces num:%d is %d",running_process->id,running_process->remaining_time);
                        running_process->state=WAITING;
                        enqueue(&process_queue,running_process);
                    }
                }
                //pick new process
                printf("SCHDEULER ENTERED \n");
                if(!isEmpty(&process_queue)){
                    printf("SCHDEULER ENTERED \n");
                    running_process= dequeue(&process_queue);
                    pick_new_process=0;
                    running_process->state=RUNNING;
                    kill(running_process->actual_id,SIGCONT);
                }
            }
        }
    }
    sleep(5);// remove it
    destroyClk(false);
    return 0;

}
