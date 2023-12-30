#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

#define NUM_FORKS 10

void intToStr(int num, char* buffer) {
    int i = 0;
    do {
        buffer[i++] = num % 10 + '0';
        num /= 10;
    } while (num > 0);
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;

        start++;
        end--;
    }
    buffer[i] = '\0';
}


int main(int argc, char* argv[]){
    int fd=open("lock-test.txt",O_CREATE|O_WRONLY);
    for (int i = 0; i < NUM_FORKS; i++){
        int pid = fork();
        if (pid == 0){

            acquire_user_lock();
            print_queue();
            printf(1, "pid lock: %d\n", getpid());


            char* write_data = "pid: ";
            int max_length = strlen(write_data);
            write(fd,write_data,max_length);
            char pid_str[2];
            intToStr(getpid(), pid_str);
            write(fd, pid_str, strlen(pid_str));
            write(fd,"\n",1);
            sleep(50);

            release_user_lock();
            exit();
        }
        //while(1);
    }
    
    while (wait() != -1);
    close(fd);

    exit();
}
