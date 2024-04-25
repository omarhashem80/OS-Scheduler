#include "headers.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Hello from scheduler\n");
    printf("Scheduler\n");
    printf("algo: %s\n", argv[1]); 
    printf("timeSlice: %s\n", argv[2]); 
    initClk();

    // int q_key = 22;
    // int quantum;
    // int q_id = msgget(q_key, 0666 | IPC_CREAT);
    // if (q_id == -1){
    //     perror("Error in create up queue");
    //     exit(-1);
    // }
    // struct Queue process_queue;
    // initializeQueue(&process_queue);
    // struct msgbuff message;
    // //TODO implement the scheduler :)
    // //upon termination release the clock resources.
    // if(atoi(argv[1])==3){ //round robin
    //     quantum=atoi(argv[2]);
    //     int pick_new_process=quantum;
    //     int next_time=getClk();
    //     struct Process * running_process=NULL;
    //     while (1)
    //     {     
    //         int rec_val= msgrcv(q_id, &message, sizeof(message.process),5, IPC_NOWAIT);
    //         printf("shceduler recieves a new process with id:%d",message.process->id);
    //         if(rec_val!=-1){//new process arrives
    //             //fork new process
    //             int pid=fork();
    //             // Convert the integer to a string
    //             char runtime[20]; 
    //             sprintf(runtime, "%d", message.process->runtime);
    //             if(pid==0)
    //                 execl("process.out",runtime,message.process->id,NULL);
                
    //             kill(pid,SIGSTOP);
    //             message.process->actual_id=pid;
    //             //message.process->excution_time=0;
    //             message.process->remaining_time=message.process->runtime;
    //             enqueue(&process_queue,message.process);
    //             //go into while loop each time send the new remaining time to this process

    //         }else{
    //             printf("NO process arrive this time slice");
    //         }

    //         if((pick_new_process==quantum)&& !isEmpty(&process_queue)){
    //             //pick new process
    //             if(running_process!=NULL){
    //                 kill(running_process->actual_id,SIGSTOP);
    //                 running_process->state=WAITING;
    //                 if(running_process->remaining_time==0){
    //                     running_process->state=TERMINATED;
    //                     running_process->turnaround_time=getClk()-running_process->arrival;
    //                     //Waiting Time = Turnaround Time - running time
    //                     running_process->waiting_time=running_process->turnaround_time-running_process->runtime;
    //                 }
    //             }
    //             running_process= dequeue(&process_queue);
    //             pick_new_process=0;
    //             running_process->state=RUNNING;
    //             kill(running_process->actual_id,SIGCONT);

    //         }
    //         if((next_time+1)==getClk()){
    //             //new time unit
    //             //you should update process 
    //              next_time++;
    //              pick_new_process++;
    //              running_process->remaining_time--;
    //              running_process->state=RUNNING;
    //            }
    //         else
    //             continue;

    //     }
    // }
    sleep(5);// remove it
    destroyClk(true);
    return 0;

}
