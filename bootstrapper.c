#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BOOTSTRAPPER_SIZE 15000

// input format:
// Bootstrapper - BOOTSTRAPPER_SIZE
// checksum - int
// Number of files - int
// for each file:
//   size of filename - int
//   filename - size of filename
//   if linux:
//     filemode - int
//   size of file - int
//   file - size of file

int main(int argc, char **argv){
    int n_files;
    char *outfilename;
    char *basename;
    char tempdir[1024] = {0};
    FILE *outfile;
    FILE *infile;
    char ch;
    int i;
    int filesize;
    int name_length;
    unsigned int checksum;
    
    // possibly linux only:
    int filemode;
    int output_filemode;
    
    // where is the temporary directory?
    // possibly linux only, check TEMP environment variable in Windows:
    char *tmp = P_tmpdir;
    char pathsep = '/';
    
    // what is the basename of this file?
    char *s = strchr(argv[0], pathsep);
    if(s==NULL){
        basename = strdup(argv[0]);
    }
    else{
        basename = strdup(s+1);
    }
    
    // have the executable open itself for reading:
    infile = fopen(argv[0], "rb");
    if(infile==NULL){
        fprintf(stderr, "Can't open %s for reading: %s\n", argv[0], strerror(errno));
        return 1;
    }
    
    // seek to where the payload starts:
    fseek(infile, BOOTSTRAPPER_SIZE, SEEK_SET);
    
    // read the payload checksum:
    fread(&checksum, sizeof(unsigned int), 1, infile);
    // read the number of files:
    fread(&n_files, sizeof(int), 1, infile);
    
    // make the temporary folder we'll be using:
    // possibly linux only, snprintf might not exist on windows - maybe use sprintf and risk segfault instead:
    snprintf(tempdir, 1023, "%s%c%s_%u", tmp, pathsep, basename, checksum);
    // _mkdir(tempdir); on windows
    if(mkdir(tempdir, 0777)<0){
        if(errno!=EEXIST){
            fprintf(stderr, "Can't create directory %s: %s\n", tempdir, strerror(errno));
            return 1;
        }
    }
    
    return 0;
}

