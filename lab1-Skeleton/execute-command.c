// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void execute_no_time_travel(command_t c)
{
  switch(c->type)
    {
    case SIMPLE_COMMAND:
      simple_command(c);
      break;
    case PIPE_COMMAND:
      pipe_command(c);
      break;
    case AND_COMMAND:
      and_command(c);
    case OR_COMMAND:
      or_command(c);
    case SUBSHELL_COMMAND:
      subshell_command(c);
    case SEQUENCE_COMMAND:
      sequence_command(c);
      break;
    default:
      fprintf(stderr, "Command type is not known.");
      exit(1);
    }
}

void execute_command(command_t c, bool time_travel)
{
  if(time_travel == 0)
    {
      execute_no_time_travel(c);
    }
  else
    {
      exit(0);
      }
}
void pipe_command(command_t c)
{
  int status;
  int fd[2];
  pid_t pid1;
  pid_t pid2;
  pid_t finished_process;
  if(pipe(fd) == -1)
    {
      fprintf(stderr, "Could not create a pipe.");
      exit(1);
    }
  pid_1 = fork();
  if(pid1 < 0)
    {
      fprintf(stderr, "Fork failed.");
      exit(1);
    }
  if(pid1 > 0)
    {
      pid2 = fork();
      if(pid2 < 0)
	{
	  fprintf(stderr, "Fork failed.");
	  exit(1);
	}
      if(pid2 > 0)
	{
	  close(fd[0]);
	  close(fd[1]);
	  finished_process = waitpid(-1, &status, 0);
	  if(finished_process == pid1)
	    {
	      c->status = status;
	      waitpid(pid2, &status, 0);
	      return;
	    }
	  if(finished_process == pid2)
	    {
	      waitpid(pid1, &status, 0);
	      c->status = status;
	      return;
	    }
	}
      if(pid2 == 0)
	{
	  close(fd[0]);
	  if(dup2(fd[1], 1) == -1)
	    {
	      fprintf(stderr, "Could not perform dup2.");
	      exit(1);
	    }
	  execute_no_time_travel(c->u.command[0]);
	  _exit(c->u.command[0]->status);
	}
    }
  if(pid1 == 0)
    {
      close(fd[1]);
      if(dup2(fd[0], 0) == -1)
	{
	  fprintf(stderr, "Could not perform dup2.");
	  exit(1);
	}
      execute_no_time_travel(c->u.command[1]);
      _exit(c->u.command[1]->status);
    }
}

void simple_command(command_t c)
{
  int status;
  char **input;
  pid_t pid = fork();
  if(pid < 0)
    {
      fprintf(stderr, "Fork failed.");
      exit(1);
    }
  if(pid > 0)
    {
      waitpid(pid, &status, 0); //might have to check for errors in child process
      c->status = status;
    }
  if(pid == 0)
    {
      create_string(&input, c);
      execvp(input[0], input);
      fprintf(stderr, "Invalid command.");
      exit(1);
    }
}

void create_string(char **input, command_t c)
{
}
