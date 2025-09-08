#include "kernel/types.h"
#include "user/user.h"

void primes(int read_fd);

int
main(int argc, char **argv)
{
  int p[2];
  pipe(p);

  for (int i = 2; i <= 280; i++) {
    write(p[1], &i, sizeof(i));
  }
  close(p[1]);
  
  primes(p[0]);
  
  exit(0);
}

void 
primes(int read_fd) 
{
  int p;
  if (read(read_fd, &p, sizeof(p)) <= 0) {
    close(read_fd);
    return;
  }
  printf("prime %d\n", p);

  int p2[2];
  pipe(p2);

  if (fork() == 0) {
    close(read_fd);
    close(p2[1]);
    primes(p2[0]);
    exit(0);
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
  }
}
