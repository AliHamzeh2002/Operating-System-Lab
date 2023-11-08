#include "types.h"
#include "user.h"

int 
find_digital_root_syscall(int num){
    int prev_ebx;

    asm volatile(
        "movl %%ebx, %0\n\t"
        "movl %1, %%ebx"
        : "=r"(prev_ebx)
        : "r"(num)
    );

    int result = find_digital_root();

    asm volatile(
        "movl %0, %%ebx"
        :: "r"(prev_ebx)
    );
    
    return result;
}

int
main(int argc, char* argv[]){
    if (argc < 2) {
        printf(1, "Usage: find_digital_root <num>\n");
        exit();
    }
    int input = atoi(argv[1]);
    int result = find_digital_root_syscall(input);
    if (result < 0){
        printf(2, "number should be positive\n");
        exit();
    }
    printf(1, "The digital root of %d is %d\n", input, result);
    exit();
}