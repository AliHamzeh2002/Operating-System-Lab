#include "types.h"
#include "user.h"


#define NUM_FORKS 10
int main(int argc, char* argv[]){
    for (int i = 0; i < NUM_FORKS; i++){
        int pid = fork();
        if (pid == 0){
           sleep(1000000);
           int x = 0;
           for (long long j = 0; j < 1000000000000; j++){
                for (long long k = 0; k < 10000000000000; k++){
                    x += k * 12 - j;
                }
            }
            break;
        }
        //while(1);
    }
    while (wait() != -1);
    exit();
}
