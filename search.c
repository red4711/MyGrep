#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define NUM_THREADS 20

struct file_search {
	char pattern[255];
	int file_index;
	char file_name[255];
};

pthread_t thread_container[NUM_THREADS];
int thread_container_size = 0;

void print_number(int i)
{
	printf(" %d ", i);
}

void print_string(const char *s)
{
	printf(" %s ", s);
}

void print_newline(){
	printf("\n");
}

void print_match(const char * file_name, int file_index, int line_number, char * line)
{
	print_number(file_index);
	print_string(file_name);
	print_number(line_number);
	print_string(line);
	print_newline();
}

void add_all_matching_lines(const char * pattern, int file_index, const char * file_name)
{
	int line_number = 1;
	char buf[BUFSIZ];
	FILE *file = fopen(file_name, "r");
	if (file == NULL)
		perror("File_fd open failed"), exit(1);
	while (fgets(buf, BUFSIZ, file) != NULL)
	{
		buf[strlen(buf) - 1] = '\0';
		if (strstr(buf, pattern) != NULL)
			print_match(file_name, file_index, line_number, buf);
		line_number++;
	}
}

static void *thread_file_search(void *ptr){
	struct file_search *data = (struct file_search *) ptr;
	add_all_matching_lines(data->pattern, data->file_index, data->file_name);
	free(ptr);
	pthread_exit(0);
}

void search(const char * pattern, int file_index, const char * file_name)
{
	struct stat file_stat;
	if (stat(file_name, &file_stat) < 0)
		return;
	if (S_ISREG(file_stat.st_mode)){
		struct file_search *data = malloc(sizeof(struct file_search));
		strcpy(data->pattern, pattern);
		strcpy(data->file_name, file_name);
		data->file_index = file_index;
		if(pthread_create(&thread_container[thread_container_size++], NULL, &thread_file_search, data))
			perror("Thread create failed"), exit(1);
	}
	else if (S_ISDIR(file_stat.st_mode))
	{
		DIR *dir = opendir(file_name);
		struct dirent *dirent;
		if (dir == NULL)
			perror("Opendir failed"), exit(1);
		while((dirent = readdir(dir))){
			if (strcmp(dirent->d_name, "..") == 0 || strcmp(dirent->d_name, ".") == 0)
				continue;
			char buf[BUFSIZ];
			sprintf(buf, "%s/%s", file_name, dirent->d_name);
			search(pattern, file_index, buf);
		}
	}
}
int main( int argc, char *argv[] )
{
	int file_index = 1;
	if ( argc < 3 )
		perror("Usage: search pattern file");
	for (int i = 2; i < argc; i++)
		search(argv[1], file_index++, argv[i]);
	for (int i = 0; i < thread_container_size; i++)
		pthread_join(thread_container[i], NULL);
	return 0;
}
