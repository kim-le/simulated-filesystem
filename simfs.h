#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);
void createfile(char *, char *);
void writefile(char *, char *, int start, int length);
void readfile(char *fsname, char *filename, int start, int length);
void deletefile(char *fsname, char *filename);

/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
void getfentries(FILE *fp, fentry *files);
void getfnodes(FILE *fp, fnode *fnodes);
int getfentry(fentry *files, char *filename);
int emptyfentry(fentry *files);
int emptyfnode(fnode *fnodes);
int bytesremaining(int j, fnode *fnodes, char *datablocks);
int nextblock(fnode *fnodes, int j);
int isnumber(char n[]);