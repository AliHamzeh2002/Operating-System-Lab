#include "types.h"
#include "user.h"

int main(int argc, char* argv[]){
    if (argc < 3){
        printf(1, "Usage: copy_file <src> <dst>\n");
        exit();
    }
    char* src = argv[1];
    char* dest = argv[2];
    if (copy_file(src, dest) < 0){
        printf(1, "copy_file: failed to copy %s to %s\n", src, dest);
        exit();
    }
    printf(1, "copy_file: successfully copied %s to %s\n", src, dest);
    exit();
}