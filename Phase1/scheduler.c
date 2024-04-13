#include "headers.h"

int main(int argc, char *argv[]) {
    printf("Scheduler\n");
    printf("algo: %s\n", argv[1]); 
    printf("timeSlice: %s\n", argv[2]); 
    
    initClk();
    
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    destroyClk(true);

    return 0;
}
