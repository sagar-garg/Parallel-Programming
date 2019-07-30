#include <string.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "search.h"
#include "helper.h"
#include <math.h>

void search_text (char* text, int num_lines, int line_length, char* search_string, int *occurences)
{
  /*
    Counts occurences of substring "search_string" in "text". "text" contains multiple lines and each line
    has been placed at text + line_length * num_lines since line length in the original text file can vary.
    "line_length" includes space for '\0'.

    Writes result at location pointed to by "occurences".


    *************************** PARALLEL VERSION **************************

    NOTE: For the parallel version, distribute the lines to each processor. You should only write
    to "occurences" from the root process and only access the text pointer from the root (all other processes
    call this function with text = NULL) 
  */

  // Write your parallel solution here
  

  //each processor counts occurence in (num_lines/num_procs) lines. 
  //Allreduce operator adds them up.
  
  int num_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int lines_per_proc=ceil(num_lines/num_procs);
  int send_count=lines_per_proc*(line_length); //number of character arrays that form a chunk to be passed to 1 processor
  //int len= strlen(text);

 char *sub_array = malloc(sizeof(char) * send_count);

//printf("rank=%d, np= %d, lines_per_proc=%d, send_count=%d, line length=%d, strlen=%d \n", 
//rank, num_procs, lines_per_proc, send_count, line_length, len);
  
  MPI_Scatter(text, send_count, MPI_CHAR, sub_array, send_count, MPI_CHAR, 0, MPI_COMM_WORLD);
  
  //printf ("rank= %d, first string is %c \n", rank, sub_array[0]);


  int local_count = 0;
  for (int i = 0; i < lines_per_proc; i++)
    {
      local_count += count_occurences(sub_array + i * line_length, search_string);
    }

  //  printf("rank= %d, local count=%d \n", rank, local_count);

MPI_Reduce( &local_count, occurences, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

//printf ("rank= %d, occurence= %d \n", rank, *occurences);
  

}
