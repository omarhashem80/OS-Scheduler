#include <math.h>
#include "headers.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include<string.h>
/*
Phase 2
*/
int starter_locations[1024]={0};
struct Queue waiting_queue;
bool allocate_memory(struct Process * p);
void deallocate_memory(int i);
void fork_process(struct Process * p);
FILE *memory_output_file;

/*
Phase 2
*/
struct Queue process_queue;
struct Process * running_process;
struct Process * terminated_process;
struct msgbuff message;
int total_cpu_time;
int ideal_time=0;
int total_non_ideal_time=0;
float avg_WTA=0;
float sum_WTA_squared=0;
float avg_wating_time=0;
float std_WTA=0;
int number_of_processes=0;
int q_key = 22;
int q_id;
bool flag=true;
int no_more_processes_to_recieve=0;
FILE *outputFilePointer; // Declare a file pointer
void process_terminated(int signum);
void process_generator_terminated(int signum);
void rr_start(int quantum);
void write_scheduler_perf();
//void receive_processes(int q_id, bool WAIT);
void hpf_start();
void update_PCB(struct Process * p,int finished){
    //waiting time =current time- arrival time-time taken in cpu uptil now
    p->waiting_time=getClk()-p->arrival-(p->runtime-p->remaining_time);
    fprintf(outputFilePointer, "At time\t%d\tprocess\t%d\t%s\tarrive\t%d\ttotal\t%d\tremain\t%d\twait\t%d",getClk(),p->id,p->state,p->arrival,p->runtime,p->remaining_time,p->waiting_time);
    if(finished){
        sum_WTA_squared+=p->WTA * p->WTA;
        avg_wating_time+=running_process->waiting_time;
        avg_WTA+=running_process->WTA;
        number_of_processes++;
        total_non_ideal_time+=p->runtime;
        fprintf(outputFilePointer,"\tTA\t%d\tWTA\t%.2f\n",running_process->turnaround_time,running_process->WTA);
    }else
        fprintf(outputFilePointer,"\n");
}
void memory_log(struct Process * p,int allocated){
    //waiting time =current time- arrival time-time taken in cpu uptil now
    char status[]="allocated";
    if(!allocated){
        strcpy(status,"freeeeeed");
    }
    fprintf(memory_output_file, "At time\t%d\t%s\t%d\tbytes\tfor\tprocess\t%d\tfrom\t%d\tto\t%d\n",getClk(),status,p->max_size,p->id,p->start_address,p->end_address);
}
int main(int argc, char *argv[]) {
    signal(SIGUSR1, process_terminated);
    signal(SIGUSR2, process_generator_terminated);
    printf("FROM SCHDULER FILE :: Hello from scheduler\n");
    printf("FROM SCHDULER FILE :: Scheduler\n");
    printf("FROM SCHDULER FILE :: algo: %s\n", argv[1]); 
    printf("FROM SCHDULER FILE :: timeSlice: %s\n", argv[2]); 
    outputFilePointer = fopen("./outputs/scheduler.log", "w");
    memory_output_file = fopen("./outputs/memory.log", "w");
    if (outputFilePointer == NULL) {
        printf("Error opening the file.\n");
        return 1; // Exit with an error code
    }
    q_id = msgget(q_key, 0666 | IPC_CREAT);
    if (q_id == -1){
        perror("Error in create up queue");
        exit(-1);
    }
    initializeQueue(&process_queue);
    initClk();
    // ---------------------------------------------------------------------------------------------------------
    if(atoi(argv[1])==1) { //HPF
        hpf_start();
    // ---------------------------------------------------------------------------------------------------------
    }else if(atoi(argv[1])==3){ //round robin
        rr_start(atoi(argv[2]));
    }
    fclose(outputFilePointer);
    sleep(5);
    destroyClk(false);
    printf("\n-------------------------------------------------------\n");
    printf("\n-------------------------------------------------------\n");
    printf("\n\t\t\tscheduer finished\n");
    printf("\n-------------------------------------------------------\n");
    printf("\n-------------------------------------------------------\n");
    return 0;
}
void process_terminated(int signum){
   if(terminated_process!=NULL){
    printf("\n Process %d WAS TERMINATED at %d ",terminated_process->id,terminated_process->turnaround_time);
    free(terminated_process);  
    terminated_process=NULL;
   }else 
        printf("TERMINATION NOT DONT YET\n");
}
void process_generator_terminated(int signum){
    no_more_processes_to_recieve=1;
}
void rr_start(int quantum){
        terminated_process=NULL;
        int pick_new_process=quantum;
        int next_time=getClk();
        running_process=NULL;
        while (running_process!=NULL||!no_more_processes_to_recieve||!isEmpty(&process_queue)||!isEmpty(&waiting_queue))
        {  
            if((next_time+1)==getClk()){
                printf("number of waiting process:%d\n",waiting_queue.size);
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
                 }else{
                    ideal_time++;
                    printf("/////////*************////////////cpu is ideal now \n");
                 }
            }
            int rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
            int f=0;
            if(rec_val!=-1){//new process arrives
                printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);                
                struct Process * p=malloc(sizeof(struct Process));
                p->runtime=message.process.runtime;
                p->arrival=message.process.arrival;
                p->id=message.process.id;
                p->remaining_time=message.process.runtime;
                p->max_size=message.process.max_size;
                printf("HI/n");
                printf("Max size :%d\n",p->max_size);
                strcpy(p->state,"stopped\0");
                if(allocate_memory(p)){
                printf("H\n");
                    printf("success to allocate\n");
                    fork_process(p);
                    enqueue(&process_queue, p);
                    memory_log(p,1);
                }else{
                    printf("Fail to allocate\n");
                    pushPQ(&waiting_queue,p,true);
                }
                
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
                        running_process->WTA=running_process->turnaround_time*1.0/running_process->runtime;
                        running_process->waiting_time=running_process->turnaround_time-running_process->runtime;
                        terminated_process=running_process; 
                        //free memory after process terminated  
                        deallocate_memory(terminated_process->start_address);
                        memory_log(running_process,0);
                        
                        //use the freed memory of the terminated process
                        bool has_mem=true;
                        while (has_mem&&!isEmpty(&waiting_queue))
                        {
                            struct Process * p=peek(&waiting_queue);
                            has_mem=allocate_memory(p);
                            if(has_mem){
                                fork_process(p);
                                enqueue(&process_queue,p);
                                dequeue(&waiting_queue);
                                memory_log(p,1);
                            }
                        }
                                              
                    }else{
                        strcpy(running_process->state,"stopped");
                        kill(running_process->actual_id,SIGTSTP);
                        printf(" proces ID:%d entered the queue again with remainig time %d queue size :%d\n",running_process->id,running_process->remaining_time,process_queue.size);
                
                        enqueue(&process_queue,running_process);
                    }
                    
                    update_PCB(running_process,f);                            
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
    total_cpu_time=getClk();
    write_scheduler_perf();
}
void write_scheduler_perf(){
    FILE *perfFilePointer; 
    perfFilePointer = fopen("./outputs/scheduler.perf", "w");
    //fprintf(perfFilePointer,"total cpu time\t%d\tideal time\t%d\n",total_cpu_time,ideal_time);
    fprintf(perfFilePointer,"CPU Utilization\t%.2f\n",(total_cpu_time-ideal_time)*100.0/total_cpu_time);
    //fprintf(perfFilePointer,"CPU Utilization\t%.2f\n",total_non_ideal_time*1.0/total_cpu_time);
    avg_wating_time/=number_of_processes;
    avg_WTA/=number_of_processes;
    std_WTA=(sum_WTA_squared/number_of_processes) - (avg_WTA*avg_WTA);
    std_WTA=sqrt(std_WTA);
    fprintf(perfFilePointer,"Avg WTA\t%.2f\n",avg_WTA);
    fprintf(perfFilePointer,"Avg Waiting\t%.2f\n",avg_wating_time);
    fprintf(perfFilePointer,"std WTA\t%.2f\n",std_WTA);

}

// void receive_processes(int q_id, bool WAIT) 
// {
//     int rec_val;
//     if(WAIT) {
//         rec_val= msgrcv(q_id, &message, sizeof(message.process),0, !IPC_NOWAIT);
//         printf("---%d\n", rec_val);
//         sleep(2);
//     }else {
//         rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
//         printf("floor\n");
//     }
//     //printf("FROM SCHDULER FILE :: HI:%d \n",message.process->id);
//     //printf("FROM SCHDULER FILE :: shceduler recieved val%d \n",rec_val);

//     if(rec_val!=-1){//new process arrives
//         // if(message == NULL) {
//         //     printf("ok222\n");
//         // }
        
//         printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);                
//         // printf("FROM SCHDULER FILE :: schulder id :%d\n",getpid());
        
//         // Convert the integer to a string
//         char runtime[20]; 
//         sprintf(runtime, "%d", message.process.runtime);
//         char id [20]; 
//         sprintf(id, "%d", message.process.id);
//         int pid=fork();
//         if(pid==0){
//             // printf("FROM SCHDULER FILE :: process id :%d\n",getpid());
//             // execl("./process.out",runtime,message.process.id,NULL);
//         char* processArguments[]={"process.out", runtime, id, NULL};
//         execv(realpath("process.out", NULL),processArguments);
//         }
//         // 
//         struct Process * p=malloc(sizeof(struct Process));
//         p->actual_id=pid;
//         p->runtime=message.process.runtime;
//         p->arrival=message.process.arrival;
//         p->id=message.process.id;
//         p->remaining_time=message.process.runtime;
//         strcpy(p->state,"stopped\0");
//         printf("FROM SCHDULER FILE3 :: schulder id :%d\n",getpid());
//         p->priority=message.process.priority;
//         // 
//         kill(pid,SIGTSTP);
//         printf("FROM SCHDULER FILE4 :: schulder id :%d\n",getpid());
//         message.process.actual_id=pid;
//         message.process.remaining_time=message.process.runtime;
//         pushPQ(&process_queue,p,false);
//         //go into while loop each time send the new remaining time to this process
//     }
// }
void hpf_start(){
        terminated_process=NULL;
        int next_time=getClk();
        running_process=NULL;
        while (running_process!=NULL||!no_more_processes_to_recieve||!isEmpty(&process_queue))
        {
            if((next_time+1)==getClk()){
                printf("next time:%d clk:%d\n",next_time,getClk());
                //new time unit
                //you should update process 
                 next_time++;
                 if(running_process!=NULL){
                    running_process->remaining_time--;
                    printf("remaing time of process:%d is %d\n",running_process->id,running_process->remaining_time);
                    
                 }else{
                    ideal_time++;
                    printf("/////////*************////////////cpu is ideal now \n");
                 }
            }
            int rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
            //printf("FROM SCHDULER FILE :: HI:%d \n",message.process->id);
            //printf("FROM SCHDULER FILE :: shceduler recieved val%d \n",rec_val);
            int f=0;
            if(rec_val!=-1){//new process arrives
                printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);                
                struct Process * p=malloc(sizeof(struct Process));
                p->runtime=message.process.runtime;
                p->arrival=message.process.arrival;
                p->id=message.process.id;
                p->remaining_time=message.process.runtime;
                p->priority=message.process.priority;
                strcpy(p->state,"stopped\0");
                if(allocate_memory(p)){
                    fork_process(p);
                    pushPQ(&process_queue, p,false);
                }else{
                    pushPQ(&waiting_queue,p,true);
                }
            }
            if((running_process && running_process->remaining_time==0) || flag){
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
                        running_process->WTA=running_process->turnaround_time*1.0/running_process->runtime;
                        running_process->waiting_time=running_process->turnaround_time-running_process->runtime;
                        terminated_process=running_process;                        
                    }
                    update_PCB(running_process,f);                            
                }
                running_process=NULL;
                //pick new process
                if(!isEmpty(&process_queue)){
                    flag=false;
                    printf("**********************************");
                    printf("number of processes in the queue:%d",process_queue.size);
                    printf("**********************************\n");
                    running_process= dequeue(&process_queue);
                    strcpy(running_process->state,"resumed");
                    if(running_process->remaining_time==running_process->runtime){
                        strcpy(running_process->state,"started");
                    }
                    update_PCB(running_process,0);
                    kill(running_process->actual_id,SIGCONT);
                }
            }
        }
    total_cpu_time=getClk();
    write_scheduler_perf();
}
/*
Phase2 
*/
void deallocate_memory(int i){
    starter_locations[i]=-starter_locations[i];
    //call allocate new process after that 
    printf("Memory at location:%d is deallocated\n",i);
    while (true)
    {
       //with next equall place 
        if((i-starter_locations[i])<1024&&starter_locations[i-starter_locations[i]]==starter_locations[i]){
        //if two equall sizes (negatives) 
        int s=-2* starter_locations[i];
        //  get the summation==s
        if(i%s==0){
            //if the start location of both of them %s ==0 then concat them 
            starter_locations[i]=-s;
            }
        //with previous 
        }
        else if((i+starter_locations[i])>=0&&starter_locations[i+starter_locations[i]]==starter_locations[i]){
            int s=-2* starter_locations[i];
            if((i+starter_locations[i])%s==0){
            //if the start location of both of them %s ==0 then concat them 
                starter_locations[(i+starter_locations[i])]=-s;
            }
            i=i+starter_locations[i];
        }else 
            return;   
    }
    
    
}
void print_memory(){
    for (int i = 0; i < 1024; i++)
    {
        if(starter_locations[i]!=0){
        fprintf(memory_output_file,"from index:%d size=%d\n at time:%d",i,starter_locations[i],getClk());  

        }
    }
    
}
bool allocate_memory(struct Process * p){
    int x=ceil(log2(p->max_size));
    int actual_size=pow(2,x);
    //printf("Actual Size is%d: vs Reserved Size%d from:%d to %d",p->max_size,actual_size);
    int i=0;
    bool found=false;
    int min_deallocated_index=-1;
    int min_not_allocated_index=-1;
    printf("HI/n");
     while (i<1024)
    {
        bool deallcated_mem=starter_locations[i]<0&&(-starter_locations[i])>=actual_size;
        if(deallcated_mem){
            if(min_deallocated_index==-1){
                min_deallocated_index=i;
            }else
            if(starter_locations[min_deallocated_index]<starter_locations[i]){
                min_deallocated_index=i;
            }
           // printf("deallocated memory at location i:%d=%d\n",i,starter_locations[i]);
        }
        if(abs(starter_locations[i])>0){
            i=i+abs(starter_locations[i]);
        }
        while(starter_locations[i]>=0) i++;
    }
    i=0;
    int j=i+actual_size-1;
    while (j<1024)
    {
        if(starter_locations[i]==0){
        printf("there is memoy not deallocated\n");
        min_not_allocated_index=i;
        break;
        }else{
            printf("there is no memory at location%d and its value%d",i,starter_locations[i]);
            i=(int)fmax(j+1,i+abs(starter_locations[i]));
        }
        
        j=i+actual_size-1;
    }
    if(min_deallocated_index!=-1||min_not_allocated_index!=-1){
        printf("%d and its size:\n",min_deallocated_index);
        printf("%d not deallocated",min_not_allocated_index);
        if(min_not_allocated_index==-1&&min_deallocated_index!=-1||min_deallocated_index!=-1&&min_not_allocated_index!=-1&&((-(starter_locations[min_deallocated_index]))<=(starter_locations[min_not_allocated_index]+actual_size)))
        {
        printf("%d\n",starter_locations[min_deallocated_index]);
            //deallocated mem is smaller than the next availabe index
            int last_index=min_deallocated_index-starter_locations[min_deallocated_index];
            j=min_deallocated_index+actual_size;
            starter_locations[min_deallocated_index]=actual_size;
            p->start_address=min_deallocated_index;
            p->end_address=min_deallocated_index+actual_size-1;
            while (j<last_index)
            {
                starter_locations[j]=-actual_size;
                j+=actual_size;
                actual_size*=2;
            }
        }else{
            starter_locations[min_not_allocated_index]=actual_size;
            p->start_address=min_not_allocated_index;
            p->end_address=min_not_allocated_index+actual_size-1;
        }
        printf("done, Actual Size is%d: vs Reserved Size%d from:%d to %d\n",p->max_size,j-i+1,i,j);
       return true;
    }
    printf("The memory is full\n");
    return false;
    // if(!fff){
    //    printf("HI\n");
    //    fff=1;
    // }
    // if(i>j)
    //     return false;
    // if(memory[i][j]==1){
    //     return false;
    // }
    // //printf("%d\n",p->max_size);
    // if((j-i+1)< p->max_size)
    //     return false;
    // if(allocate_memory(p,i,j/2)){
    //     return true;
    // }
    // if(allocate_memory(p,j/2 +1,j)){
    //     memory[j/2][j]=1;
    //     return true;
    // }
    // p->start_address=i;
    // p->end_address=j;
    // memory[i][j]=1;
    // printf("Actual Size is%d: vs Reserved Size%d from:%d to %d",p->max_size,j-i+1,i,j);
    // return true;

}
void fork_process(struct Process * p){
    char runtime[20]; 
    sprintf(runtime, "%d", p->runtime);
    char id [20]; 
    sprintf(id, "%d", p->id);
    int pid=fork();
    if(pid==0){
        // printf("FROM SCHDULER FILE :: process id :%d\n",getpid());
        // execl("./process.out",runtime,p->id,NULL);
        char* processAgruments[]={"process.out", runtime,id, NULL};
        execv(realpath("process.out", NULL),processAgruments);
    }
    p->actual_id=pid;
    kill(p->actual_id,SIGTSTP);

}
/*
Phase2 
*/
/*

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
                printf("FROM SCHDULER FILE5 :: schulder id :%d\n",getpid());
                p->priority=message.process.priority;
                // enqueue(&process_queue,p);
                pushPQ(&process_queue, p,false);
                kill(pid,SIGTSTP);
*/