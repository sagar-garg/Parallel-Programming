#include <string.h>
#include <time.h>
#include<stdlib.h>
#include "helper.h"

int count_occurences(char* string, const char* sub_string)
{
  /*
    Count the occurences of word "sub_string" in "string". Words are separated by spaces, full stops, quotations,commas, semicolons, colons, hyphens, parantheses and question marks. 
  */
  int count = 0;

  const char * delimiters = " .\",;:-?\'()";

  char* string_cpy = (char*)malloc(sizeof(char) * (strlen(string) + 1) );
  strcpy(string_cpy, string);

  char* string_start = strtok(string_cpy, delimiters);

  while( string_start != NULL )
    {
      if (strcmp(string_start, sub_string) == 0)
        {
          count++;
        }
      string_start = strtok(NULL, delimiters);
    }

  free(string_cpy);
  return count;
}

int create_lines(char* text )
{
  /*
    Modifies text to replace endline characters \n with \0. Returns number of lines and also sets lines
    to start of each line.
   */
  int num_lines = 0;

  // loop over text and replace \n with \0
  for ( int i = 0; text[i] != '\0'; i++)
    {
      if (text[i] == '\n')
        {
          num_lines++;
          text[i] = '\0';
        }
    }

  return num_lines;
}

int get_max_line_length(char** line_pointers, int num_lines)
{
  /*
   */

  int max = -1;

  for (int i = 0; i < num_lines; i++)
    {
      int line_length = strlen(line_pointers[i]) + 1;
      if (line_length > max)
        max = line_length;
    }
  return max;
}

void set_line_pointers (char* text, char **lines, int num_lines)
{
  /*
    This function sets each pointer in lines to the start of a line in "text". lines is an array of char pointers.
    "text" contains lines separated by null characters.
  */
  char * line_start = text;

  for( int i = 0; i < num_lines; i++)
    {
      int len = strlen(line_start);
      lines[i] = line_start;
      line_start += len + 1; // + 1 for \0
    }

}

double time_diff(const struct timespec *first, const struct timespec *second, struct timespec *diff)
{
	struct timespec tmp;
	const struct timespec *tmp_ptr;

	if (first->tv_sec > second->tv_sec || (first->tv_sec == second->tv_sec && first->tv_nsec > second->tv_nsec))
    {
      tmp_ptr = first;
      first = second;
      second = tmp_ptr;
    }

	tmp.tv_sec = second->tv_sec - first->tv_sec;
	tmp.tv_nsec = second->tv_nsec - first->tv_nsec;

	if (tmp.tv_nsec < 0)
    {
      tmp.tv_sec -= 1;
      tmp.tv_nsec += 1000000000;
    }

	if (diff != NULL )
    {
      diff->tv_sec = tmp.tv_sec;
      diff->tv_nsec = tmp.tv_nsec;
    }

	return tmp.tv_sec + tmp.tv_nsec / 1000000000.0;
}
