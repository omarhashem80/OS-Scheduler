#include <math.h>
#include"headers.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include<string.h>

struct Queue process_queue;
struct PriorityQueue_Ali* pq_Ali ;
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
void receive_processes(int q_id, bool WAIT);
void hpf_start();
void strn_start();
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
void images(){
    int pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { // Child process
        // Convert algorithm option and path text to strings
        signal(SIGINT, SIG_IGN);
        execlp("python3", "python3", "converter.py", NULL);
        exit(EXIT_FAILURE);
    }
    wait(NULL);
}
int main(int argc, char *argv[]) {
    
    signal(SIGUSR1, process_terminated);
    signal(SIGUSR2, process_generator_terminated);
    printf("FROM SCHDULER FILE :: Hello from scheduler\n");
    printf("FROM SCHDULER FILE :: Scheduler\n");
    printf("FROM SCHDULER FILE :: algo: %s\n", argv[1]); 
    printf("FROM SCHDULER FILE :: timeSlice: %s\n", argv[2]); 
    outputFilePointer = fopen("./outputs/scheduler.log", "w");
    
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
    if (atoi(argv[1]) == 2) { //HPF
        hpf_start();
        // ---------------------------------------------------------------------------------------------------------
    }else if (atoi(argv[1]) == 0) { //STRN
        strn_start();
    }else if(atoi(argv[1])==1){ //round robin
        rr_start(atoi(argv[2]));
    }
    fclose(outputFilePointer);

    images();
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
        while (running_process!=NULL||!no_more_processes_to_recieve||!isEmpty(&process_queue))
        {  
            if((next_time+1)==getClk()){
                printf("next time:%d clk:%d\n",next_time,getClk());
                //new time unit
                //you should update process 
                next_time++;
                if(running_process!=NULL){
                    running_process->remaining_time--;
                    if(running_process->remaining_time<0){
                       running_process->remaining_time = 0; 
                    }
                    printf("remaing time of process:%d is %d\n",running_process->id,running_process->remaining_time);
                    if(running_process->remaining_time<=0){
                        pick_new_process=quantum;
                    }else
                    pick_new_process++;
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
                printf("FROM SCHDULER FILE1 :: schulder id :%d\n",getpid());
                p->priority=message.process.priority;
                enqueue(&process_queue,p);
                kill(pid,SIGTSTP);
            }

            if(pick_new_process==quantum){
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
    perfFilePointer = fopen("/home/omarhashem/Project/OS-Scheduler/Phase1/outputs/scheduler.perf", "w");
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
    fclose(perfFilePointer);

}
void receive_processes(int q_id, bool WAIT) 
{
    int rec_val;
    if(WAIT) {
        rec_val= msgrcv(q_id, &message, sizeof(message.process),0, !IPC_NOWAIT);
        printf("---%d\n", rec_val);
        sleep(2);
    }else {
        rec_val= msgrcv(q_id, &message, sizeof(message.process),0, IPC_NOWAIT);
        printf("floor\n");
    }
    //printf("FROM SCHDULER FILE :: HI:%d \n",message.process->id);
    //printf("FROM SCHDULER FILE :: shceduler recieved val%d \n",rec_val);

    if(rec_val!=-1){//new process arrives
        // if(message == NULL) {
        //     printf("ok222\n");
        // }
        
        printf("FROM SCHDULER FILE :: new process arrived at:%d\n",message.process.arrival);                
        // printf("FROM SCHDULER FILE :: schulder id :%d\n",getpid());
        
        // Convert the integer to a string
        char runtime[20]; 
        sprintf(runtime, "%d", message.process.runtime);
        char id [20]; 
        sprintf(id, "%d", message.process.id);
        int pid=fork();
        if(pid==0){
            // printf("FROM SCHDULER FILE :: process id :%d\n",getpid());
            // execl("./process.out",runtime,message.process.id,NULL);
        char* processArguments[]={"process.out", runtime, id, NULL};
        execv(realpath("process.out", NULL),processArguments);
        }
        // 
        struct Process * p=malloc(sizeof(struct Process));
        p->actual_id=pid;
        p->runtime=message.process.runtime;
        p->arrival=message.process.arrival;
        p->id=message.process.id;
        p->remaining_time=message.process.runtime;
        strcpy(p->state,"stopped\0");
        printf("FROM SCHDULER FILE3 :: schulder id :%d\n",getpid());
        p->priority=message.process.priority;
        // 
        kill(pid,SIGTSTP);
        printf("FROM SCHDULER FILE4 :: schulder id :%d\n",getpid());
        message.process.actual_id=pid;
        message.process.remaining_time=message.process.runtime;
        pushPQ(&process_queue,p);
        //go into while loop each time send the new remaining time to this process
    }
}
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
                printf("FROM SCHDULER FILE5 :: schulder id :%d\n",getpid());
                p->priority=message.process.priority;
                // enqueue(&process_queue,p);
                pushPQ(&process_queue, p);
                kill(pid,SIGTSTP);
                
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
            }
            if(!running_process && !isEmpty(&process_queue)){
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
    total_cpu_time=getClk();
    write_scheduler_perf();
}
void strn_start() {
    pq_Ali = createPriorityQueue(10000);
    printf("Ali's task begins\n");
    terminated_process = NULL;
    int next_time = getClk();
    running_process = NULL;
    while (running_process != NULL || !no_more_processes_to_recieve || !isEmpty_Ali(pq_Ali))
    {
        if ((next_time + 1) == getClk()) {
            printf("next time:%d clk:%d\n", next_time, getClk());
            //new time unit
            //you should update process 
            next_time++;
            if (running_process != NULL) {
                running_process->remaining_time--;
                 if(running_process->remaining_time<0){
                       running_process->remaining_time = 0; 
                    }
                printf("remaining time of process:%d is %d\n", running_process->id, running_process->remaining_time);
            }
            else {
                ideal_time++;
                printf("/////////*************////////////cpu is idle now \n");
            }
        }

        int rec_val = msgrcv(q_id, &message, sizeof(message.process), 0, IPC_NOWAIT);
        int f = 0;
        if (rec_val != -1) {//new process arrives
            printf("FROM SCHDULER FILE :: new process arrived at:%d\n", message.process.arrival);
            char runtime[20];
            sprintf(runtime, "%d", message.process.runtime);
            char id[20];
            sprintf(id, "%d", message.process.id);
            int pid = fork();
            if (pid == 0) {
                char* processAgruments[] = { "process.out", runtime, id, NULL };
                execv(realpath("process.out", NULL), processAgruments);
                exit(EXIT_FAILURE);
            }
            struct Process* p = malloc(sizeof(struct Process));
            p->actual_id = pid;
            p->runtime = message.process.runtime;
            p->arrival = message.process.arrival;
            p->id = message.process.id;
            p->remaining_time = message.process.runtime;
            strcpy(p->state, "stopped\0");
            printf("FROM SCHEDULER FILE5 :: scheduler id :%d\n", getpid());
            p->priority = message.process.priority;

            // Enqueue the arriving process
            insert(pq_Ali, p);
            kill(pid, SIGTSTP);

            // Check if there's a running process and if the arriving process has a shorter remaining time
            if (running_process != NULL && p->remaining_time < running_process->remaining_time) {
                // Preempt the running process
                strcpy(running_process->state, "preempted");
                update_PCB(running_process, 0);////////////ask
                kill(running_process->actual_id, SIGSTOP);
                // Enqueue the preempted process back into the queue
                insert(pq_Ali, running_process);
                // Set the arriving process as the new running process
                running_process = p;
            }
        }

        // Check for process preemption or completion
        if ((running_process && running_process->remaining_time == 0) || flag) {
            if (running_process != NULL) {

                int f = 0;
                printf("**********************************");
                printf("signal should be passed to %d", running_process->actual_id);
                printf("**********************************\n");
                printf("remaining time should be zero:%d\n", running_process->remaining_time);

                if (running_process->remaining_time == 0) {
                    //todo: wait the process for exit code
                    f = 1;
                    printf("/////////////////////////////////\n");
                    printf("Process should terminate if side\n");
                    printf("/////////////////////////////////\n");

                    strcpy(running_process->state, "finish");

                    running_process->turnaround_time = getClk() - running_process->arrival;
                    //Waiting Time = Turnaround Time - running time
                    running_process->WTA = running_process->turnaround_time * 1.0 / running_process->runtime;
                    running_process->waiting_time = running_process->turnaround_time - running_process->runtime;
                    terminated_process = running_process;
                }
                update_PCB(running_process, f);
            }
            running_process = NULL;

            //pick new process
            if (!isEmpty_Ali(pq_Ali)) {
                flag = false;
                printf("**********************************");
                printf("number of processes in the queue:%d", pq_Ali->size);
                printf("**********************************\n");

                running_process = extractMin(pq_Ali)->data;

                strcpy(running_process->state, "resumed");
                if (running_process->remaining_time == running_process->runtime) {
                    strcpy(running_process->state, "started");
                }

                update_PCB(running_process, 0);
                kill(running_process->actual_id, SIGCONT);
            }
        }
    }

    total_cpu_time = getClk();
    write_scheduler_perf();
    printf("Ali's task done\n");
}