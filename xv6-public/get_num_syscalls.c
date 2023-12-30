#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

#define NUM_FORKS 3
int main(int argc, char* argv[]){
    int fd=open("shared_file.txt",O_CREATE|O_WRONLY);
    for (int i = 0; i < NUM_FORKS; i++){
        int pid = fork();
        if (pid == 0){
            acquire_user_lock();
            
            char* write_data = "salam";
            int max_length = 5;

            write(fd,write_data,max_length);
            write(fd,"\n",1);
            
            release_user_lock();
            exit();
            
        }
        //while(1);
    }
    
    while (wait() != -1);
    close(fd);


    get_num_syscalls();
    exit();
}
