#include <dirent.h> // For Directories;
#include <stdio.h>  // For I/O;
#include <stdlib.h> // For exit();
#include <unistd.h> // getopt();
#include <getopt.h> // getopt_long();
#include <sys/stat.h> // For param files;
#include <string.h>// strstok();
#include <time.h> // Forasctime
#include <sys/types.h> //
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>

char directory[255] = "./";
char c; 	// Возврашение результата getopt();
int error = 0; 	// увеличивается, когда возникают неизвестная аргументная строка.
DIR* dir;
struct dirent* entry;
struct stat file_st;

int cmp(const void * a, const void * b); 
void func_3();
void func_5(); // func for -l;

int main(int argc, char** argv)
{	
	if((c = getopt(argc, argv, "a:bl")) == -1)
	{
		if(argc > 1)
		{
			int i = 0;
			
			if(argc == 2)
			{
				while(i < argc)
				{
					DIR* dir_f;
					if((dir_f = opendir(argv[i])) == NULL){
						i++; 
						continue;
					}
				
					closedir(dir_f);

					memset(directory, 0, 255);
					strcpy(directory, argv[i]);

					func_3();
					i++;
				}
			}
			else
			{
				while(i < argc)
				{
					DIR* dir_f;
					if((dir_f = opendir(argv[i])) == NULL){
						i++; 
						continue;
					}
				
					closedir(dir_f);

					memset(directory, 0, 255);
					strcpy(directory, argv[i]);

					printf("%s:\n", argv[i]);
					func_3();
					if(i < (argc - 1)) printf("\n");
					i++;
				}
			}		
		} 
		else
		{
			func_3();
		}
	}
	else
	{
		do{
			switch(c)
			{
			case('a'):
				printf("Arg is: %s ; \n", optarg);
				break;
			case('b'):
				printf("FLAG b recieved  \n");
				break;
			case('l'):
				if(argc > 2)
				{	
					int i = 1;

					while(i < argc)
					{
						DIR* dir_f;
						if((dir_f = opendir(argv[i])) == NULL){
						 i++; 
						 continue;
						}
						closedir(dir_f);

						memset(directory, 0, 255);
						strcpy(directory, argv[i]);
						
						if((argc - 1) == i && argc == 3)
						{
							func_5();
							break;
						}else{
							if(argc > 3) printf("\n %s: \n",argv[i]);
							func_5();
							if(i < (argc - 1)) printf("\n");
							i++;
						}
					}

				}else{
					func_5();
				}

				break;
			case('?'):
				error += 1;
				printf("Uncnown FLAG \n");
				break;
			default:
				printf("Uncnown FLAG \n");
				dir = opendir(directory);

				if(!dir)
				{
					perror("diropen");
					exit(1);
				}

				while((entry = readdir(dir)) != NULL)
				{
					printf("%d - %s [%d] %llu \n", entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);
				}

				closedir(dir);
				break;
			}

		}while((c = getopt(argc, argv, "a:b")) != -1);
	}

	return 0;
}

int cmp(const void * a, const void * b) {
    return strcmp(*(char**)a, *(char**)b);
}

void strmode(mode_t mode, char* buf)
{
	const char chars[] = "rwxrwxrwx";

	for(size_t i = 0; i<9; i++)
	{
		buf[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
	}
	buf[9] = '\0';
}

void func_5()
{
	dir = opendir(directory);

	if(!dir)
	{
		perror("diropen");
		exit(1);
	}
	
	char** Files = (char**)calloc(0, sizeof(char*)); 
	int i = 0;
	size_t blk_cnt = 0;

	while((entry = readdir(dir)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
				
		char file_name[strlen(entry->d_name)];
		memset(file_name, 0, strlen(entry->d_name) + 1);
		strncpy(file_name, entry->d_name, strlen(entry->d_name));
		int fd = dirfd(dir);

		if(fstatat(fd, file_name, &file_st, 0) == 0)
		{	
			if(file_name[0] == '.') continue;
	
			blk_cnt += (file_st.st_blocks / 2);

			Files = (char**)realloc(Files, (++i)*sizeof(char*));
			Files[i-1] = (char*)calloc(strlen(file_name), sizeof(char));
			memset(Files[i-1], 0, strlen(file_name) + 1);
			strncpy(Files[i-1], file_name, strlen(file_name)*sizeof(char));
		}
	}

	qsort(Files, i, sizeof(char*), cmp);
	printf("total \033[36m%zu\033[0m\n", blk_cnt);

	for(int f = 0; f < i; f++)
	{
	
		struct stat file_st_1;
		int fd = dirfd(dir);
		fstatat(fd, Files[f],&file_st_1, 0);
	
		char mode[10];
		strmode(file_st_1.st_mode, mode);

		struct passwd* pwd;
		pwd = getpwuid(file_st_1.st_uid);
			
		if(pwd == NULL) continue;
					
		struct group*  gro;
		gro = getgrgid(file_st_1.st_gid);
					
		struct tm* timeinfo = localtime(&file_st_1.st_mtime);

		if(Files[f][0] == '.') continue;

		char buf_time[255];
		strftime(buf_time, sizeof(buf_time), "%d %b %Y %H:%M", timeinfo);
	
		if(file_st_1.st_mode & S_IXUSR)
		{

			if(S_ISDIR(file_st_1.st_mode))
			{
				printf("d%s  %d  %s  %8s", mode, file_st_1.st_nlink, pwd->pw_name, gro->gr_name);
				printf(" %6lld  %s \x1b[1;34m %-8s \x1b[0m \n", file_st_1.st_size, buf_time, Files[f]);
			}
			else
			{
				printf("-%s  %d  %s  %8s", mode, file_st_1.st_nlink, pwd->pw_name, gro->gr_name);
				printf(" %6lld  %2s \x1b[1;32m %-8s \x1b[0m \n", file_st_1.st_size, buf_time, Files[f] );
			}
		}
		else
		{
			printf("-%s  %d  %s  %8s", mode, file_st_1.st_nlink, pwd->pw_name, gro->gr_name);
			printf(" %6lld  %2s  %-8s  \n", file_st_1.st_size, buf_time, Files[f]);
		}
	}
	
	for(size_t j = 0; j < i ; j++)
	{
		free(Files[j]);
	}

	free(Files);
	
	if(closedir(dir) == -1) printf("ERROR %s\n", strerror(errno));
}

void func_3()
{
	dir = opendir(directory);

	if(!dir)
	{
		perror("diropen");
		exit(1);
	}
	
	char** Files = (char**)calloc(0, sizeof(char*)); 
	int i = 0;

	while((entry = readdir(dir)) != NULL)
	{
		if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
				
		char file_name[strlen(entry->d_name)];
		memset(file_name, 0, strlen(entry->d_name) + 1);
		strncpy(file_name, entry->d_name, strlen(entry->d_name));
		int fd = dirfd(dir);

		if((fstatat(fd ,file_name, &file_st, 0)) != -1)
		{	
			if(file_name[0] == '.') continue;
	
			Files = (char**)realloc(Files, (++i)*sizeof(char*));
			Files[i-1] = (char*)calloc(strlen(file_name), sizeof(char));
			memset(Files[i-1], 0, strlen(file_name) + 1);
			strncpy(Files[i-1], file_name, strlen(file_name)*sizeof(char));
		}
	}

	qsort(Files, i, sizeof(char*), cmp);

	for(int f = 0; f < i; f++)
	{
	
		struct stat file_st_1;
		int fd = dirfd(dir);
		fstatat(fd, Files[f],&file_st_1, 0);

		if(S_ISDIR(file_st_1.st_mode))		
		{
			printf("\033[1;34m%s  \033[0m", Files[f]);
		}else if(file_st_1.st_mode & S_IXUSR)

		{
			printf("\x1b[1;32m%s \x1b[0m", Files[f]);
		}else{
			printf("%s  ", Files[f]);
		}
	}

	printf("\n");
	
	for(size_t j = 0; j < i ; j++)
	{
		free(Files[j]);
	}

	free(Files);
	
	if(closedir(dir) == -1) printf("ERROR %s\n", strerror(errno));
}

