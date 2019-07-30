#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include"life.h"
#include"life_ref.h"
#include"helper.h"

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  int rank, num_procs;

  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // consider making this adjustable
  int width = 300;
  int height = 300;

  int num_iterations = 1000;

  int (*grid_seq)[width] = NULL;
  int (*grid_par)[width] = NULL;

  if (rank == 0)
    {
      grid_seq = malloc ( sizeof( int[height][width] )) ;
      grid_par = malloc ( sizeof( int[height][width] )) ;

      if (grid_seq == NULL || grid_par == NULL)
        {
          fprintf(stderr, "Failed to allocate memory. Aborting.\n");
          MPI_Abort(MPI_COMM_WORLD, 1);
        }

      initialize_grid(height, width, grid_seq);

      // initialize parallel grid
      memcpy( &(grid_par[0][0]), &(grid_seq[0][0]), sizeof(int) * height * width);

      // get sequential results
      simulate_ref(height, width, grid_seq, num_iterations);
    }

  simulate(height, width, grid_par, num_iterations);

  int exit_status;

  if (rank == 0)
    {
      int row;
      int col;
      int diff = compare_grids(height, width, grid_seq, grid_par, &row, &col);

      if ( diff == 0 )
        {
          printf("Unit test passed\n");
          exit_status = EXIT_SUCCESS;
        }
      // differences in parallel and sequential version
      else
        {
          printf("Unit test failed. First difference at row %d, col %d\n", row, col );
          printf("Hint: You can use the save_to_file function in helper.c to inspect the state of the grid.\n");
          exit_status = EXIT_FAILURE;
        }
    }
  // for processes other than root, return success
  else
    {
      MPI_Finalize();
      return EXIT_SUCCESS;
    }

  free (grid_par);
  free (grid_seq);
  MPI_Finalize();

  return exit_status;
}
