// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

struct command_stream
{
  int array_size;
  command_t command_array;
  char** string_array;
  int command_position;
} stream;


command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  command_stream_t stream_t = &stream;
  stream_t = malloc(sizeof(command_stream_t));
  stream_t->string_array = malloc(8*sizeof(char*));
  int i = 0;
  for(; i<8; i++) {
    stream_t->string_array[i] = malloc(100*sizeof(char));
  }
  stream_t->string_array[0] = "true";
  stream_t->string_array[1] = "g++ -c foo.c";
  stream_t->string_array[2] = ": : :";
  stream_t->string_array[3] = "cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!";
  stream_t->string_array[4] = "a b<c > d";
  stream_t->string_array[5] = "cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!";
  stream_t->string_array[6] = "a&&b||c &&d | e && f| g<h";
  stream_t->string_array[7] = "a<b>c|d<e>f|g<h>i";
  stream_t->string_array[8] = "mk dir ; cd dir; ls;";
  stream_t->string_array[9] = "(Hello | cat < /etc/passwd)";
  stream_t->string_array[10] = "a && b && (Hello || d) | cat";
  stream_t->command_position = 0;
  stream_t->array_size = 11;
  /*stream_t = (command_stream_t) malloc(sizeof(command_stream_t));
  stream_t->array_size = 1;
  stream_t->command_position = 0;
  stream_t->string_array[0] = temp2;*/
  int next_byte;
 /* stream_t->command_array = malloc(20 * sizeof (command_t));
  while((next_byte=get_next_byte(get_next_byte_argument)) >= 0)
    {
         printf("%c", next_byte);
    }*/
  return stream_t;
}
//This helper function removes beginning and trailing spaces, between 
//the pointers given in the parameters.
char*
read_part_command(char* start, char* end) {
  char* return_ptr;
  while(*start == ' ')
    start++;
  end--;
  while(*end == ' ')
    end--;
  int size = end - start;
  return_ptr = (char*) malloc(size+2);
  strncpy(return_ptr, start, size+1);
  return_ptr[size+2] = '\0';
  //printf("return_ptr %sE\n", return_ptr);
  return return_ptr;
}

//Majority of work to find commands found here
//Priority goes (), <>, |, &&||, ;, nothing.
//Therefore, search in reverse order and recursively parse the commands.
command_t
parse_command_stream (char* test) {
  char* pipe = strchr(test, '|');
  char* start_ptr = test;
  char* pos_ptr = start_ptr;
  char* end_ptr = test+strlen(test);
  char* left;
  char* right;
  
  command_t cmd = ((command_t) malloc(sizeof(command_t)));
  cmd->u.word = (malloc(1*sizeof(char*)));
  *cmd->u.word = malloc(sizeof(start_ptr));
  
  //Start searching for the tokens
  char* and_token = strrchr(pos_ptr, '&');
  char* temp_or = strstr(pos_ptr, "||");

  //OR token must not be mistaken for PIPE, so look for second |
  char* or_token = NULL;
  while(temp_or != NULL) {
    or_token = temp_or;
    temp_or = strstr(temp_or+2, "||");
  }
  char* in_token = strrchr(pos_ptr, '>');
  char* out_token = strrchr(pos_ptr, '<');
  char* semi_token = strrchr(pos_ptr, ';');
  char* subshell_token = strrchr(pos_ptr, '(');
  char* end_subshell_token = strrchr(pos_ptr, ')');
  

  if(subshell_token != NULL) {
    if(and_token - subshell_token > 0 ) 
      and_token = NULL;
    if(or_token - subshell_token > 0)
      or_token = NULL;
    if(pipe - subshell_token > 0)
      pipe = NULL;
    if(in_token - subshell_token > 0)
      in_token = NULL;
    if(out_token - subshell_token >0)
      out_token = NULL;
    if(semi_token - subshell_token > 0)
      semi_token = NULL;
  }
//If no tokens are found, this is a simple command.
  if(pipe == NULL && and_token == NULL && or_token == NULL && in_token == NULL 
                  && out_token == NULL && semi_token == NULL 
                                       && subshell_token == NULL) {
    cmd->type = SIMPLE_COMMAND;
    strncpy(*cmd->u.word, start_ptr, strlen(start_ptr)); 
  } else 

//Search for sequence commands next.
  if(semi_token != NULL) {
    cmd->type = SEQUENCE_COMMAND;
    left = read_part_command(start_ptr, semi_token);
    right = read_part_command(semi_token+1, end_ptr);
    cmd->u.command[0] = parse_command_stream(left);
    cmd->u.command[1] = parse_command_stream(right);
  } else 

//Search for ANDS and ORS at the same time, then go from right-left order priority
if(and_token != NULL || or_token != NULL) {
    if(and_token == NULL || (and_token != NULL && or_token != NULL 
                                       && ((and_token - or_token) < 0))) {
      left = read_part_command(start_ptr, or_token);
      right = read_part_command(or_token+2, end_ptr);
      cmd->type = OR_COMMAND;
    } else {
      left = read_part_command(start_ptr, and_token-1);
      right = read_part_command(and_token+1, end_ptr);
      cmd->type = AND_COMMAND;
    }
    cmd->u.command[0] = parse_command_stream(left);
    cmd->u.command[1] = parse_command_stream(right);
  } else 

//Search for pipes
if(pipe != NULL) {
    left = read_part_command(start_ptr, pipe);
    right = read_part_command(pipe+1, end_ptr);
    cmd->type = PIPE_COMMAND;
    cmd->status = -1;
    cmd->u.command[0] = parse_command_stream(left);
    cmd->u.command[1] = parse_command_stream(right);
  } else 

//I/O Redirection, look for both and right-left priority
if(in_token != NULL || out_token != NULL) {
    cmd->type = SIMPLE_COMMAND;
    if(out_token - in_token > 0) {
      left = read_part_command(start_ptr, out_token);
      right = read_part_command(out_token+1, end_ptr);
      cmd->input = malloc(sizeof(start_ptr));
      strncpy(cmd->input, right, strlen(right));
    } else {
      left = read_part_command(start_ptr, in_token);
      right = read_part_command(in_token+1, end_ptr);
      cmd->output = malloc(sizeof(start_ptr));
      strncpy(cmd->output, right, strlen(right));
    }
    strncpy(*cmd->u.word, left, strlen(left));
  } else 

//Subshell commands are highest priority so they are found last so they are added to the top of the tree
if(subshell_token != NULL) {
  cmd->type = SUBSHELL_COMMAND;
  left = read_part_command(++subshell_token, end_subshell_token);
  cmd->u.subshell_command = malloc(sizeof(command_t));
  cmd->u.subshell_command = parse_command_stream(left);
}
  return cmd;
}

command_t
read_command_stream (command_stream_t s)
{
  command_t cmd;
  if( s->command_position < s->array_size) {
    cmd = parse_command_stream(s->string_array[s->command_position]);
    s->command_position++;
  } else {
    cmd = NULL;
  }
  return cmd;
}






