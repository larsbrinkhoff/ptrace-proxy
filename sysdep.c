/**********************************************************************
sysdep.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include <linux/unistd.h>

#ifdef i386
#  define REG_SYSCALL (4 * 11)
#  define REG_RESULT  (4 * 6)
#  define REG_ARG1    (4 * 0)
#  define REG_ARG2    (4 * 1)
#  define REG_ARG3    (4 * 2)
#  define REG_ARG4    (4 * 3)
#  define REG_ARG5    (4 * 4)
#endif
#ifdef __x86_64__
#  define REG_SYSCALL (8 * 10)	/* rax */
#  define REG_RESULT  (8 * 10)	/* rax */
#  define REG_ARG1    (8 * 14)	/* rdi */
#  define REG_ARG2    (8 * 13)	/* rsi */
#  define REG_ARG3    (8 * 12)	/* rdx */
#  define REG_ARG4    (8 * 7)	/* r10 */
#  define REG_ARG5    (8 * 9)	/* r8 */
#  define REG_ARG6    (8 * 8)	/* r9 */
#endif
#ifdef arm
#  define REG_SYSCALL (4 * 17)
#  define REG_RESULT  (4 * 0)
#  define REG_ARG1    (4 * 0)
#  define REG_ARG2    (4 * 1)
#  define REG_ARG3    (4 * 2)
#  define REG_ARG4    (4 * 3)
#  define REG_ARG5    (4 * 4)
#endif

int
syscall_get_number (pid_t pid)
{
  return ptrace (PTRACE_PEEKUSER, pid, REG_SYSCALL, 0);
}

void
syscall_cancel (pid_t pid)
{
  int err;

  err = ptrace (PTRACE_POKEUSER, pid, REG_SYSCALL, __NR_getpid);
  if (err == -1)
    {
      fprintf (stderr, "ptproxy: couldn't cancel syscall: %s\n",
	       strerror (errno));
      exit (1);
    }
}

void
syscall_get_args (pid_t pid, long *arg1, long *arg2,
		  long *arg3, long *arg4, long *arg5)
{
  *arg1 = ptrace (PTRACE_PEEKUSER, pid, REG_ARG1, 0);
  *arg2 = ptrace (PTRACE_PEEKUSER, pid, REG_ARG2, 0);
  *arg3 = ptrace (PTRACE_PEEKUSER, pid, REG_ARG3, 0);
  *arg4 = ptrace (PTRACE_PEEKUSER, pid, REG_ARG4, 0);
  *arg5 = ptrace (PTRACE_PEEKUSER, pid, REG_ARG5, 0);
}

void
syscall_set_result (pid_t pid, long result)
{
  ptrace (PTRACE_POKEUSER, pid, REG_RESULT, result);
}

void
syscall_continue (pid_t pid)
{
  ptrace (PTRACE_SYSCALL, pid, 0, 0);
}
