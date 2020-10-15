/* This file contains functions that are not part of the visible "interface".
 * They are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

/* Internal helper functions first.
 */

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if((fp = fopen(filename, mode)) == NULL) {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void
closefs(FILE *fp)
{
    if(fclose(fp) != 0) {
        perror("closefs");
        exit(1);
    }
}

/* File system operations: creating, deleting, reading, and writing to files.
 */

void createfile(char *fsname, char *filename) {
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    FILE *fp = openfs(fsname, "r+");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }
    for (int index = 0; index < MAXFILES; index++){
        if (strcmp(files[index].name, filename) == 0){
            fprintf(stderr, "Error: file name already exists\n");
            closefs(fp);
            exit(1);
        }
    }
    int i = emptyfentry(files);
    int j = emptyfnode(fnodes);
    if (i == -1 || j == -1){
        fprintf(stderr, "Error: not enough space in file system\n");
        closefs(fp);
        exit(1);
    }
    fentry newfile = files[i];
    fnode newnode = fnodes[j];
    
    strncpy(newfile.name, filename, 11);
    newnode.blockindex *= -1;
    newfile.firstblock=newnode.blockindex;

    files[i] = newfile;
    fnodes[j] = newnode;

    fseek(fp, 0, SEEK_SET);
    fwrite(files, sizeof(fentry), MAXFILES, fp);
    fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp);

    closefs(fp);
}

void writefile(char *fsname, char *filename, int start, int length){
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    char datablocks[MAXBLOCKS * BLOCKSIZE] = {0};
    char block[sizeof(char)];

    FILE *fp = openfs(fsname, "r+");
    int d = 0;


    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    for (int i=0; i<MAXBLOCKS * BLOCKSIZE; i++){
        if((fread(block, 1, 1, fp)) == 1) {
            datablocks[d] = *block;
            d++;
        }
    }
    //if there is more data, then error
    if (fread(block, 1, 1, fp) == 1) {
        fprintf(stderr, "Error: could not read datablocks\n");
        closefs(fp);
        exit(1);
    }
    int f = getfentry(files, filename);
    if (f == -1){
        fprintf(stderr, "Error: file does not exist\n");
        closefs(fp);
        exit(1);
    }
    int bytes_remaining = length;
    int j = files[f].firstblock;
    int offset = BLOCKSIZE * j + start;
    if (bytesremaining(j, fnodes, datablocks) < (length + start)){
        fprintf(stderr, "Error: not enough space to write file\n");
        closefs(fp);
        exit(1);
    }
    
    if (files[f].size < start){
        fprintf(stderr, "Error: write start position is larger than file size\n");
        closefs(fp);
        exit(1);
    }     

    if (ferror(fp)) {
        fprintf(stderr, "Error: could not read data block\n");
        closefs(fp);
        exit(1);
    }

    if (start == files[f].size){
        files[f].size += length;
    } else if (start < files[f].size && (start + length) > files[f].size){
        files[f].size = start + length;
    }

    // if the start location is > 128
    while ((BLOCKSIZE * j + start) >= ((j + 1) * BLOCKSIZE)){
        j = nextblock(fnodes, j);
        start -= BLOCKSIZE;
        offset = BLOCKSIZE * j + start;
    }

    int bytes_available = BLOCKSIZE - start;
    
    //actually writing it now
    // if there is more space in data block than bytes to be written
    if (bytes_available > bytes_remaining){ 
        int i = 0;
        while (bytes_remaining > 0){
            fread(block, sizeof(char), 1, stdin);
            datablocks[offset + i] = *block;
            i++;
            bytes_remaining--;
        }
    } 
    // if there are more bytes to be written than bytes available in data block
    else {
        while (bytes_remaining > 0){
            for (int i = 0; i < bytes_available; i++){
                fread(block, sizeof(char), 1, stdin);
                datablocks[offset + i] = *block;
                bytes_remaining--;
                if (bytes_remaining == 0){
                    i = bytes_available;
                }
            }
            if (bytes_remaining != 0){
                j = nextblock(fnodes, j);
                bytes_available = BLOCKSIZE;
                offset = BLOCKSIZE * j;
            }
        }
    }

    fseek(fp, 0, SEEK_SET);
    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(datablocks, sizeof(datablocks), 1, fp) < 1) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }
    
    closefs(fp);

}

void readfile(char *fsname, char *filename, int start, int length){
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    char datablocks[MAXBLOCKS * BLOCKSIZE] = {0};
    char block[sizeof(char)];

    FILE *fp = openfs(fsname, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    int d = 0;
    while((fread(block, 1, 1, fp)) != 0) {
        datablocks[d] = *block;
        d++;
    }
    
    int f = getfentry(files, filename);
    if (f == -1){
        fprintf(stderr, "Error: file does not exist\n");
        closefs(fp);
        exit(1);
    }
    
    if (length > files[f].size){
        fprintf(stderr, "Error: not enough data to read from file for given length\n");
        closefs(fp);
        exit(1);
    }

    if (files[f].size < start){
        fprintf(stderr, "Error: start position is larger than file size\n");
        closefs(fp);
        exit(1);
    }     
    int bytes_remaining = length;
    int j = files[f].firstblock;
    int offset = BLOCKSIZE * j + start;

    // if the start location is > 128
    while ((BLOCKSIZE * j + start) >= ((j + 1) * BLOCKSIZE)){
        j = nextblock(fnodes, j);
        start -= BLOCKSIZE;
        offset = BLOCKSIZE * j + start;
    }

    int bytes_available = BLOCKSIZE - start;

    //actually reading it now
    // if there is more space in data block than bytes to be read
    if (bytes_available > bytes_remaining){ 
        int i = 0;
        while (bytes_remaining > 0){
            fprintf(stdout, "%c", datablocks[offset + i]);
            i++;
            bytes_remaining--;
        }
    } 
    // if there are more bytes to be read than bytes available in data block
    else {
        while (bytes_remaining > 0){
            for (int i = 0; i < bytes_available; i++){
                fprintf(stdout, "%c", datablocks[offset + i]);
                bytes_remaining--;
                if (bytes_remaining == 0){
                    i = bytes_available;
                }
            }
            if (bytes_remaining != 0){
                j = nextblock(fnodes, j);
                bytes_available = BLOCKSIZE;
                offset = BLOCKSIZE * j;
            }
        }
    }
    closefs(fp);
}

void deletefile(char *fsname, char *filename){
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    char datablocks[MAXBLOCKS * BLOCKSIZE] = {0};
    char block[sizeof(char)];
    int d = 0;

    FILE *fp = openfs(fsname, "r+");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    while((fread(block, 1, 1, fp)) != 0) {
        datablocks[d] = *block;
        d++;
    }
    int f = getfentry(files, filename);
    if (f == -1){
        fprintf(stderr, "Error: file does not exist\n");
        closefs(fp);
        exit(1);
    }
    int j = files[f].firstblock;
    int offset = BLOCKSIZE * j;
    while (fnodes[j].nextblock != -1){
        for (int i = 0; i < BLOCKSIZE; i++){
            datablocks[offset+i] =(char)0;
        }
        fnodes[j].blockindex *= -1;
        int k = fnodes[j].nextblock;
        fnodes[j].nextblock = -1;
        j = k;
        offset = BLOCKSIZE * j;
    }
    if (fnodes[j].nextblock == -1){
        for (int i = 0; i < BLOCKSIZE; i++){
            datablocks[offset+i] = (char)0;
        }
        fnodes[j].blockindex *= -1;
    }

    files[f].name[0] = '\0';
    files[f].size = 0;
    files[f].firstblock = -1;


    fseek(fp, 0, SEEK_SET);
    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(datablocks, sizeof(datablocks), 1, fp) < 1) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }
    closefs(fp);
}

//read fentries
void getfentries(FILE *fp, fentry *files){
    fread(files, sizeof(fentry), MAXFILES, fp);
}

void getfnodes(FILE *fp, fnode *fnodes){
    fread(fnodes, sizeof(fnode), MAXBLOCKS, fp);
}

int getfentry(fentry *files, char *filename){
    for (int i = 0; i < MAXFILES; i++){
        if (strcmp(files[i].name, filename) == 0){
            return i;
        }
    }
    return -1;
}

// find first open fentry and fnode
int emptyfentry(fentry *files){
    for (int i = 0; i < MAXFILES; i++) {
        if (strcmp(files[i].name, "") == 0 && (files[i]).size == (short)0 && files[i].firstblock == -1){
            return i;
        }
    }
    return -1;
}

int emptyfnode(fnode *fnodes){
    for (int i = 0; i < MAXBLOCKS; i++) {
        if (fnodes[i].blockindex < 0 && fnodes[i].nextblock == -1){
            return i;
        }
    }
    return -1;
}

// number of data blocks left
int bytesremaining(int j, fnode *fnodes, char *datablocks){
    int blocks_remaining = 0;
    // just free space
    for (int i = 0; i < MAXBLOCKS; i++) {
        if (fnodes[i].blockindex < 0){
            blocks_remaining++;
        }
    }
    // blocks owned
    while (fnodes[j].nextblock != -1){
        blocks_remaining++;
        j++;
    }
    if (fnodes[j].nextblock == -1){
        blocks_remaining++;
    }
    return blocks_remaining * BLOCKSIZE;
}

int nextblock(fnode *fnodes, int j){
    if (fnodes[j].nextblock != -1){
        return fnodes[j].nextblock;
    } else{
        int k = emptyfnode(fnodes);
        fnodes[j].nextblock = k;
        fnodes[k].blockindex *= -1;
        return k;
    }
}

int isnumber(char n[]) {
    int i;
    for (i = 0; i < strlen(n); i++) {
        if (n[i] > '9' || n[i] < '0'){
            return 0;
        }
    }
    return 1;
}

// Signatures omitted; design as you wish.