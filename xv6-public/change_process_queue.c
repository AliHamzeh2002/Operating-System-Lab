#include "types.h"
#include "user.h"

void set_queue(int pid, int new_queue)
{
    if (pid < 1)
    {
        printf(1, "Invalid pid\n");
        return;
    }
    if (new_queue < 1 || new_queue > 3)
    {
        printf(1, "Invalid queue\n");
        return;
    }
    int res = change_process_queue(pid, new_queue);
    if (res < 0)
        printf(1, "Error changing queue\n");
    else {
        printf(1, "process with pid = %d chnaged queue from %d to %d\n", pid, res, new_queue);
    }
}
int main(int argc,char* argv[]){
    if (argc<3){
        printf(1,"not enough params");
        exit();
    }
    set_queue(atoi(argv[1]),atoi(argv[2]));
    exit();

}