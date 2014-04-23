#include "tinybundle.h"

#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
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

#ifdef _WIN32
int escape_args(int argc, char **argv, char out[], int outlength){
    // Do escaping of command line arguments for purposes of feeding to Windows' CreateProcess function.
    // Returns 0 on success, -1 if length of output array would be exceeded.
    // Rules are:
    // Is there whitepace (tab, space) anywhere? If so, start with a quote.
    // Backslashes before quote? If so, escape them, and escape the quote.
    // Backslashes at the end? Was there whitespace anywhere? If so, escape the backslashes.
    // Was there whitespace anywhere? If so, end with a quote.
    char BACKSLASH = '\\';
    char SPACE = ' ';
    char TAB = '\t';
    char QUOTE = '"';
    
    int argcounter; // loop over input args
    int incounter; // loop over chars of a single arg
    int outcounter; // loop over the output chararray
    int slashcounter; // loop over accumulated slashes
    
    char *arg;
    char c;
    int whitespace;
    int slashes;
    int arglen;
    
    outcounter = 0;
    for(argcounter=0; argcounter<argc; argcounter++){
        arg = argv[argcounter];
        arglen = strlen(arg);
        whitespace = 0;
        slashes = 0;
        // Whitespace?
        for(incounter=0; incounter<arglen; incounter++){
            c = arg[incounter];
            if((c==SPACE)||(c==TAB)){
                whitespace = 1;
                // Start with a quote:
                if (outcounter >= outlength) return -1;
                out[outcounter] = QUOTE;
                outcounter++;
                break;
            }   
        }
        for(incounter=0; incounter<arglen; incounter++){
            c = arg[incounter];
            // Backslash?
            if(c==BACKSLASH){
                // Wait until we see if a quote follows before writing it:
                slashes++;
            }
            // Quote?
            else if(c==QUOTE){
                // Escape and write any slashes preceding this quote:
                for (slashcounter=0; slashcounter<slashes; slashcounter++){
                    if (outcounter >= outlength-1) return -1;
                    out[outcounter] = BACKSLASH;
                    out[outcounter+1] = BACKSLASH;
                    outcounter += 2;
                }
                slashes = 0;
                // Escape and write the quote:
                if (outcounter >= outlength-1) return -1;
                out[outcounter] = BACKSLASH;
                out[outcounter+1] = QUOTE;
                outcounter += 2;
            }
            // Other character?
            else{
                // Write any slashes preceding this character:
                for(slashcounter=0; slashcounter<slashes; slashcounter++){
                    if (outcounter >= outlength) return -1;
                    out[outcounter] = BACKSLASH;
                    outcounter++;
                }
                slashes = 0;
                // Write the character:
                if (outcounter >= outlength) return -1;
                out[outcounter] = c;
                outcounter++;
            }
        }
        // End of string. Was there whitespace?
        if (whitespace){
            // We'll be ending in a quote mark, so escape and write any preceding slashes: 
            for(slashcounter=0; slashcounter<slashes; slashcounter++){
                if (outcounter >= outlength-1) return -1;
                out[outcounter] = BACKSLASH;
                out[outcounter+1] = BACKSLASH;
                outcounter += 2;
            }
            // End with a quote mark:
            if (outcounter >= outlength) return -1;
            out[outcounter] = QUOTE;
            outcounter++;
        }
        else{
            // Won't be ending with a quote mark, so write slashes without escaping them:
            for(slashcounter=0; slashcounter<slashes; slashcounter++){
                if (outcounter >= outlength) return -1;
                out[outcounter] = BACKSLASH;
                outcounter++;
            }
        }
        // Space before the next arg:
        if (outcounter >= outlength) return -1;
        out[outcounter] = SPACE;  
        outcounter++ ;
    }
    // Null terminator
    if (outcounter >= outlength) return -1;
    out[outcounter] = 0;
    return 0;
}
#endif

int mkdirp(char *path, int final){
    char partialpath[PATH_MAX];
    int i;
    for(i=0; i < strlen(path)-!final; i++){
        partialpath[i] = path[i];
        partialpath[i+1] = 0;
        if((path[i]==PATHSEP)||(path[i+1]==0)){
            #ifdef _WIN32
            if(_mkdir(partialpath)<0){
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
    char basename[PATH_MAX];
    char tempdir[PATH_MAX];
    FILE *outfile;
    FILE *infile;
    int i;
    int j;
    long filesize;
    int name_length;
    unsigned int checksum;
    char copybuffer[BLOCK_SIZE];
    
    #ifdef _WIN32
    char *tmp = getenv("TEMP");
    char command_string[CMD_MAX];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int retcode;
    #else
    int filemode;
    char *tmp = P_tmpdir;
    # endif
    
    // what is the basename of this file?
    strcpy(basename, argv[0]);
    while((s = strchr(basename, PATHSEP))!=NULL){
        strcpy(basename, s+1);
    }
    
    #ifdef _WIN32
    // Strip the .exe extension off the end by replacing the last dot with a null char:
    if((s = strchr(basename, '.'))!=NULL){
        fprintf(stderr, "s is %s\n", s);
        if((strcmp(s, ".exe")==0)||(strcmp(s, ".EXE")==0)){
            fprintf(stderr, "s is %s\n", s);
            s[0] = 0;
        }
    }
    #endif

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
        // read the length of the filename:
        fread(&name_length, sizeof(int), 1, infile);
        // read the filename:
        fread(&outfilename, name_length, 1, infile);
        #ifndef _WIN32
        // read the filemode:
        fread(&filemode, sizeof(int), 1, infile);
        #endif
        // read the filesize:
        fread(&filesize, sizeof(long), 1, infile);
        
        // create the output file, including parent directories if required:
        strcpy(outfile_abspath, tempdir);
        if (strlen(outfilename) + strlen(tempdir) > PATH_MAX){
            fprintf(stderr, "Destination filepath %s%s is too long (>%d chars):", tempdir, outfilename, PATH_MAX-1);
            return 1;
        }
        strcat(outfile_abspath, outfilename);
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
        
        // Copy the data to the output file:
        for(j=0; j<filesize/BLOCK_SIZE; j++){
            fread(&copybuffer, sizeof(char), BLOCK_SIZE, infile);
            fwrite(&copybuffer, sizeof(char), BLOCK_SIZE, outfile);
        }
        fread(&copybuffer, sizeof(char), filesize % BLOCK_SIZE, infile);
        fwrite(&copybuffer, sizeof(char), filesize % BLOCK_SIZE, outfile);
        
        // We're done with the output file:    
        fclose(outfile);
        
        #ifndef _WIN32
        if (chmod(outfile_abspath, filemode) < 0){
           fprintf(stderr, "Could not set file permissions on output file %s: %s\n", outfile_abspath, strerror(errno));
        }
        #endif
        if(i==0){
            // First file. Remember this filepath - it is the executable we want to run when we're done unpacking.
            strcpy(executable, outfile_abspath);
        }
    }
    fclose(infile);
    // Set argv[0] so the target process sees its own name there, rather than ours:
    argv[0] = executable;
        
    #ifdef _WIN32
    escape_args(argc, argv, command_string, CMD_MAX);
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    if(CreateProcess(NULL, command_string, NULL, NULL, 1, 0, NULL, NULL, &si, &pi)!=0){
        fprintf(stderr, "Can't execute %s: %s\n", executable, strerror(errno)); // Possibly GetLastError() instead of errno
    }
    // Wait for process to end:
    WaitForSingleObject(pi.hProcess, INFINITE);
    retcode = GetExitCodeProcess(pi);
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return retcode;
    #else
    if (execv(executable, argv)<0){
        fprintf(stderr, "Can't execute %s: %s\n", executable, strerror(errno));
        return 1;
    }
    #endif
    // should not get up to here:
    return 1;
}


