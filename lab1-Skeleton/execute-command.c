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
*/

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
      }
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

void simple_command(command_t c, char **input)
{
  int status;
  pid_t pid = fork();
  if(pid < 0)
    {
      fprintf(stderr, "Fork failed.");
      exit(1);
    }
  if(pid > 0)
    {
      waitpid(pid, &status, 0); 
      c->status = status;
    }
  if(pid == 0)
    {
      if(c->input != NULL)
	{
	  int file_ptr1 = open(c->input, O_RDWR);
	  if(file_ptr1 < 0)
	    {
	      fprintf(stderr, "Error opening file.");
	      return;
	    }
	  dup2(file_ptr1, 0); //check for errors here
	  close(file_ptr1);
	}
      if(c->output != NULL)
	{
	  int file_ptr2 = open(c->output, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	  if(file_ptr2 < 0)
	    {
	      fprintf(stderr, "Error writing to file.");
	      return;
	    }
	  dup2(file_ptr2, 1); //check for errors here
	  close(file_ptr2);
	}
      execvp(input[0], input);
      fprintf(stderr, "Invalid command.");
      _exit(1);
    }
}


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
    simple_command(c, args);
    //There is no redirection, this is a VERY SIMPLE command
    /* if(c->input == NULL && c->output == NULL) {
      simple_command(c, args);
    } 
    //The left side TAKES IN data from the right side. right side must be
    //file? Can it be a string, or another command?
    if(c->input != NULL) {
      int fd_in = open(c->input, O_RDWR);
      if(fd_in < 0)
	{
	  fprintf(stderr, "Error");
	}
      if(dup2(fd_in, 0) < 0)
	{
	  fprintf(stderr, "Error");
	}
      if(close(fd_in) < 0)
	{
	  fprintf(stderr, "Error");
	}
      simple_command(c, args);
      char* right_arg = malloc(sizeof(c->input)+1);
      memcpy(right_arg, c->input, sizeof(c->input));
      right_arg[strlen(c->input)] = '\0';
      args[actual_size] = malloc(sizeof(right_arg));
      memcpy(args[actual_size], right_arg, sizeof(right_arg));
      fork_simple(args, c->output, c);
    }
    if(c->output != NULL) {
       fork_simple(args, c->output, c);
    }*/
  } else 
  if(c->type == AND_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    if(c->u.command[0]->status != 0){
      c->status = c->u.command[0]->status;
      return;
    }
    execute_command(c->u.command[1], time_travel);
    c->status = c->u.command[1]->status;
  } else if(c->type == OR_COMMAND) {
    subshell_execute_command(c->u.command[0], time_travel, output);
    if(c->u.command[0]->status != 0){
      subshell_execute_command(c->u.command[1], time_travel, output);
      c->status = c->u.command[1]->status;
    }
    c->status = c->u.command[0]->status;
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
    simple_command(c, args);
    /*
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
      }*/
  } else 
  if(c->type == AND_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    if(c->u.command[0]->status != 0)
      c->status = c->u.command[0]->status;
      return;
    execute_command(c->u.command[1], time_travel);
    c->status = c->u.command[1]->status;
  } else if(c->type == OR_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    if(c->u.command[0]->status != 0) {
      execute_command(c->u.command[1], time_travel);
      c->status = c->u.command[1]->status;
    }
    c->status = c->u.command[0]->status;
  }else
  if(c->type == PIPE_COMMAND) {
    pipe_command(c, time_travel);
  } else 
  if(c->type == SEQUENCE_COMMAND) {
    sequence_command(c, time_travel);
  } else 
  if(c->type == SUBSHELL_COMMAND) {
    subshell_execute_command(c->u.subshell_command, time_travel, c->output);
    c->status = c->u.subshell_command->status;
  }
}
