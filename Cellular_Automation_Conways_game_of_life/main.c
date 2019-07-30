#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<time.h>
#include"life.h"
#include"helper.h"
#include"gui.h"

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  int rank, num_procs;

  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int width = 10;
  int height = 10;

  int num_iterations = 10;
  int (*grid)[width] = NULL;

  int show_gui = 0;
  if (argc > 1) {
      if (argc == 2 && strcmp(argv[1], "-g") == 0) {
          show_gui = 1;
      } else {
          fprintf(stderr, "Usage:\n  %s [-g]\n", argv[0]);
          exit(153);
      }
  }
  
  if (rank == 0)
    {
      if (show_gui)
        {
          gui_create_window(1, argv);
        }
        
      grid = malloc ( sizeof( int[height][width] )) ;

      if (grid == NULL )
        {
          fprintf(stderr, "Failed to allocate memory. Aborting.\n");
          //      MPI_Abort(MPI_COMM_WORLD, 1);
          exit(1);
        }

      initialize_grid(height, width, grid);
    }
  else
    {
      show_gui = 0;
    }

  global_show_gui = show_gui;

  struct timespec start, stop;

  clock_gettime(CLOCK_MONOTONIC, &start);
  simulate(height, width, grid, num_iterations);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  if (rank == 0)
    {
      printf("Time: %lf seconds\n", time_diff(&start, &stop, NULL));
    }

  free (grid);
  MPI_Finalize();

  return 0;
}

