#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

void createfile(char *fsname, char *filename) {
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    FILE *fp = openfs(fsname, "w");

    getfentries(fp, files);
    getfnodes(fp, fnodes);
    int i = findfentry(files);
    int j = findfnode(fnodes);
    fentry newfile = files[i];
    
    fnode newnode = fnodes[j];
    for (int index = 0; index < MAXFILES; index++){
        if (files[index].name == filename){
            fprintf(stderr, "Error: file name already exists\n");
            closefs(fp);
            exit(1);
        }
    }
    strncpy(newfile.name, filename, 11);
    newnode.blockindex *= -1;
    newfile.firstblock=newnode.blockindex;
    files[i] = newfile;
    fnodes[j] = newnode;
    fwrite(files, sizeof(fentry), MAXFILES, fp);
    fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp);

    closefs(fp);
}
    