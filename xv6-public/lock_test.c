#include "types.h"
#include "user.h"
#define NUM_FORKS 10
int x;
int main(int argc, char* argv[]){
    x = 0;
    for (int i = 0; i < NUM_FORKS; i++){
        int pid = fork();
        if (pid == 0){
           //printf(1, "pid : %d\n", getpid());
           
           acquire_user_lock();
           print_queue();
           printf(1, "pid lock: %d\n", getpid());
           sleep(100);
           for (long long j = 0; j < 100; j++){
                x++;
            }
            //printf(1, "pid lock: %d\n", getpid());
            release_user_lock();
           // printf(1, "x : %d\n", x);
            exit();
        }
        //while(1);
    }
    
    while (wait() != -1);
    printf(1, "x : %d\n", x);
    exit();
}
