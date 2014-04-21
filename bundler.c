#include <stdio.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h>
#include <errno.h>

#define BOOTSTRAPPER_SIZE 15000


// output format:
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
    int n_infiles;
    char *outfilename;
    FILE *outfile;
    FILE *thisfile;
    char *infilename;
    FILE *infile;
    char ch;
    int i;
    int filesize;
    int name_length;
    unsigned int checksum;
    unsigned int readbuffer;
    
    // possibly linux only:
    int filemode;
    int output_filemode;
    struct stat stat_buf;
    
    // get number of input files and open the output file for writing:
    n_infiles = argc - 2;
    outfilename = argv[argc-1];
    
    // Open the output file for writing:
    outfile = fopen(outfilename, "wb");
    if(outfile==NULL){
        fprintf(stderr, "Can't open output file %s for writing: %s\n", outfilename, strerror(errno));
        return 1;
    }    
    
    // have the executable open itself for reading:
    thisfile = fopen(argv[0], "rb");
    if(thisfile==NULL){
        fprintf(stderr, "Can't open %s for reading: %s\n", argv[0], strerror(errno));
        return 1;
    }   
    
    // write the bootstrapper from the executable to the output file:
    fseek(thisfile, -BOOTSTRAPPER_SIZE, SEEK_END);
    for(i=0; i<BOOTSTRAPPER_SIZE; i++){
        ch=getc(thisfile);
        putc(ch, outfile);
    }
    
    // close executable file:
    fclose(thisfile);
    
    // write zeros where the payload checksum will eventually be.
    // We will come back and write after we have computed it:
    checksum = 0;
    fwrite(&checksum, sizeof(unsigned int), 1, outfile);
    
    // write the number of input files to the output file:
    fwrite(&n_infiles, sizeof(int), 1, outfile);
    
    // write the input files to the output file, one by one:
    for (i = 1; i < n_infiles+1; i++){
        infilename = argv[i];
        // write the length of the input file's filename to the output file:
        name_length = strlen(infilename);
        fwrite(&name_length, sizeof(int), 1, outfile);
        
        // write the input file's filename to the output file:
        fprintf(outfile, "%s", infilename);
        
        // possibly linux only:
        // write the file mode to the file:
        stat(argv[i], &stat_buf);
        filemode = stat_buf.st_mode;
        fwrite(&filemode, sizeof(int), 1, outfile);
        if(i==1){
            // this is the first file, record its mode so we 
            // can set the mode of the output file the same:
            output_filemode = filemode;
        }
        
        // open the input file for reading:
        infile = fopen(infilename, "rb");
        if(infile==NULL){
            fprintf(stderr, "Can't open input file %s for reading: %s\n", infilename, strerror(errno));
            return 1;
        }
        
        // write the input file's size to the output file:
        fseek(infile, 0, SEEK_END);
        filesize = ftell(infile);
        fseek(infile, 0, SEEK_SET);
        fwrite(&filesize, sizeof(int), 1, outfile);
        
        // write the contents of the input file to the output file:
        while((ch=getc(infile))!=EOF){
            putc(ch, outfile);
        }
        // close the input file:
        fclose(infile);
    }
    // close the output file:
    fclose(outfile);
    
    // reopen the output file in rw mode, so we can compute the checksum:
    outfile = fopen(outfilename, "r+b");
    if(outfile==NULL){
        fprintf(stderr, "Can't open output file %s in read/write mode: %s\n", outfilename, strerror(errno));
        return 1;
    }
    // compute the checksum of the payload, chunking by the size of an int:
    fseek(outfile, BOOTSTRAPPER_SIZE + sizeof(unsigned int), SEEK_SET);
    while(fread(&readbuffer, sizeof(unsigned int), 1, outfile)){
        checksum += readbuffer;
    }
    
    // Write the checksum to the output file:
    fseek(outfile, BOOTSTRAPPER_SIZE, SEEK_SET);
    fwrite(&checksum, sizeof(unsigned int), 1, outfile);
    
    // close the output file:
    fclose(outfile);
    
    // if linux:
    if (chmod(outfilename, output_filemode) < 0){
        fprintf(stderr, "Could not set file permissions on output file %s: %s\n", outfilename, strerror(errno));
    }
    
    return 0;
}

