#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<mpi.h>
#include "helper.h"
#include "search.h"

// Max sub string size
#define SUB_STRING_SIZE 40

int main( int argc, char** argv)
{

  int rank, num_procs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  char* search_string = (char*) malloc( sizeof(char) * SUB_STRING_SIZE) ;
  int num_lines;
  int line_length;
  char *text = NULL;

  // read data on rank 0 only
  if (rank == 0)
    {
      int search_string_length;
      char **line_pointers = NULL;
      char* buffer = NULL;
      char* filename;
      FILE *file_pointer = NULL;


      if ( argc != 3 )
        {
          fprintf(stderr, "Usage: search_{seq|par} filename search_string\n");
          MPI_Abort(MPI_COMM_WORLD,1);
        }

      filename = argv[1];
      search_string_length = strlen(argv[2]);

      if ( search_string_length >= SUB_STRING_SIZE-1)
        {
          fprintf(stderr, "Search string size should be less than SUB_STRING_SIZE characters \n");
          MPI_Abort(MPI_COMM_WORLD, 1);
        }
      memcpy(search_string, argv[2], search_string_length + 1); 

      file_pointer = fopen(filename, "r");

      if (file_pointer == NULL)
        {
          printf("Failed to open file: %s \n", filename);
          MPI_Abort(MPI_COMM_WORLD, 1);
        }

      // find file size
      // Assuming SEEK_END is defined
      fseek(file_pointer, 0L, SEEK_END);
      int text_size = ftell(file_pointer) + 1; // +1 accounts for \0
      fseek(file_pointer, 0L, SEEK_SET);

      // create buffer
      buffer = (char* ) malloc (sizeof(char) * text_size);

      if ( buffer == NULL )
        {
          fprintf(stderr, "Could not allocate buffer for text file. Exiting.\n");
          MPI_Abort(MPI_COMM_WORLD, 1);
        }

      // read file
      int elements_read = fread( buffer, sizeof(char), text_size, file_pointer);
      if (elements_read != text_size-1)
        {
          fprintf(stderr, "Error reading from file: %s. Terminating.\n", filename);
          MPI_Abort(MPI_COMM_WORLD, 1);
        }

      // set null
      buffer[elements_read] = '\0';

      // break text into lines
      num_lines = create_lines(buffer);
      line_pointers = (char **)malloc( sizeof(char*) * num_lines );

      if (line_pointers == NULL)
        {
          fprintf(stderr, "Failed to allocate line pointers. Terminating program.\n");
          MPI_Abort (MPI_COMM_WORLD, 1);
        }

      set_line_pointers(buffer, line_pointers, num_lines );
      line_length = get_max_line_length(line_pointers, num_lines);

      // copy data to new buffer
      text = (char*)malloc( sizeof(char) * num_lines * line_length);
      if (text == NULL)
        {
          fprintf(stderr, "Failed to allocate text. Terminating program.\n");
          MPI_Abort (MPI_COMM_WORLD, 1);
        }
      // copy data from line pointers to text
      for (int i = 0; i < num_lines; i++)
        {
          strcpy(text + line_length * i, line_pointers[i]);
        }

      free(line_pointers);
      free(buffer);
      fclose(file_pointer);

      // Broadcast results
      MPI_Bcast(search_string, SUB_STRING_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
      MPI_Bcast(&num_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&line_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
      
    }
  else
    {
      // Matching Bcasts from other processes
      MPI_Bcast(search_string, SUB_STRING_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
      MPI_Bcast(&num_lines, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&line_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

  int occurences;

  struct timespec start, stop;

  clock_gettime(CLOCK_MONOTONIC, &start);
  
  //int lines_per_proc=num_lines/num_procs;
  search_text( text, num_lines, line_length, search_string, &occurences);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  if (rank == 0) {
    printf("Time: %lf seconds\n", time_diff(&start, &stop, NULL));
    printf("Found %d occurence(s) of string '%s'\n", occurences, search_string);  
  }


  free(text);
  free(search_string);

  MPI_Finalize();

  return 0;
}
