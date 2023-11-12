#include "types.h"
#include "user.h"

int main(int argc, char* argv[]){

    if (fork() == 0){
        sleep(1000);
        printf(1, "Child Process lifetime: %d\n", get_process_lifetime(getpid()));
    }
    
    else {
        wait();
        sleep(200);
        printf(1, "Parent Process lifetime: %d\n", get_process_lifetime(getpid()));

    }
    exit();   
}