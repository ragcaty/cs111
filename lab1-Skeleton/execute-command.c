// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int
command_status (command_t c)
{
  return c->status;
}

/*void execute_no_time_travel(command_t c)
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
}*/
void pipe_command(command_t c, bool time_travel)
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
  pid1 = fork();
  if(pid1 < 0)
    {
      fprintf(stderr, "Fork failed.");
      exit(1);
    }
  if(pid1 > 0)
    {
      close(fd[0]);
      close(fd[1]);
      waitpid(pid1, &status, 0);
    }
  if(pid1 == 0)
    {
      pid2 = fork();
      if(pid2 < 0)
	{
	  fprintf(stderr, "Fork failed.");
	  exit(1);
	}
      if(pid2 > 0)
	{
	  waitpid(pid2, &status, 0);
	  execute_command
  /*pid1 = fork();
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
	  execute_command(c->u.command[0], time_travel);
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
      execute_command(c->u.command[1], time_travel);
      _exit(c->u.command[1]->status);
      }*/
}

void sequence_command(command_t c, bool time_travel)
{
  int status;
  pid_t pid1;
  pid_t pid2;
  pid1 = fork();
  if(pid1 < 0)
    {
      fprintf(stderr, "Fork failed.");
      exit(1);
    }
  if(pid1 > 0)
    {
      waitpid(pid1, &status, 0);
    }
  if(pid1 == 0)
    {
      pid2 = fork();
      if(pid2 < 0)
	{
	  fprintf(stderr, "Fork failed.");
	  exit(1);
	}
      if(pid2 > 0)
	{
	  waitpid(pid2, &status, 0);
	  execute_command(c->u.command[1], time_travel);
	  _exit(c->u.command[1]->status);
	}
      if(pid2 == 0)
	{
	  execute_command(c->u.command[0], time_travel);
	  _exit(c->u.command[0]->status);
	}
    }
}
/*
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
      input = parse(*c->u.word);
      execvp(input[0], input);
      fprintf(stderr, "Invalid command.");
      _exit(1);
    }
}
*/
//Helps to parse string into args[] for execvp
//Parses on spaces and null token
//Returns char** args

char** 
parse(char* string)
{
  if(strlen(string) == 1) {}
    //do something for one character?
  char* start = string;
  char* end = string;
  char* nullend = &string[strlen(string)];
  char** args = (char**) malloc(5*sizeof(char*));
  int size_args = 5;
  int i = 0;
  while(end != nullend) {
    while(*end != ' ' &&  end != nullend) {
      end++;
    }
    int length = 1+(end - start);
    args[i] = malloc(length*sizeof(char));
    memcpy(args[i], start, length);
    args[i][length-1] = '\0';
    if(end == nullend)
      break;
    end++;
    start = end;
    i++;
    if(i == size_args) {
      size_args *= 2;
      args = realloc(args, size_args*sizeof(char*)); 
    }
  }
  if(i == size_args-1)
    args = realloc(args, (size_args+1)*sizeof(char*));
  return args;
}

void
fork_simple (char** args, char* c_output, command_t c)
{
//If c_output is not null, redirect stdout to be stdin of that function
//e.g sort < a > b. Redirect output of sort < a into b
  pid_t child_pid;
  int pid_status;
  //int defout = dup(1);
  int filePtr = NULL;
  if(c_output != NULL) {
    filePtr = open(c_output, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
    if(filePtr < 0) {
      fprintf(stderr, "File reading error\n");
      return;
    }
  }
//Everything that should go to stdout goes to filePtr
  dup2(filePtr, 1);
  child_pid = fork();
  if(child_pid < 0) {
    fprintf(stderr, "failed to fork\n");
    return;
  }
  if(child_pid == 0) {
      if(execvp(args[0], args)  == -1) {
        fprintf(stderr, "Fail on execvp, simple command\n");
        _exit(pid_status);
      }
  }
//Return stdout to normal
  //dup2(defout, 1);
  close(filePtr);
  //close(defout);
  wait(&pid_status);
  c->status = pid_status;
}

void
subshell_execute_command (command_t c, bool time_travel, char* output)
{
  /*  error (1, 0, "command execution not yet implemented");*/
  if(c->type == SIMPLE_COMMAND) {
    char** args = parse(*(c->u.word));
    int actual_size = 0;
    char** ptr = args;
    while(*ptr != NULL) {
      actual_size++;
      ptr++;
    }
    //There is no redirection, this is a VERY SIMPLE command
    if(c->input == NULL && c->output == NULL) {
      fork_simple(args, output, c);
    } 
    //The left side TAKES IN data from the right side. right side must be
    //file? Can it be a string, or another command?
    if(c->input != NULL) {
      char* right_arg = malloc(sizeof(c->input)+1);
      memcpy(right_arg, c->input, sizeof(c->input));
      right_arg[strlen(c->input)] = '\0';
      args[actual_size] = malloc(sizeof(right_arg));
      memcpy(args[actual_size], right_arg, sizeof(right_arg));
      fork_simple(args, c->output, c);
    }
    if(c->output != NULL) {
      fork_simple(args, c->output, c);
    }
  } else 
  if(c->type == AND_COMMAND) {
    subshell_execute_command(c->u.command[0], time_travel, output);
    if(c->u.command[0]->status != 0)
      return;
    subshell_execute_command(c->u.command[1], time_travel, output);
  } else if(c->type == OR_COMMAND) {
    subshell_execute_command(c->u.command[0], time_travel, output);
    if(c->u.command[0]->status != 0)
      subshell_execute_command(c->u.command[1], time_travel, output);
  } else
  if(c->type == PIPE_COMMAND) {
    pipe_command(c, time_travel);
  } else 
  if(c->type == SEQUENCE_COMMAND) {
    sequence_command(c, time_travel);
  } else
  if(c->type == SUBSHELL_COMMAND) {
    subshell_execute_command(c->u.subshell_command, time_travel, output);
  }
}

void
execute_command (command_t c, bool time_travel)
{
  /*  error (1, 0, "command execution not yet implemented");*/
  if(c->type == SIMPLE_COMMAND) {
    char** args = parse(*(c->u.word));
    int actual_size = 0;
    char** ptr = args;
    while(*ptr != NULL) {
      actual_size++;
      ptr++;
    }
    //There is no redirection, this is a VERY SIMPLE command
    if(c->input == NULL && c->output == NULL) {
      fork_simple(args, NULL, c);
    } 
    //The left side TAKES IN data from the right side. right side must be
    //file? Can it be a string, or another command?
    if(c->input != NULL) {
      char* right_arg = malloc(sizeof(c->input)+1);
      memcpy(right_arg, c->input, sizeof(c->input));
      right_arg[strlen(c->input)] = '\0';
      args[actual_size] = malloc(sizeof(right_arg));
      memcpy(args[actual_size], right_arg, sizeof(right_arg));
      fork_simple(args, c->output, c);
    }
    if(c->output != NULL) {
      fork_simple(args, c->output, c);
    }
  } else 
  if(c->type == AND_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    if(c->u.command[0]->status != 0)
      return;
    execute_command(c->u.command[1], time_travel);
  } else if(c->type == OR_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    if(c->u.command[0]->status != 0)
      execute_command(c->u.command[1], time_travel);
  }else
  if(c->type == PIPE_COMMAND) {
    pipe_command(c, time_travel);
  } else 
  if(c->type == SEQUENCE_COMMAND) {
    sequence_command(c, time_travel);
  } else 
  if(c->type == SUBSHELL_COMMAND) {
    subshell_execute_command(c->u.subshell_command, time_travel, c->output);
  }
}
