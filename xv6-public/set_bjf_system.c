#include "types.h"
#include "user.h"

void set_system_bjf(int priority_ratio, int arrival_time_ratio, int executed_cycle_ratio)
{
    if (priority_ratio < 0 || arrival_time_ratio < 0 || executed_cycle_ratio < 0)
    {
        printf(1, "Invalid ratios\n");
        return;
    }
    int res = set_bjf_system(priority_ratio, arrival_time_ratio, executed_cycle_ratio);
    if (res < 0)
        printf(1, "Error setting BJF params\n");
    else
        printf(1, "BJF params set successfully\n");
}

int main(int argc,char* argv[]){
   if (argc < 4)
        {
            printf(1,"not enough params");
            exit();
        }
        set_system_bjf(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
}