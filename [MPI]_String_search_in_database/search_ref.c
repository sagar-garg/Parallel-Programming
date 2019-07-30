#include <string.h>
#include<stdio.h>
#include "search.h"
#include "helper.h"

void search_text_ref (char* text, int num_lines, int line_length, char* search_string, int *occurences)
{
  /*
    Counts occurences of substring "search_string" in "text". Texts contains multiple lines and each line
    has been placed text + line_length * num_lines since line length in the original text can vary.

    Note that "search_string" need not be a word. For example, searching for "my" in "myelin sheath" will return 1.

    Writes result at location pointed to by "occurences".
   */

  int running_count = 0;
  for (int i = 0; i < num_lines; i++)
    {
      running_count += count_occurences(text + i * line_length, search_string);
    }

  *occurences = running_count;
}
