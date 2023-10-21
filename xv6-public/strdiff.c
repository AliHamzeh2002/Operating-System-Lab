#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(1, "Usage: strdiff <word1> <word2>\n");
        exit();
    }
    char *word1 = argv[1];
    char *word2 = argv[2];
    char result[16]; 
    memset(result, '\0', 16); 
    int i;
    int len1 = strlen(word1);
    int len2 = strlen(word2);
    int maxLength = len1 > len2 ? len1 : len2;

    for (i = len1; i < maxLength; i++) {
        if (i < len1 && word1[i] >= 'A' && word1[i] <= 'Z') {
            word1[i] = word1[i] + 'a' - 'A';
        }
        else if(i >= len1)
            word1[i] = ' ';
    }
    for (i = len2; i < maxLength; i++) {
        if (i < len2 && word2[i] >= 'A' && word2[i] <= 'Z') {
            word2[i] = word2[i] + 'a' - 'A';
        }
        else if(i >= len2)
            word2[i] = ' ';
    }
    for (i = 0; i < maxLength; i++) {
        
        if (word1[i] >= word2[i]) {
            result[i] = '0';
        }
        else {
            result[i] = '1';
        }
    }
    unlink("result_strdiff.txt");
    int fd=open("result_strdiff.txt",O_CREATE|O_WRONLY);

    if (fd<0) {
        printf(1,"result_strdiff.:cannot create result_strdiff.txt\n");
        exit();
    }
   write(fd,result,maxLength);
   write(fd,"\n",1);
   close(fd);
   exit();


}
//if(word1[i])