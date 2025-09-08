#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
	int parent2child[2], child2parent[2];
	char buf[1], byte = 'x';

	pipe(parent2child);
	pipe(child2parent);

	if(fork() == 0) {
		// Child process
		close(parent2child[1]); // Close write end of parent2child
		close(child2parent[0]); // Close read end of child2parent

		read(parent2child[0], buf, 1); // Read from parent
		fprintf(1, "%d: received ping\n", getpid());
		write(child2parent[1], buf, 1); // Send a byte to parent
	} else {
		// Parent process
		close(parent2child[0]); // Close read end of parent2child
		close(child2parent[1]); // Close write end of child2parent

		write(parent2child[1], &byte, 1); // Send a byte to child
		read(child2parent[0], buf, 1); // Read from child, it will wait!
		fprintf(1, "%d: received pong\n", getpid());
	}

  exit(0);
}
