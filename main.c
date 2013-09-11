/**********************************************************************
main.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>

extern void proxy (pid_t, pid_t);

int
main (int argc, char **argv)
{
  pid_t gdb_pid, child_pid;

  gdb_pid = fork ();
  if (gdb_pid == 0)
    {
      ptrace (PTRACE_TRACEME, 0, 0, 0);
      execlp ("gdb", "gdb", NULL);
      exit (1);
    }
  ptrace (PTRACE_SYSCALL, gdb_pid, 0, 0);

  child_pid = fork ();
  if (child_pid == 0)
    {
      ptrace (PTRACE_TRACEME, 0, 0, 0);
      execl ("child", "child", NULL);
      exit (1);
    }

  proxy (gdb_pid, child_pid);

  return 0;
}
