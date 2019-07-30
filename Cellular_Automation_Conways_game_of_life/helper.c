#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include "helper.h"
#include "gui.h"

void copy_edges ( int height, int width, int grid[height][width] )
{
  /*
    Copies data from the inner part of the grid to
    shadow (padding) rows and columns to transform the grid into a torus.
  */


  // Copy data to the boundaries
  for( int i = 1; i < height-1; i++ )
    {
      // join rows together
      grid[i][0] = grid[i][width-2];
      grid[i][width-1] = grid[i][1];
    }

  for ( int j = 1; j < width - 1; j++)
    {
      // join columns together
      grid[0][j] = grid[height-2][j];
      grid[height-1][j] = grid[1][j];
    }

  // Fix edges
  grid[0][0] = grid[height-2][width-2];
  grid[height-1][width-1] = grid[1][1];
  grid[0][width-1] = grid[height-2][1];
  grid[height-1][0] = grid[1][width-2];

}

int global_show_gui;

void evolve( int height, int width, int grid[height][width] )
{
  /*
    Apply the game of life rules on a Torus --> grid contains shadow rows and columns
    to simplify application of rules i.e. grid actually ranges from grid [ 1.. height - 2 ][ 1 .. width - 2]
  */

  // pointer to v
  int (*temp)[width] = malloc( sizeof ( int[height][width] ) );

  for( int i = 1; i < height - 1; i++)
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

  free(temp);

}
void save_to_file( int height, int width, int grid[height][width], char const* filename )
{
  /*
    If you do not want to specify a filename, just pass an empty string.
   */

  int filename_len = strlen(filename);


  if (filename_len == 0)
    {
      filename="./grid.txt";
    }

  FILE *file_pointer;


  file_pointer = fopen(filename, "w");

  if (file_pointer == NULL)
    {
      fprintf(stderr, "Failed to open file.\n");
      exit(1);
    }
  // write header
  fprintf(file_pointer, "%d %d\n", height - 2, width - 2); 

  // write inner values
  for (int i = 1; i < height - 1; i++ )
    {
    for (int j = 1; j < width - 1; j++ )
      {
        fprintf(file_pointer, "%d ", grid[i][j]);
      }
    // new line after every row
    fprintf(file_pointer, "\n");
    }

  fclose(file_pointer);

}


void initialize_grid( int height, int width, int grid[height][width] )
{
  // "random" numbers
  srand(223);

  for( int i = 1; i < height - 1; i++ )
    {
      for (int j = 1; j < width - 1; j++ )
        {
          // divide range into two parts
          float num = (double)rand() / (double)RAND_MAX;
          grid[i][j] = num < 0.5 ? 0 : 1;
        }

    }
  copy_edges(height, width, grid);
}

void print_grid(int height, int width, int grid[height][width])
{

  printf("\n");

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
        {
          printf("%d ", grid[i][j]);
        }
      printf("\n");
    }
}


int compare_grids(int height, int width, int grid_seq[height][width], int grid_par[height][width], int *row, int *col)
{
  /*
    Compare the inner parts of grid_seq with grid_par i.e. excluding the padding rows. Writes the index of first
    difference in "row" and "col".

    Returns
    0: if arrays are equal
    1: if arrays are different
   */

  for (int i = 1; i < height - 1; i++ )
    {
      for (int j = 1; j  < width - 1; j++ )
        {
          if ( grid_seq[i][j] != grid_par[i][j])
            {
              *row = i;
              *col = j;
              return 1;
            }
        }
    }
  return 0;

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
