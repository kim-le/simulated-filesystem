# Simulated File System
This is a program created in my Software Tools and Systems Programming class at the University of Toronto Mississauga. It simulates a single-directory file system with a limit on number of files and total space. 

The functionality of the program includes the initialization of the file system, the ability to create a file, delete a file, write to a file, and read from a file.

## Installation
Download the zip file on Github or clone the repo:

```bash
git clone https://github.com/kim-le/simulated-filesystem.git
```

## Usage
To start using the simulated file system, run the Makefile:
```bash
make
```
The simulated filesystem is run as:
```bash
simfs -f myfs command args
```
Where the `-f` option specifies the actual file that the simulated file system occupies, `command` is the command to be run on the simulated file system, and a list of arguments for the command is represented by `args`.  
Note that different commands take different numbers of arguments.

## Initialization
To initialize a file system, run the script below, where `myfs` is the desired name of the simulated file system.

```bash
simfs -f myfs initfs
```

## Viewing the file system
To view the contents of the simulated filesystem in a readable form, use the `printfs` command, where `myfs` is the name of your file system.
```bash
simfs -f myfs printfs
```

## Creating a file
To create a simulated file, use the `createfile` command, where `myfs` is the name of your file system and `name` is replaced with the desired file name to be created. 
```bash
simfs -f myfs createfile name
```

## Deleting a file
To delete a simulated file, use the `deletefile` command, where `myfs` is the name of your file system and `name` is replaced with the name of the file to be deleted. 
```bash
simfs -f myfs createfile name
```

## Writing to a file
To write to a simulated file, use the `writefile` command, where `myfs` is the name of your file system, `name` is the name of the simulated file to write to, `start` is the offset into the file to start writing at, and `length` is the length of the chunk of data to write.
```bash
simfs -f myfs writefile name start length
```

## Reading a file
To read a simulated file, use the `readfile` command, where `myfs` is the name of your file system, `name` is the name of the simulated file to read from, `start` is the offset into the file to start reading at, and `length` is the length of the chunk of data to read.
```bash
simfs -f myfs readfile name start length
```
