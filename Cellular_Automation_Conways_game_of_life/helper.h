#ifndef _HELPER_H_LIFE
#define _HELPER_H_LIFE

#include<time.h>

extern int global_show_gui;

void save_to_file(int height, int width, int grid[height][width], char const *filename );
void copy_edges ( int height, int width, int grid[height][width] );
void initialize_grid( int height, int width, int grid[height][width] );
void evolve( int height, int width, int grid[height][width] );
void print_grid(int height, int width, int grid[height][width]);
int compare_grids(int height, int width, int grid_seq[height][width], int grid_par[height][width], int *row, int *col);
double time_diff(const struct timespec *first, const struct timespec *second, struct timespec *diff);

#endif
