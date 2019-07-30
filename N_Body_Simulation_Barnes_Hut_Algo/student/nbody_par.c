#include "nbody.h"
#include <stdlib.h>
#include<mpi.h>
#include<stdio.h>


void acceleration_check(Node* node){
	for (int j = 0; j < node->leaf.count; ++j) {
		printf("%f %f \n",node->leaf.vx[j],node->leaf.vy[j]);
	}

	// Running Sequential Version



}

void simulate_par(int steps, int num_leaves, Node* root) {
    // TODO

    // You do not need to call MPI_Init and MPI_Finalize, this is already done.

    // root is the root node of the whole domain. The data will be initialised in each process, so
    // you do not need to copy it at the start. However, the main process (rank 0) has to have the
    // full data at the end of the simulation, so you will need to collect it.

	// MPI Variables
	int rank, num_procs;

  	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Allocation of Leaves by ID to the respective rank of process
    Node* node = node_find(root,rank);
    //printf("%i %i\n",node->id,rank );

    // Nearest Neighbours Node Array
    Node** nearby = malloc(sizeof(Node*) * num_leaves);
    // Number of Nearest Neighbours counter
    int nearby_count = 0;
    // Finding the Nearest Neighbours
    node_find_nearby(nearby, &nearby_count, node, root);

    //printf("%i \n",nearby_count ); // Max is 11

    // Leaving Points Information
    float leaving[5*MAX_LEAVING];

    for (int step = 0; step < steps; ++step) {
        // Do the n-body computation
        compute_acceleration(node, root);
        int left = move_objects(node, leaving);

        // Check for Correct Calculation for Leaves

        // Re-distribute all the objects that have left their original boundary
        for (int i = 0; i < left; ++i) {
            for (int n = 0; n < nearby_count; ++n) {
                if (node_contains_point(nearby[n], leaving[5*i], leaving[5*i+1])) {
                    // put object into node nearby[n]
                    node_leaf_append_object(nearby[n], leaving[5*i], leaving[5*i+1],
                        leaving[5*i+2], leaving[5*i+3], leaving[5*i+4]);
                    break;
                }
            }

            // If the object is not within the boundary of any node, it has left
            // the domain and will be discarded.
        }
                printf("Check\n");

        // Update bookkeeping data
        node_update_low_res(node);
    }

    // Collect all the Results Together

}
