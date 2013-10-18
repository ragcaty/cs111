// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
int
command_status (command_t c)
{
  return c->status;
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
//Debugging args
/*  int j = 0; 
  for(; j<size_args; j++)
    printf("%s \n", args[j]);*/
  return args;
}

void
fork_simple (char** args, char* c_output)
{
//If c_output is not null, redirect stdout to be stdin of that function
//e.g sort < a > b. Redirect output of sort < a into b
  pid_t child_pid;
  int child_status;
  int defout = dup(1);
  FILE* filePtr = NULL;
  if(c_output != NULL) {
    filePtr = open(c_output, O_RDWR);
    if(filePtr == NULL) {
      fprintf(stderr, "File reading error");
      exit(1);
    }
  }
//Everything that should go to stdout goes to filePtr
  dup2(filePtr, 1);
  child_pid = fork();
  if(child_pid < 0) {
    fprintf(stderr, "failed to fork");
    exit(1);
  }
  if(child_pid == 0) {
    execvp(args[0], args);
    fprintf(stderr, "Fail on execvp, simple command");
    exit(1); 
  }
//Return stdout to normal
  dup2(defout, 1);
  close(filePtr);
  close(defout);
  pid_t tpid = wait(&child_status);
  if(filePtr != NULL)
    close(filePtr);
  printf("done with simple fork\n");
}

void
execute_command (command_t c, bool time_travel)
{
  /*  error (1, 0, "command execution not yet implemented");*/
  char** args;
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
      fork_simple(args, NULL);
    } 
    //The left side TAKES IN data from the right side. right side must be
    //file? Can it be a string, or another command?
    if(c->input != NULL) {
      char* right_arg = malloc(sizeof(c->input)+1);
      memcpy(right_arg, c->input, sizeof(c->input));
      right_arg[strlen(c->input)] = '\0';
      args[actual_size] = malloc(sizeof(right_arg));
      memcpy(args[actual_size], right_arg, sizeof(right_arg));
           fork_simple(args, c->output);
    }
    if(c->output != NULL) {
      fork_simple(args, c->output);
    }
  } else 
  if(c->type == AND_COMMAND || c->type == OR_COMMAND) {
    execute_command(c->u.command[0], time_travel);
    execute_command(c->u.command[1], time_travel);
  } else
  if(c->type == SUBSHELL_COMMAND) {
    execute_command(c->u.subshell_command, time_travel);
  }
}
