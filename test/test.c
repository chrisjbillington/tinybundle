#include <stdio.h>
#include <string.h>
#include <errno.h>

// If linux:
#define pathsep  '/'

#define test_file  "file_1.txt"

int main(int argc, char **argv){
    char filepath[1024] = {0};
    char filecontents[1024] = {0};
    FILE *f;    
    int i;
    int foundlastslash = 0;
    char ch;
    
    fprintf(stdout, "Hello, world!\n");
    fprintf(stdout, "This is %s\n", argv[0]);
    
    for(i=strlen(argv[0])-1; i>=0; i--){
        if((!foundlastslash)&&(argv[0][i]==pathsep)){
            foundlastslash = 1;
            filepath[i+1] = 0;
        }
        if(foundlastslash){
            filepath[i] = argv[0][i];
        }
    }
    strcat(filepath, test_file);
    f = fopen(filepath, "r");
    if(f==NULL){
        fprintf(stderr, "Can't open %s for reading: %s\n", filepath, strerror(errno));
        return 1;
    }
    i = 0;
    while((ch = getc(f))!=EOF){
        filecontents[i] = ch;
        i++;
    }
    filecontents[i] = 0;
    fprintf(stdout, "The contents of %s are:\n%s\n", filepath, filecontents);
    return 0;
}
