#include "tinybundle.h"

#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#endif
            
            
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

int mkdirp(char *path, int final){
    char partialpath[PATH_MAX];
    int i;
    for(i=0; i < strlen(path)-!final; i++){
        partialpath[i] = path[i];
        partialpath[i+1] = 0;
        if((path[i]==PATHSEP)||(path[i+1]==0)){
            #ifdef _WIN32
            if(_mkdir(partialpath7)<0){
                if(errno!=EEXIST){
                    return -1;
                }
            }
            #else
            if(mkdir(partialpath, 0755)<0){
                if(errno!=EEXIST){
                    return -1;
                }
            }
            #endif

        }
    }
    return 0;
}

int main(int argc, char **argv){
    int n_files;
    char executable[PATH_MAX];
    char *s;
    char outfilename[PATH_MAX];
    char outfile_abspath[PATH_MAX];
    char *basename;
    char tempdir[PATH_MAX];
    FILE *outfile;
    FILE *infile;
    int i;
    int j;
    long filesize;
    int name_length;
    unsigned int checksum;
    
    // possibly linux only:
    int filemode;
    
    // where is the temporary directory?
    // possibly linux only, check TEMP environment variable in Windows:
    char *tmp = P_tmpdir;
    
    // what is the basename of this file?
    // TODO: this is a memory leak
    basename = strdup(argv[0]);
    while((s = strchr(basename, PATHSEP))!=NULL){
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
    sprintf(tempdir, "%s%c%s_%u/", tmp, PATHSEP, basename, checksum);
    if(mkdirp(tempdir, 1)<0){
        fprintf(stderr, "Can't create directory %s: %s\n", tempdir, strerror(errno));
        return 1;
    }
    for(i=0; i<n_files; i++){
    	// TODO abstract these to something that returns a struct
        // read the length of the filename:
        fread(&name_length, sizeof(int), 1, infile);
        // read the filename:
        fread(&outfilename, name_length, 1, infile);
        // linux only
        // read the filemode:
        fread(&filemode, sizeof(int), 1, infile);
        // read the filesize:
        fread(&filesize, sizeof(long), 1, infile);
        
        strcpy(outfile_abspath, tempdir);
        strcat(outfile_abspath, outfilename);
        // create the output file, including parent directories if required:
        if(mkdirp(outfile_abspath, 0)<0){
            fprintf(stderr, "Can't create directories for %s: %s\n", outfile_abspath, strerror(errno));
            return 1;
        }
        outfile = fopen(outfile_abspath, "wb");
        if(outfile==NULL){
            //TODO allow for error where another process has the file: ERROR_SHARING_VIOLATION or ERROR_LOCK_VIOLATION?
            fprintf(stderr, "Can't open %s for writing: %s\n", outfile_abspath, strerror(errno));
            return 1;
        }
        
        // TODO: Replace with fread() and fwrite() in blocks of BLOCK_SIZE.
        for(j=0; j<filesize; j++){
            putc(getc(infile), outfile);
        }
        fclose(outfile);
        
        // TODO: chmod is in sys/stat.h? Better to use umask() to set file creation mode instead of changing it later.
        // if linux:
       // if (chmod(outfile_abspath, filemode) < 0){
       //     fprintf(stderr, "Could not set file permissions on output file %s: %s\n", outfile_abspath, strerror(errno));
       // }
        if(i==0){
            strcpy(executable, outfile_abspath);
        }
    }
    fclose(infile);
    argv[0] = executable;
    
    #ifdef _WIN32
        // TODO system() or ShellExecEx in kernel32
    #else
    	// TODO this should probably return the same return code as the other process
        execv(executable, argv);
    #endif
    
    // TODO I see lots of strings but no free()
    return 0;
}

