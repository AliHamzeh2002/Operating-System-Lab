#include "types.h"
#include "user.h"

int main(int argc, char* argv[]){
    if (fork() == 0){
        sleep(10);
    }
    else if (fork() == 0){
        sleep(10);
    }
    else if (fork() == 0){
        if (fork() == 0){
            printf(1, "Uncle count: %d\n", get_uncle_count(getpid()));
        }
        else{
            wait();
        }
        
    }
    
    else {
        wait();
        wait();
        wait();
    }
    exit();   
}