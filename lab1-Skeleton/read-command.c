=======
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
  int full_command_size;
  int full_command_position;
  char **full_commands;
}stream;


static command_stream_t stream_t = &stream;

int is_valid_character(int input)
{
  if((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') || (input >= '0' && input <= '9') || (input == '!') || (input == '%') || (input == '+') || (input ==',') | (input == '-') || (input == '.') || (input == '/') || (input == ':') || (input == '@') || (input == '^') || (input == '_'))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int is_valid_token(int input)
{
  if(input == ';' || input == '|' || input == '&' || input == '(' || input == ')' || input == '<' || input == '>')
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int is_valid_semicolon(char *input)
{
  int m;
  int is_valid;
  is_valid = 0;
  for(m = 0; input[m] != '\0'; m++)
    {
      if(is_valid_character(input[m]) == 1)
	{
	  is_valid = 1;
	}
    }
  return is_valid;
}

int is_correct_command(char *input)
{
  int j;
  int correct_command;
  int previous_character;
  previous_character = 0;
  correct_command = 1;
  for(j = 0; input[j] != '\0'; j++)
    {
      if(input[j] == '<' || input[j] == '>')
	{
	  if((previous_character == '<' || previous_character == '>'))
	    {
	      return 0;
	    }
	  correct_command = 0;
	  previous_character = input[j];
	  continue;
	}
      if((is_valid_token(input[j]) == 1) || (is_valid_character(input[j]) == 1))
	{
	  correct_command = 1;
	  previous_character = input[j];
	  continue;
	}
    }
  return correct_command;
}
  

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  int next_byte;
  int previous_byte;
  int previous_character;
  int line_number;
  int start_comment;
  int command_position;
  int command_size;
  int and_symbols;
  int correct_command;
  int complete_subshell;
  int pipeline_count;
  int last_newline;
  previous_character = 0;
  pipeline_count = 0;
  complete_subshell = 1;
  previous_byte = 0;
  line_number = 1;
  start_comment = 0;
  command_position = 0;
  command_size = 10;
  and_symbols = 0;
  correct_command = 0;
  stream_t->full_commands = malloc(5 * sizeof(char*));
  stream_t->full_command_size = 5;
  stream_t->full_command_position = 0;
  int i;
  for(i = 0; i < 5; i++)
  {
    stream_t->full_commands[i] = malloc(10 * sizeof(char));
  }

  while((next_byte=get_next_byte(get_next_byte_argument)) >= 0)
    {
      if(stream_t->full_command_position == (stream_t->full_command_size - 1))
	{
	  stream_t->full_command_size += 5;
	  stream_t->full_commands = realloc(stream_t->full_commands, stream_t->full_command_size*sizeof(char*));
	  int j;
	  for(j = 0; j < 5; j++)
	    {
	      stream_t->full_commands[stream_t->full_command_position+j+1] = malloc(10 * sizeof(char));
	    }
	}
      if(stream_t->full_commands[stream_t->full_command_position][command_position] == (command_size - 1))
	{
	  command_size += 10;
	  stream_t->full_commands[stream_t->full_command_position] = realloc(stream_t->full_commands[stream_t->full_command_position], command_size * sizeof(char));
	}
	
      if(next_byte == '#') //have to account for the end of comments
	{
	  if(previous_byte == ' ' || previous_byte == '\n' || previous_byte == '\t' || previous_byte == 0)
	    {
	      start_comment = 1;
	      continue;
	    }
	  else
	    {
	      fprintf(stderr, "%d: Syntax Error\n", line_number);
	       exit(0);
	    }
	} 

        if((is_valid_token(next_byte) != 1) && (is_valid_character(next_byte) != 1) && (next_byte != ' ') && (next_byte != '\n') && (next_byte != '\t'))
	{
	  if(start_comment == 0)
	    {
	      fprintf(stderr, "%d: Syntax Error\n", line_number);
	       exit(0);
	    }
	  else
	    {
	      continue;
	    }
	}
	
	if(((is_valid_character(next_byte)== 1) || (next_byte == ' ') || (next_byte == '\t')) && start_comment == 0)
	  {
	    stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
	    command_position++;
	    if(is_valid_character(next_byte) == 1)
	      {
		last_newline = 0;
		previous_character = next_byte;
		previous_byte = next_byte;
		continue;
	      }
	    else
	      {
		previous_byte = next_byte;
	      }
	  }
	//satisfied up to this point
	
	if((is_valid_token(next_byte) == 1) && (start_comment == 0))
	  {
	    switch(next_byte)
	      {
	      case ';' :
		if(is_valid_semicolon(stream_t->full_commands[stream_t->full_command_position]) == 1)
		  {
		    last_newline = 0;
		    stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		    command_position++;
		    previous_character = next_byte;
		    previous_byte = next_byte;
		    continue;
		  }
		else
		  {
		    fprintf(stderr, "%d: Syntax Error\n", line_number);
		    exit(0);
		  }
		break;
	      case '|' : //at the end of full commands, check to make sure there are enough pipes
		if((previous_character == '|') && (is_valid_character(previous_byte) != 1) && (previous_byte != '|'))
		  {
		    fprintf(stderr, "%d: Syntax Error\n", line_number);
		    exit(0);
		  }
		if(last_newline == 1)
		  {
		    fprintf(stderr, "%d: Syntax Error\n", line_number);
		    exit(0);
		  }
		if(previous_character == '|')
		  {
		    if(pipeline_count == 1)
		      {
			fprintf(stderr, "%d: Syntax Error\n", line_number);
			exit(0);
		      }
		    pipeline_count++;
		    last_newline = 0;
		    stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		    command_position ++;
		    previous_character = next_byte;
		    previous_byte = next_byte;
		    continue;
		  }
		pipeline_count = 0;
		last_newline = 0;
		stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		command_position ++;
	        previous_character = next_byte;
		previous_byte = next_byte;
		continue;
		break;
	      case '&':
		if((previous_character == '&') && (previous_byte != '&'))
		  {
		    fprintf(stderr, "%d: Syntax Error\n", line_number);
		    exit(0);
		  }
	       	if(last_newline == 1)
		  {
		    fprintf(stderr, "%d: Syntax Error\n", line_number);
		    exit(0);
		  }
		if(previous_byte == '&')
		  {
		    if(and_symbols == 1)
		      {
			fprintf(stderr, "%d: Syntax Error\n", line_number);
			exit(0);
		      }
		    and_symbols++;
		    last_newline = 0;
		    stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		    command_position ++;
		    previous_character = next_byte;
		    previous_byte = next_byte;
		    continue;
		  }
		last_newline = 0;
		stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		command_position ++;
	        previous_character = next_byte;
		previous_byte = next_byte;
		continue;
		break;
	      case ')':
	      case '(': //check to see that there are enough parens
		last_newline = 0;
		stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		command_position++;
		previous_character = next_byte;
		previous_byte = next_byte;
		complete_subshell++;
		continue;   
		break;
	      case '<':
	      case'>':
		last_newline = 0;
		stream_t->full_commands[stream_t->full_command_position][command_position] = next_byte;
		command_position++;
		previous_character = next_byte;
		previous_byte = next_byte;
		continue;
		break;
	      }
	  }

	if(next_byte == '\n')
	  {
	    if(start_comment == 1)
	      {
		last_newline = 1;
		line_number++;
		start_comment = 0;
		continue;
	      }
	    if(previous_character == '<' || previous_character == '>')
	      {
		fprintf(stderr,"%d: Syntax Error\n", line_number);
		exit(0);
	      }
	    if(last_newline == 1)
	      {
		last_newline = 0;
		correct_command = 1;
		line_number++;
		stream_t->full_command_position++;
		command_position = 0;
		continue;
	      }
	    last_newline = 1;
	    continue;
	  }
	
	if((correct_command == 1) && (is_correct_command(stream_t->full_commands[(stream_t->full_command_position)-1]) != 1))
	  {
	    fprintf(stderr, "%d: Syntax Error", line_number);
	    exit(0);
	  }
    
    }
	  
  int j; 
  char *ptr;
  for(j=0; j <= stream_t->full_command_position; j++)
    {
      ptr = stream_t->full_commands[j];
      int m;
      for(m = 0; ptr[m] != '\0'; m++)
	{
	      printf("%c", ptr[m]);
	   
	}
      printf("\n");
    }
 
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
  if( s->full_command_position < s->array_size) {
    cmd = parse_command_stream(s->full_commands[s->full_command_position]);
    s->full_command_position++;
  } else {
    cmd = NULL;
  }
  return cmd;
}


