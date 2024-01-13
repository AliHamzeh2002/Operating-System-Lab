#include "types.h"
#include "user.h"

int main(int argc, char* argv[]){
    char* shared_mem = open_sharedmem(1);
    char* value = (char*) shared_mem;
    *value = 0;
    for (int i = 0; i < 10; i++){
        if (fork() == 0){
            char* shared_mem = open_sharedmem(1);
            char* value = (char*) shared_mem;
            acquire_user_lock();
            *value += 1;
            printf(1, "Child: %d\n", *value);
            release_user_lock();
            close_sharedmem(1);
            exit();
        }
    }
    while (wait() != -1);
    //sleep(500);
    printf(1, "Parent: %d\n", *value);
    
    close_sharedmem(1);
        
    exit();
}