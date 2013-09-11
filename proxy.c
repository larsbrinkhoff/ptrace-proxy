/**********************************************************************
proxy.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <asm/unistd.h>

#include "ptproxy.h"
#include "sysdep.h"
#include "wait.h"

static void debugger_normal_return (debugger_state *);

/*
 * Handle debugger trap, i.e. syscall.
 */

extern void
debugger_syscall (debugger_state *debugger)
{
  int cancel = 0;
  long arg1, arg2, arg3, arg4, arg5;
  int syscall;

  syscall = syscall_get_number (debugger->pid);
  syscall_get_args (debugger->pid, &arg1, &arg2, &arg3, &arg4, &arg5);
 
  switch (syscall)
    {
    case __NR_execve:
      debugger->handle_trace = debugger_syscall; /* execve never returns */
      break;
    case __NR_ptrace:
      proxy_ptrace (debugger, arg1, arg2, arg3, arg4);
      cancel = 1;
      debugger->handle_trace = debugger_cancelled_return;
      break;
    case __NR_waitpid:
    case __NR_wait4:
      proxy_wait (debugger,
		  syscall == __NR_waitpid ? WAIT_WAITPID :
		  syscall == __NR_wait4   ? WAIT_WAIT4 : -1,
		  arg1, (int *)arg2, arg3, (void *)arg4);
      cancel = 1;
      debugger->handle_trace = proxy_wait_return;
      break;
    default:
      debugger->handle_trace = debugger_normal_return;
    }

  if (cancel)
    syscall_cancel (debugger->pid);

  syscall_continue (debugger->pid);
}

void
debugger_normal_return (debugger_state *debugger)
{
  debugger->handle_trace = debugger_syscall;
  syscall_continue (debugger->pid);
}

void
debugger_cancelled_return (debugger_state *debugger)
{
  syscall_set_result (debugger->pid, debugger->result);
  debugger->handle_trace = debugger_syscall;
  syscall_continue (debugger->pid);
}

void
proxy (pid_t debugger_pid, pid_t child)
{
  debugger_state debugger;
  debugee_state debugee;
  int status;
  int pid;

  debugger.handle_trace = debugger_syscall;
  debugger.pid = debugger_pid;
  debugger.debugee = &debugee;
  debugger.stopped = 0;

  debugee.traced = 0;
  debugee.stopped = 0;
  debugee.event = 0;
  debugee.zombie = 0;
  debugee.died = 0;
  debugee.first_stop = 1;
  debugee.pid = child;
  debugee.debugger = &debugger;

  sleep (1);

  while (debugger_pid != -1)
    {
      do
	{
	  pid = waitpid (-1, &status, WUNTRACED);
	}
      while (pid == -1 && errno == EINTR);

      if (pid == -1)
	{
	  exit (1);
	}

      if (pid == debugger_pid)
        {
	  if (WIFSTOPPED (status))
	    {
	      /* debugger got a signal */

	      if (WSTOPSIG (status) == SIGTRAP)
		{
		  debugger.handle_trace (&debugger);
		}
	      else
		{
		  /* go on as usual */
		  ptrace (PTRACE_SYSCALL, debugger.pid, 0, WSTOPSIG (status));
		}
	    }
	  else if (WIFEXITED (status))
	    {
	      /* debugger exited with status WEXITSTATUS (status) */
	      debugger_pid = -1;
	    }
	  else if (WIFSIGNALED (status))
	    {
	      /* debugger died from signal WTERMSIG (status) */
	      debugger_pid = -1;
	    }
	  else
	    {
	      /* unknown event */
	      exit (1);
	    }
        }
      else if (pid == child)
        {
	  debugee.stopped = 1;
	  debugee.event = 1;
	  debugee.wait_status = status;

	  if (WIFSTOPPED (status))
	    {
	      /* child got a signal */
	      if (debugee.first_stop)
		{
		  ptrace (PTRACE_CONT, debugee.pid, 0, 0);
		  debugee.stopped = 0;
		  debugee.event = 0;
		  debugee.first_stop = 0;
		}
	    }
	  else if (WIFEXITED (status))
	    {
	      /* child exited with status WEXITSTATUS (status) */
              debugee.zombie = 1;
	      child = -1;
	    }
	  else if (WIFSIGNALED (status))
	    {
	      /* child died from signal WTERMSIG (status) */
              debugee.zombie = 1;
	      child = -1;
	    }
	  else
	    {
	      /* unknown event */
	      exit (1);
	    }

	  if (debugee.stopped)
	    {
	      int r;

	      r = kill (debugger.pid, SIGCHLD);
	      if (debugger.stopped)
		proxy_wait_return (&debugger);
	    }
        }
    }
}
