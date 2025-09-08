#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXARGLEN 32

/*
 * NOTE: 
 * beginner attempt at xargs
 * text "echo 1 2 3 | xargs echo 4"
 * the arguments printed are xargs echo 4
 * 

int
main(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		fprintf(1, "%s ", argv[i]);
	}
  exit(0); 
}
*/

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(1, "Usage: xargs <command> [args...]\n");
		exit(1);
	} 

	// Standard xargs for Unix, misread.
	/* 
	if (argv[1][0] == '-' && argv[1][1] == 'n') {
		int n = atoi(argv[2]);
		if (n <= 0) {
			fprintf(1, "xargs: invalid argument\n");
			exit(1); 	
		} else {
			// TODO: Implement -n option
		}
	} 
	*/

	char buffer[MAXARG * MAXARGLEN] = {0}, ch;
	char *p = buffer;
	char *argv_exec[MAXARG];
	int argc_exec = 0;
	for (int i = 1; i < argc; i++) {
		argv_exec[argc_exec++] = argv[i];
	} 

	while (read(0, &ch, sizeof(char)) > 0) {
		if (p == buffer) {
			argv_exec[argc_exec++] = p;
			*p++ = ch;
		} else if (ch == '\n' || ch == ' ') { 
			if (argc_exec < MAXARG - 1) {
				*p++ = '\0'; // Null-terminate the argument
			} else {
				fprintf(1, "xargs: argument list too long\n");
				exit(1);
			}

			if (ch == '\n') {
				if (fork() == 0) {
					exec(argv_exec[0], argv_exec);
					exit(1);
				} else {
					wait(0); 
				} 
				argc_exec = argc - 1;
				p = buffer;
			} else {
				argv_exec[argc_exec++] = p;
			}
		} else {
			*p++ = ch; 
		}
	}

	exit(0);
}

	// char ch;
	// char *command[MAXARG] = {0};
	// int arg_count = 0, char_count = 0;

	// NOTE: the standard input is the executed output
	/* while (read(0, &ch, sizeof(char)) > 0) {
		write(1, &ch, sizeof(char));
		switch (ch) {
			case '\n': 
			case ';':
			command[arg_count++][char_count++] = '\0';
			command[arg_count] = 0; 
			int p[2];
			pipe(p);
			if (fork() == 0) {
				close(p[0]);
				close(1);
				dup(p[1]);
				close(p[1]);

				exec(command[0], command);
				exit(1);
			} else {
				close(p[1]);

				char buf[MAXARGLEN];
				int n;
				if ((n = read(p[0], buf, sizeof(buf))) > 0) {
					buf[n] = 0;
					argv[argc++] = buf;
					argv[argc] = 0;
				}
				close(p[0]);
				wait(0);
			}
			arg_count = 0;
			char_count = 0;
			break;

			case ' ':
			command[arg_count++][char_count++] = '\0';
			char_count = 0;
			break;

			default:
			if (char_count < MAXARGLEN - 1) {
				command[arg_count][char_count++] = ch;
			} else {
				fprintf(1, "xargs: argument too long\n");
				exit(1);
			} 
		}
	}*/