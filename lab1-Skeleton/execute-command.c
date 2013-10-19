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
command_status (command_t c) //return status of the command
{
  return c->status;
}

void pipe_command(command_t c, bool time_travel) //executing pipe command
{
  int status;
  int fd[2]; //descriptors for pipe
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
  if(pid1 > 0) //still in the parent shell process
    {
      pid2 = fork();
      if(pid2 < 0)
	{
	  fprintf(stderr, "Fork failed.");
	  exit(1);
	}
      if(pid2 > 0)
	{
	  close(fd[0]); //remove all of the parent's ties to the pipe
	  close(fd[1]);
	  finished_process = waitpid(-1, &status, 0); //wait for any process
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
      if(pid2 == 0) //the first part of the pipe
	{
	  close(fd[0]); //close the read_end
	  if(dup2(fd[1], 1) == -1)
	    {
	      fprintf(stderr, "Could not perform dup2.");
	      exit(1);
	    }
	  execute_command(c->u.command[0], time_travel);
	  _exit(c->u.command[0]->status);
	}
    }
  if(pid1 == 0) //second part of the pipe
    {
      close(fd[1]); //close the write end
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
      if(pid2 > 0) //second part of the sequence command
	{
	  waitpid(pid2, &status, 0); 
	  execute_command(c->u.command[1], time_travel);
	  _exit(c->u.command[1]->status);
	}
      if(pid2 == 0)
	{
	  execute_command(c->u.command[0], time_travel); //first part of sequence command
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
      if(c->input != NULL) //setup input and output for redirection
	{
	  int file_ptr1 = open(c->input, O_RDWR);
	  if(file_ptr1 < 0)
	    {
	      fprintf(stderr, "Error opening file: %s.", c->input);
	      _exit(1);
	    }
	  int exit_status1 = dup2(file_ptr1, 0);
	  if(exit_status1 < 0)
	    {
	      fprintf(stderr, "Dup2 error.");
	      _exit(1);//check for errors here
	    }
	  int exit_status2 = close(file_ptr1);
	  if(exit_status2 < 0)
	    {
	      fprintf(stderr, "Problem closing file.");
	      _exit(1);
	    }
	}
      if(c->output != NULL)
	{
	  int file_ptr2 = open(c->output, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	  if(file_ptr2 < 0)
	    {
	      fprintf(stderr, "Error writing to file.");
	      _exit(1);
	    }
	  int exit_status3 = dup2(file_ptr2, 1);
	  if(exit_status3 < 0)
	    {
	      fprintf(stderr, "Dup2 error."); //check for errors here
	      _exit(1);
	    }
	  int exit_status4 = close(file_ptr2);
	  if( exit_status4 < 0)
	    {
	      fprintf(stderr, "Problem closing file.");
	      _exit(1);
	    }
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
     c->status = c->u.subshell_command->status;
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
