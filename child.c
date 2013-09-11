#include <stdio.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
  volatile int x = 1;

  printf ("hello world, my pid is %d\n", getpid ());
  while (x)
    ;
  printf ("my work here is done\n");
  return 0;
}
