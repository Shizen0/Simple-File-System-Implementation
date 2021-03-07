#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const int REG_FILE = 0;
const int DIRECTORY = 1;
const int DATA_BLOCK_SIZE = 512;

FILE *sfs;
int currentDirectory = 0;

void init_fileSystem();

void simpleBash();
void mkdir();
void mkfile();
void cd();
void ls();
void lsrec();
void help();

struct super_block {
	int inode_bitmap;
	int data_bitmap[10];
};

struct inode_st {
    int type;
    int size;
    int data_block_indices[10];
};

struct dir_ent {
  char name [28];  
  unsigned int inode_no;
};

int getInodeNo(struct super_block sb);
int getDataBlockNo(struct super_block sb);
struct inode_st getCurrentInode();

struct super_block sb;

int main() {	
	sfs = fopen("D:\\Shizen\\Desktop\\FileSystem\\sfs.bin", "w+");	
	init_fileSystem();
	
	simpleBash();
	
	return 0;
}

void simpleBash() {
	printf("Write 'help' to see the possible commands.\n");
	char command[32];
	while(1) {
		printf("> ");
		scanf("%s", command);		
		if(strcmp(command, "mkdir") == 0) {
			bool fileExists = false;
			char fileName[32];
			scanf("%s", fileName);			
    		struct inode_st tempInode = getCurrentInode();  		
    		
    		int numOfEntries = tempInode.size/sizeof(struct dir_ent);
			fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st)+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
			
			struct dir_ent tempEntry;
			int i;
			for(i=0; i<numOfEntries; i++){
				fread(&tempEntry, sizeof(struct dir_ent), 1, sfs);
				if(strcmp(tempEntry.name, fileName) == 0) {
					printf("File with the same name already exists!\n");
					fileExists = true;
					break;
				}				
			}
			if(!fileExists) {
				mkdir(fileName);
			}							
		}
		else if(strcmp(command, "mkfile") == 0) {
			bool fileExists = false;
			char fileName[32];
			scanf("%s", fileName);			
			  
    		struct inode_st tempInode = getCurrentInode();  		
    		
    		int numOfEntries = tempInode.size/sizeof(struct dir_ent);
			fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st)+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
	
			struct dir_ent tempEntry;
			int i;
			for(i=0; i<numOfEntries; i++){
				fread(&tempEntry, sizeof(struct dir_ent), 1, sfs);
				if(strcmp(tempEntry.name, fileName) == 0) {
					printf("File with the same name already exists!\n");
					fileExists = true;
					break;
				}				
			}
			if(!fileExists) {
				mkfile(fileName);
			}
		}
		else if(strcmp(command, "cd") == 0) {
			cd();
		}
		else if(strcmp(command, "ls") == 0) {
			ls();
		}
		else if(strcmp(command, "lsrec") == 0) {
			int tab = 0;
			lsrec(tab, 0);
		}
		else if(strcmp(command, "help") == 0) {	
			help();
		}
		else if(strcmp(command, "exit") == 0){
			fclose(sfs);
			break;
		}
		else {
			printf("Unvalid command!\n");
		}
	}
}

void mkdir(char *fileName) {	
	struct inode_st newDir;
	newDir.type = DIRECTORY;
	newDir.size = 0;
	
	int inode = getInodeNo(sb); 
	int datablock = getDataBlockNo(sb);
	
	int i;
	for(i=0; i<10; i++)
        newDir.data_block_indices[i] = datablock;
        
    struct dir_ent dot;
	strcpy(dot.name,".");
	dot.inode_no = inode;
	
	struct dir_ent dotdot;
	strcpy(dotdot.name,"..");
	dotdot.inode_no = currentDirectory;
	
	newDir.size = sizeof(struct dir_ent)*2;
	
	int mask = 1 << inode; 
    sb.inode_bitmap = (sb.inode_bitmap & ~mask) | ((1 << inode) & mask); 
	
	int index = datablock/32;
	int p = datablock%32;	
	mask = 1 << p; 
    sb.data_bitmap[index] = (sb.data_bitmap[index] & ~mask) | ((1 << p) & mask);
    
    fseek(sfs, 0, SEEK_SET);
    fwrite(&sb, sizeof(struct super_block), 1, sfs);
    
    struct inode_st tempInode = getCurrentInode();
    
    tempInode.size += sizeof(struct dir_ent);
    
    fseek(sfs, sizeof(struct super_block)+(currentDirectory*sizeof(struct inode_st)), SEEK_SET);
    fwrite(&tempInode, sizeof(struct inode_st), 1, sfs);
    
    fseek(sfs, sizeof(struct super_block)+inode*sizeof(struct inode_st), SEEK_SET);
	fwrite(&newDir, sizeof(struct inode_st), 1, sfs);
	
	fseek(sfs, sizeof(struct super_block)+(32*sizeof(struct inode_st))+(newDir.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
	fwrite(&dot, sizeof(struct dir_ent), 1, sfs);
	fwrite(&dotdot, sizeof(struct dir_ent), 1, sfs);
	
	struct dir_ent newDirEnt;
	strcpy(newDirEnt.name,fileName);
	newDirEnt.inode_no = inode;
	
	fseek(sfs, sizeof(struct super_block)+(32*sizeof(struct inode_st))+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE)+(tempInode.size-32), SEEK_SET);
	
	fwrite(&newDirEnt, sizeof(struct dir_ent), 1, sfs);		
}

void mkfile(char *fileName) {
	
	char text[] = "My name is Selim and his name is Doða. We have met at English preparation school 4 years ago...";
	
	struct inode_st newFile;
	newFile.type = REG_FILE;
	newFile.size = sizeof(text);
	
	int inode = getInodeNo(sb); 
	int datablock = getDataBlockNo(sb);
	
	int i;
	for(i=0; i<10; i++)
        newFile.data_block_indices[i] = datablock;
    
    int mask = 1 << inode; 
    sb.inode_bitmap = (sb.inode_bitmap & ~mask) | ((1 << inode) & mask); 
	
	int index = datablock/32;
	int p = datablock%32;	
	mask = 1 << p; 
    sb.data_bitmap[index] = (sb.data_bitmap[index] & ~mask) | ((1 << p) & mask);
    
    fseek(sfs, 0, SEEK_SET);
    fwrite(&sb, sizeof(struct super_block), 1, sfs);
    
    struct inode_st tempInode = getCurrentInode();
    
    tempInode.size += sizeof(struct dir_ent);
    
    fseek(sfs, sizeof(struct super_block)+(currentDirectory*sizeof(struct inode_st)), SEEK_SET);
    fwrite(&tempInode, sizeof(struct inode_st), 1, sfs);
    
    fseek(sfs, sizeof(struct super_block)+inode*sizeof(struct inode_st), SEEK_SET);
	fwrite(&newFile, sizeof(struct inode_st), 1, sfs);
	
	fseek(sfs, sizeof(struct super_block)+(32*sizeof(struct inode_st))+(newFile.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
	fwrite(&text, sizeof(text), 1, sfs);
	
	struct dir_ent newFileEnt;
	strcpy(newFileEnt.name,fileName);
	newFileEnt.inode_no = inode;
	
	fseek(sfs, sizeof(struct super_block)+(32*sizeof(struct inode_st))+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE)+(tempInode.size-32), SEEK_SET);
	fwrite(&newFileEnt, sizeof(struct dir_ent), 1, sfs);
}

void cd() {
	char fileName[32];
	scanf("%s", fileName);
	bool fileExists = false;
	
	struct inode_st tempInode = getCurrentInode();
	
	int numOfEntries = tempInode.size/sizeof(struct dir_ent);
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st)+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
	
	struct dir_ent tempEntry;
	int i;
	for(i=0; i<numOfEntries; i++){
		fread(&tempEntry, sizeof(struct dir_ent), 1, sfs);
		if(strcmp(tempEntry.name, fileName) == 0) {
			
			fileExists = true;
			fseek(sfs, sizeof(struct super_block)+(tempEntry.inode_no*sizeof(struct inode_st)), SEEK_SET);  
			
    		struct inode_st tempInode;
    		fread(&tempInode, sizeof(struct inode_st), 1, sfs);
    		
    		if(tempInode.type == DIRECTORY) {
    			currentDirectory = tempEntry.inode_no;
			}
			else {
				printf("This file is not a directory!\n");
			}
			break;
		}
	}
	if(!fileExists) {
		printf("No such directory!\n");
	}
}

void ls() {
	struct inode_st tempInode = getCurrentInode();
	
	int numOfEntries = tempInode.size/sizeof(struct dir_ent);
	fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st)+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
	
	struct dir_ent tempEntry;
	int i;
	for(i=0; i<numOfEntries; i++){
		fread(&tempEntry, sizeof(struct dir_ent), 1, sfs);
		printf("Directory %d: %s\n",i+1, tempEntry.name);
	}
}

void lsrec(int tab, int currentDirectory) {
	
	fseek(sfs, sizeof(struct super_block)+(currentDirectory*sizeof(struct inode_st)), SEEK_SET);  
    struct inode_st tempInode;
    fread(&tempInode, sizeof(struct inode_st), 1, sfs);
	
	if(tempInode.type == DIRECTORY) {
		int numOfEntries = tempInode.size/sizeof(struct dir_ent);
		fseek(sfs, sizeof(struct super_block)+32*sizeof(struct inode_st)+(tempInode.data_block_indices[9]*DATA_BLOCK_SIZE), SEEK_SET);
		
		int i;
		struct dir_ent entries[numOfEntries];
		for(i=0; i<numOfEntries; i++){
			fread(&entries[i], sizeof(struct dir_ent), 1, sfs);
		}
		
		int j;
		for(i=0; i<numOfEntries; i++){
			for(j=0; j<tab; j++) {
				printf("\t");
				printf("    ");
			}
			printf("Directory %d: %s\n",i+1, entries[i].name);
			if(strcmp(entries[i].name, ".") != 0 && strcmp(entries[i].name, "..") != 0) {
				currentDirectory = entries[i].inode_no;
				tab++;
				lsrec(tab, currentDirectory);
				tab--;
			}
		}
	}
}

void help() {
	printf("mkdir <directory name>: use it to create a new directory in the current directory\n");
	printf("mkfile <file name>: use it to create a new regular file in the current directory\n");
	printf("ls: use it to print list of the files/directories in the current directory\n");
	printf("lsrec: use it to print everything in the file system\n");
	printf("cd <directory name>: use it to change the current directory\n");
	printf("exit: use it to exit the program\n");
}

void init_fileSystem() {
	sb.inode_bitmap = 0;
	
	int i;
	for(i =0; i<10; i++)
		sb.data_bitmap[i] = 0;	
	
	struct inode_st root;
	root.type = DIRECTORY;
	root.size = 0;
	
	for(i=0; i<10; i++)
        root.data_block_indices[i] = 0;
        
    sb.inode_bitmap = 1;
    sb.data_bitmap[0] = 1;
    
    struct dir_ent dot;
	strcpy(dot.name,".");
	dot.inode_no = 0;
	
	struct dir_ent dotdot;
	strcpy(dotdot.name,"..");
	dotdot.inode_no = 0;
	
	root.size = sizeof(struct dir_ent)*2;
	
	fwrite(&sb, sizeof(struct super_block), 1, sfs);
	fwrite(&root, sizeof(struct inode_st), 1, sfs);
	
	fseek(sfs, sizeof(struct super_block)+(32*sizeof(struct inode_st))+(root.data_block_indices[9]*DATA_BLOCK_SIZE),SEEK_SET);
	
	fwrite(&dot, sizeof(struct dir_ent), 1, sfs);
	fwrite(&dotdot, sizeof(struct dir_ent), 1, sfs);
}

int getInodeNo(struct super_block sb) {
	int i;
	for(i=0; i<32; i++) {
		if((sb.inode_bitmap & 1) == 0) {
			return i;
		}
		else {
			sb.inode_bitmap = sb.inode_bitmap >> 1;
		}
	}
	printf("There is not enough space in the file system!");
	exit(0);
}

int getDataBlockNo(struct super_block sb) {
	int i;
	int j;
	for(i=0; i<10; i++) {
		for(j=0; j<32; j++) {
			if((sb.data_bitmap[i] & 1) == 0) {
				return (i*32)+j;
			}
			else {
				sb.data_bitmap[i] = sb.data_bitmap[i] >> 1;
			}
		}
	}
	printf("There is not enough space in the file system!");
	exit(0);
}

struct inode_st getCurrentInode() {
	fseek(sfs, sizeof(struct super_block)+(currentDirectory*sizeof(struct inode_st)), SEEK_SET);  
    struct inode_st tempInode;
    fread(&tempInode, sizeof(struct inode_st), 1, sfs);
    
    return tempInode;
}
