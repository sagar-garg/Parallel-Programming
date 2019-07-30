#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<string.h>
#include"helper.h"
#include"life.h"
#include"gui.h"

void p_evolve( int height, int width, int grid[height][width], int (*temp)[width])
{
  /*
    Apply the game of life rules on a Torus --> grid contains shadow rows and columns
    to simplify application of rules i.e. grid actually ranges from grid [ 1.. height - 2 ][ 1 .. width - 2]
  */

  // pointer to v

  for( int i = 1; i < height-1; i++)
    for ( int j = 1; j < width - 1; j++ )
      {

        int sum = grid[i-1][j-1]  + grid[i-1][j] + grid[i-1][j+1] + \
          grid[i][j-1] + grid[i][j+1] + \
          grid[i+1][j-1] + grid[i+1][j] + grid[i+1][j+1];

        if ( grid[i][j] == 0 )
          {
            // reproduction
            if (sum == 3)
              {
                temp[i][j] = 1;
              }
            else
              {
                temp[i][j] = 0;
              }
          }
        // alive
        else
          {
            // stays alive
            if (sum == 2 || sum == 3)
              {
                temp[i][j] = 1;
              }
            // dies due to under or overpopulation
            else
              {
                temp[i][j] = 0;
              }
          }

      }

  copy_edges(height, width, temp);

  for (int i = 0; i < height ; i++)
    for(int j = 0; j < width ; j++)
      {
        grid[i][j] = temp[i][j];
      }


}

void simulate(int height, int width, int grid[height][width], int num_iterations)
{
  /*
    Write your parallel solution here. You first need to distribute the data to all of the
    processes from the root. Note that you cannot naively use the evolve function used in the
    sequential version of the code - you might need to rewrite it depending on how you parallelize
    your code.

    For more details, see the attached readme file.
  */
  int rank, num_procs;
  int proc_top, proc_bot;
  int h_b;
  int h_t;
  // int w_r, w_l;
  int h_chunk;

  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int * sendbuffer = (int *) malloc(sizeof(int)*width);
  int * recvbuffer = (int *) malloc(sizeof(int)*width);

  if (rank < num_procs-1) proc_top = rank + 1;
  else proc_top = 0;
  if (rank > 0) proc_bot = rank - 1;
  else proc_bot = num_procs - 1;
  
  h_b = rank * ((height-2)/num_procs) + 1;
  h_t = (rank + 1) * ((height-2)/num_procs);
  if (rank == num_procs - 1) {
    h_t = height-2;
  }
  h_chunk = h_t - h_b + 1;

  int * disps = NULL;
  int * recvcounts = NULL;
  if (rank == 0) {
    // printf("TOTAL: %d\n", width*(height-2));
    disps = (int*) malloc(sizeof(int)*num_procs);
    recvcounts = (int*) malloc(sizeof(int)*num_procs);
    for (int i = 0; i < num_procs; i++) {
      disps[i] = (i * h_chunk + 1) * width;
      if (i < num_procs-1)
        recvcounts[i] = ((height-2)/num_procs) * width;
      else
        recvcounts[i] = ((height-2) - i * ((height-2)/num_procs))  *  width;
      // printf("disp[%d] = %d, count[%d] = %d\n",i, disps[i] ,i,recvcounts[i]);
    }
  }
  
  if (rank != 0) {
    grid = malloc ( sizeof( int[h_chunk+2][width] ));
  }
  int (*temp)[width] = malloc( sizeof ( int[h_chunk+2][width] ) );
  
  
  MPI_Scatterv(&grid[0][0], recvcounts, disps, MPI_INT, &grid[1][0], h_chunk*width, MPI_INT, 0, MPI_COMM_WORLD);
 
  copy_edges(h_chunk+2, width, grid );
  for (int k = 0; k < num_iterations; k++)
    {
      for (int i = 0; i < width; i++) {
        sendbuffer[i] = grid[h_chunk][i];
      }
      MPI_Sendrecv(sendbuffer, width, MPI_INT, proc_top, 0, recvbuffer, width, MPI_INT, proc_bot, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (int i = 0; i < width; i++) {
        grid[0][i] = recvbuffer[i];
        sendbuffer[i] = grid[1][i];
      }
      MPI_Sendrecv(sendbuffer, width, MPI_INT, proc_bot, 0, recvbuffer, width, MPI_INT, proc_top, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (int i = 0; i < width; i++) {
        grid[h_chunk+1][i] = recvbuffer[i];
      }
      p_evolve( h_chunk+2, width, grid , temp);
    }
  MPI_Gatherv(&grid[1][0], h_chunk*width, MPI_INT, &grid[0][0], recvcounts, disps, MPI_INT, 0, MPI_COMM_WORLD);

  free(temp);
  // if (rank == 0) save_to_file(height, width, grid, "par2.txt");  
  // if (rank == 0) copy_edges(height, width, grid);
}
