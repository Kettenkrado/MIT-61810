#include "kernel/types.h"
#include "user/user.h"


__attribute__((noreturn)) void prime(int read_fd);

int
main(int argc, char **argv)
{
	int p[2];
	pipe(p);

	for (int i = 2; i <= 100; i++) {
		write(p[1], &i, sizeof(i));
	}
	close(p[1]); 
	prime(p[0]);

	exit(0);
}

__attribute__((noreturn))
void 
prime(int read_fd) 
{
	int p;
	if (read(read_fd, &p, sizeof(p)) <= 0) {
		exit(0);
	}
	fprintf(1, "prime %d\n", p);

	int p2[2];
	pipe(p2);
	if (fork() == 0) {
		close(read_fd);
		close(p2[1]); 
		prime(p2[0]);
	} else {
		close(p2[0]);
		int num;
		while (read(read_fd, &num, sizeof(num)) > 0) {
			if (num % p != 0) {
				write(p2[1], &num, sizeof(num));
			}
		}
		close(read_fd);
		close(p2[1]); 
		wait(0); 
		exit(0);
	}
}
