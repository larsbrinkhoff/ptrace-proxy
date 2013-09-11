/**********************************************************************
ptrace.c

Copyright (C) 1999 Lars Brinkhoff.  See the file COPYING for licensing
terms and conditions.
**********************************************************************/

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include "ptproxy.h"

void
proxy_ptrace (struct debugger *debugger, int arg1, pid_t arg2,
	      long arg3, long arg4)
{
  if (arg2 != debugger->debugee->pid)
    return;

  if (debugger->debugee->died)
    {
      debugger->result = -ESRCH;
      debugger->handle_trace = debugger_cancelled_return;
      return;
    }

  switch (arg1)
    {
    case PTRACE_ATTACH:
      if (debugger->debugee->traced)
	debugger->result = -EPERM;
      else
	{
	  int result;
	  debugger->result = 0;
	  debugger->debugee->traced = 1;
	  result = kill (arg2, SIGSTOP);
	}
      break;

    case PTRACE_CONT:
      debugger->result = ptrace (PTRACE_CONT, arg2, arg3, arg4);
      break;

    case PTRACE_DETACH:
      if (debugger->debugee->traced)
	{
	  debugger->result = 0;
	  debugger->debugee->traced = 0;
	}
      else
	debugger->result = -EPERM;
      break;

    case PTRACE_GETFPREGS:
      {
	long regs[27];
	int i;

	debugger->result = ptrace (PTRACE_GETFPREGS, arg2, 0, regs);
	if (debugger->result == -1)
	  debugger->result = -errno;
	else
	  {
	    for (i = 0; i < 27; i++)
	      ptrace (PTRACE_POKEDATA, debugger->pid,
		      arg4 + 4 * i, regs[i]);
	  }
      }
      break;

    case PTRACE_GETREGS:
      {
	long regs[17];
	int i;

	debugger->result = ptrace (PTRACE_GETREGS, arg2, 0, regs);
	if (debugger->result == -1)
	  debugger->result = -errno;
	else
	  {
	    for (i = 0; i < 17; i++)
	      ptrace (PTRACE_POKEDATA, debugger->pid,
		      arg4 + 4 * i, regs[i]);
	  }
      }
      break;

    case PTRACE_KILL:
      debugger->result = ptrace (PTRACE_KILL, arg2, arg3, arg4);
      if (debugger->result == -1)
	debugger->result = -errno;
      break;

    case PTRACE_PEEKDATA:
    case PTRACE_PEEKTEXT:
    case PTRACE_PEEKUSER:
      errno = 0;
      debugger->result = ptrace (arg1, arg2, arg3, 0);
      if (debugger->result == -1 && errno != 0)
	debugger->result = -errno;
      else
	{
	  ptrace (PTRACE_POKEDATA, debugger->pid, arg4, debugger->result);
	  debugger->result = 0;
	}
      break;

    case PTRACE_POKEDATA:
    case PTRACE_POKETEXT:
    case PTRACE_POKEUSER:
      debugger->result = ptrace (arg1, arg2, arg3, arg4);
      if (debugger->result == -1)
	debugger->result = -errno;
      break;

    case PTRACE_SETFPREGS:
      {
	long regs[27];
	int i;

	for (i = 0; i < 27; i++)
	  regs[i] = ptrace (PTRACE_PEEKDATA, debugger->pid,
			    arg4 + 4 * i, 0);
	debugger->result = ptrace (PTRACE_SETFPREGS, arg2, 0, regs);
	if (debugger->result == -1)
	  debugger->result = -errno;
      }
      break;

    case PTRACE_SETREGS:
      {
	long regs[17];
	int i;

	for (i = 0; i < 17; i++)
	  regs[i] = ptrace (PTRACE_PEEKDATA, debugger->pid,
			    arg4 + 4 * i, 0);
	debugger->result = ptrace (PTRACE_SETREGS, arg2, 0, regs);
	if (debugger->result == -1)
	  debugger->result = -errno;
      }
      break;

    case PTRACE_SINGLESTEP:
      debugger->result = ptrace (PTRACE_SINGLESTEP, arg2, arg3, arg4);
      if (debugger->result == -1)
	debugger->result = -errno;
      break;

    case PTRACE_SYSCALL:
      debugger->result = -EINVAL; 
      break;

    case PTRACE_TRACEME:
      debugger->result = -EINVAL; 
      break;

    default:
      debugger->result = -EINVAL; 
    }
}
