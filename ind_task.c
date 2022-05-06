#define BUFFER 1024
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

void scaning(char *src, char *dest);
char* writing_filepath(char *path, char *filename);
void creating_process(char *srcFile, char *destFile);
int copying_file(char *src_filename, char *dest_filename);

long count_proc = 0, curr_proc = 0;

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		fprintf(stderr, "Invalid parameters!\nThe_1st_parameter - the name of the first directory to scan.\nThe_2nd_parameter - the name of the second directory to sync.\nThe_3rd_parameter - the number of concurrent processes.\n");
		fprintf(stderr, "The format of the command should be: %s The_1st_parameter The_2nd_parameter The_3rd_parameter\n", argv[0]);
		return 1;
	}
	
	errno = 0;
	char *endptr;
	
	count_proc = strtol(argv[3], &endptr, 10);

	if((errno == ERANGE && (count_proc == LONG_MAX || count_proc == LONG_MIN)) || (errno != 0 && count_proc == 0))
	{
		perror("strtol");
		return 1;
	}
	else if(endptr == argv[3])
	{
		fprintf(stderr, "You entered an incorrect number of processes.\n");
		return 1;
	}
	else if(endptr[0] != '\0')
	{
		fprintf(stderr, "Invalid number of processes.\n");
		return 1;
	}
	else if(count_proc < 1)
	{
		fprintf(stderr, "There must be at least one process.\n");
		return 1;
	}
	
	if (strcmp(argv[1], argv[2]) == 0) 
	{
        	printf("The same folders are entered for scanning, the program does not need to be executed.\n");
        	return 0;
    	}
	
	scaning(argv[1], argv[2]);
	
	while (curr_proc != 0)
	{
        	int status;
        	int child_pid = wait(&status);
        	
        	if (child_pid == -1) 
        	{
            		perror("wait");
            		return 1;
        	}
        
        	curr_proc--;
    	}
	
	return 0;
		
}

void scaning(char *src, char *dest)
{
	int flag;
	
	char source[PATH_MAX];
	char destination[PATH_MAX];
	
	strcpy(source, src);
	strcpy(destination, dest);
	
	DIR *StartDir, *FinishDir;
	
	StartDir = opendir(src);
	FinishDir = opendir(dest);
	if(StartDir == NULL)
	{
		perror("opendir");
		return;
	}
	
	if(FinishDir == NULL)
	{
		closedir(StartDir);
		perror("opendir");
		return;
	}
	
	struct dirent *data_src, *data_dest;
	
	while((data_src = readdir(StartDir)) != NULL)
	{
		switch (data_src->d_type)
		{
		
			case DT_REG:
				
				flag = 0;
			
				while((data_dest = readdir(FinishDir)) != NULL)
				
					if((strcmp(data_src->d_name, data_dest->d_name) == 0) && (data_dest->d_type == DT_REG))
					{
						flag = 1;
						break;
					}
			
				if(flag == 0)
				{
					char *src_pathFile = writing_filepath(source, data_src->d_name);
					char *dest_pathFile = writing_filepath(destination, data_src->d_name);
					creating_process(src_pathFile, dest_pathFile);
					free(src_pathFile);
					free(dest_pathFile);
				}
			
				rewinddir(FinishDir);
				break;
		
			
			case DT_DIR:
				
				flag = 0;
				
				if (strcmp(data_src->d_name, ".") == 0 || strcmp(data_src->d_name, "..") == 0) 
				{
					break;
				}	
			
				while((data_dest = readdir(FinishDir)) != NULL)
				
					if((strcmp(data_src->d_name, data_dest->d_name) == 0) && (data_dest->d_type == DT_DIR))
					{
						flag = 1;
						char *src_pathDir = writing_filepath(source, data_src->d_name);
						char *dest_pathDir = writing_filepath(destination, data_dest->d_name);
						scaning(src_pathDir, dest_pathDir);
						free(src_pathDir);
						free(dest_pathDir);
						break;
					}
				
				if(flag == 0)
				{
					char *src_Dir = writing_filepath(source, data_src->d_name);
					char *dest_Dir = writing_filepath(destination, data_src->d_name);
				
					struct stat buffer;
					if (stat(src_Dir, &buffer) == -1)
					{
						perror("stat");
						exit(EXIT_FAILURE);
					}
					
					if (mkdir(dest_Dir, buffer.st_mode) == -1) 
					{
       					perror("mkdir");
        					exit(EXIT_FAILURE);
    					}
				
					scaning(src_Dir, dest_Dir);
					free(src_Dir);
					free(dest_Dir);	
				}
			
				rewinddir(FinishDir);
				break;
				
			default:
                		printf("This file %s passed, because it isn't regular or directory.\n", data_src->d_name);
                		break;
		}
	}
	
	if((closedir(StartDir) != 0) || (closedir(FinishDir) != 0))
	{
		perror("closedir");
		exit(EXIT_FAILURE);
	}
		
}

char* writing_filepath(char *path, char *filename)
{
	char *buff;
	
	buff = calloc((strlen(path) + strlen(filename) + 2), sizeof(char));
	if(buff == NULL)
	{
		perror("calloc");
		return "";
	}
	
	strcpy(buff, path);
	
	if(buff[strlen(buff) - 1] != '/')
	{
		strcat(buff, "/");
	}
	
	strcat(buff, filename);
	
	return buff;
}

void creating_process(char *srcFile, char *destFile)
{
	if(curr_proc >= count_proc)
	{
		int status = 0;
        	int child_pid = wait(&status);
        	
        	if (child_pid == -1) {
            		perror("wait");
            		return;
        	}
        
        	curr_proc--;
	}
		
	pid_t child;
	
	child = fork();
	switch(child)
	{
		case -1:
			fprintf(stderr, "An error has occurred, the process cannot be created.\n");
			exit(EXIT_FAILURE);
		
		case 0: 
            		copying_file(srcFile, destFile);
			exit(EXIT_SUCCESS);		
		
		default:
			curr_proc++;		
	}
	
}

int copying_file(char *src_filename, char *dest_filename)
{			
	FILE *filesrc, *filedest;
			
	filesrc = fopen(src_filename, "rb");
	if(filesrc == NULL)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	filedest = fopen(dest_filename, "w+b");
	if(filedest == NULL)
	{
		fclose(filesrc);
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	char content[BUFFER];
	int bytes = 0, copy_bytes = 0;
	while((bytes = fread(&content, 1, BUFFER, filesrc)) != 0)
	{
		fwrite(&content, 1, bytes, filedest);
		copy_bytes += bytes;
	}
		
	struct stat info;
	if(stat(src_filename, &info) == 0)
	{
		if(chmod(dest_filename, info.st_mode) != 0)
		{
			perror("chmod");
		}
	}
	else
	{
		perror("stat");
	}
		
	if((fclose(filesrc) != 0) || (fclose(filedest) != 0))
	{
		perror("fclose");
	}
	
	printf("PID: %d File path: %s Number of bytes copied: %d\n", getpid(), src_filename, copy_bytes);
		
	return 0;
}
