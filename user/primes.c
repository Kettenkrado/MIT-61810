#include "kernel/types.h"
#include "user/user.h"

void primes(int p[2]);

int
main(int argc, char **argv)
{
  int p[2];
  pipe(p);

  if (fork() == 0) {
    primes(p);
  } else {
    close(p[0]); 
    for (int i = 2; i <= 280; i++) {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]);
    wait(0); 
  }
  
  exit(0);
}

void 
primes(int p[2]) 
{
  int prime;
  close(p[1]); 
  if (read(p[0], &prime, sizeof(prime)) <= 0) {
    return;
  }
  fprintf(1, "prime %d\n", prime);

  int p2[2];
  pipe(p2);

  if (fork() == 0) {
    close(p[0]);
    close(p2[1]);
    primes(p2);
    exit(0);
  } else {
    close(p2[0]);
    int num;
    while (read(p[0], &num, sizeof(num))) {
      if (num % prime) {
        write(p2[1], &num, sizeof(num));
      }
    }
    close(p[0]);
    close(p2[1]);
    wait(0);
  }

  exit(0);
}