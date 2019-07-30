#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<string.h>
#include"helper.h"
#include"life.h"
#include"gui.h"
#include <math.h>

void simulate(int height, int width, int grid[height][width], int num_iterations)
{
  /*
    Write your parallel solution here. You first need to distribute the data to all of the
    processes from the root. Note that you cannot naively use the evolve function used in the
    sequential version of the code - you might need to rewrite it depending on how you parallelize
    your code.

    For more details, see the attached readme file.
  */


  int num_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Status *status=NULL;
  int height_per_proc=ceil((double)height/(double)num_procs);

  //int height_last= height-(height_per_proc)*(num_procs-1);
  //printf("h1= %d, h2=%d \n, r1=%d", height, height_per_proc, height_last);

  int send_count = width*height_per_proc;

int itop, ibottom, rank_b, rank_t;

int *bufsend = (int*)malloc((width)*sizeof(int)); //including padding
int *bufrecv = (int*)malloc((width)*sizeof(int));
int *big_array = (int *)malloc(sizeof(int) * (height)*(width));
int *small_array= (int*)malloc((send_count)*sizeof(int)); //array

//making a matrix
int (*grid_local)[width] = NULL;
grid_local = malloc ( sizeof( int[height_per_proc][width] )) ;
//int *grid_local= (int*)malloc(sizeof(int[height_per_proc][width])); //matrix

  
//converting matrix to array
  if (rank==0){
  

 for (int i=0; i<height; i++){
   for (int j=0; j<width; j++){
     big_array[i*(width)+j]=grid[i][j];
   }
 }

printf("\n");

for (int i=0; i<height*width; i++)
printf("%d", big_array[i]);
}

//passing array to all processors
MPI_Scatter(big_array, send_count, MPI_INT, small_array, send_count, MPI_INT, 0, MPI_COMM_WORLD); 

//for last processor

//  if (rank==num_procs-1)
//{  height_per_proc=height-(height_per_proc)*(num_procs-1); }

/*
if (rank==1){
for (int i=0; i<height_per_proc*width; i++)
{printf("%d", small_array[i]);}
printf("\n");
}
*/

for (int i=0; i<height_per_proc; i++){
   for (int j=0; j<width; j++){
     grid_local[i][j]=small_array[i*(width)+j];
   }
 }


/* for last processor
if (rank==num_procs-1){
  int height_last=height-(height_per_proc)*(num_procs-1);
for (int i=0; i<height_last; i++){
   for (int j=0; j<width; j++){
     grid_local[i][j]=small_array[i*(width)+j];
   }
 }
}
*/

   print_grid(height_per_proc, width, grid_local);
   printf("\n");

   

ibottom=rank*height_per_proc;

if (rank!=num_procs-1)
{itop=ibottom+height_per_proc-1;}
else
{itop=height-1;}

if (rank==0) 
{rank_b=num_procs-1;}
else 
{rank_b=rank-1;}


if (rank==num_procs-1)
{rank_t=0;}
else 
{rank_t=rank+1;}

//sending bottom. first for all. helps in clearing commincation pipeline.
for (int j=0; j<width; j++)
bufsend[j]=grid[ibottom][j];

MPI_Send(bufsend, width-1, MPI_INT, rank_b, 1, MPI_COMM_WORLD);
MPI_Recv(bufrecv, width-1, MPI_INT, rank_t, 1, MPI_COMM_WORLD, status);

for (int j=0; j<width; j++)
grid[itop][j]=bufrecv[j];


//sending top
for (int j=0; j<width; j++)
bufsend[j]=grid[itop][j];

MPI_Send(bufsend, width-1, MPI_INT, rank_t, 2, MPI_COMM_WORLD);
MPI_Recv(bufrecv, width-1, MPI_INT, rank_b, 2, MPI_COMM_WORLD, status);

for (int j=0; j<width; j++)
grid[ibottom][j]=bufrecv[j];


//communcaition done. now evolve function
int (*temp)[width] = malloc( sizeof ( int[height_per_proc][width] ) );

  for( int i = 1; i < height_per_proc - 1; i++)
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

  








