#include<stdlib.h>
#include"helper.h"
#include"life_ref.h"

void simulate_ref(int height, int width, int grid[height][width], int num_iterations)
{
  /*
  */
  copy_edges(height, width, grid );
  for (int i = 0; i < num_iterations; i++)
    {
      evolve( height, width, grid );
    }

}
