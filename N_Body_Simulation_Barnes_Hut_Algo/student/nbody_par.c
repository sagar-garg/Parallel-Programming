#include "nbody.h"
#include <stdlib.h>
#include<mpi.h>
#include<stdio.h>

void simulate_par(int steps, int num_leaves, Node* root) {
   int rank, num_procs;
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    Node* node = node_find(root,rank);
    Node** nearby = malloc(sizeof(Node*) * num_leaves);
    int nearby_count = 0;
    node_find_nearby(nearby, &nearby_count, node, root);

    float leaving[5*MAX_LEAVING];
    float nrecv;
    
    for (int step = 0; step < steps; ++step) {
        // Do the n-body computation
        compute_acceleration(node, root);
   		int left = move_objects(node, leaving);

   		float* send_array;
        send_array= (float *) calloc ((5*left+1)*num_leaves, sizeof(float));   

        // Re-distribute all the objects that have left their original boundary
        for (int i = 0; i < left; ++i) {
            for (int n = 0; n < nearby_count; ++n) {
                if (node_contains_point(nearby[n], leaving[5*i], leaving[5*i+1])) {
                	++send_array[nearby[n]->id * (5 * left + 1)]; 
                    send_array[(nearby[n]->id * (5 * left + 1) + 1) + 5*i] = leaving[5*i];
                    send_array[(nearby[n]->id * (5 * left + 1) + 2) + 5*i] = leaving[5*i + 1];
                    send_array[(nearby[n]->id * (5 * left + 1) + 3) + 5*i] = leaving[5*i + 2];
                    send_array[(nearby[n]->id * (5 * left + 1) + 4) + 5*i] = leaving[5*i + 3];
                    send_array[(nearby[n]->id * (5 * left + 1) + 5) + 5*i] = leaving[5*i + 4];                    
                    break;
                }
            }
        }

       
        // exchanging particle objects between processors
        for (int n = 0; n < nearby_count; n++){
        	MPI_Sendrecv(&send_array[nearby[n]->id * (5 * left + 1)],1,MPI_FLOAT,nearby[n]->id,0,
            &nrecv,1,MPI_FLOAT,nearby[n]->id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        	if (send_array[nearby[n]->id * (5 * left + 1)] == 0 && nrecv == 0){
        		continue;
        	}

        	float* recv_array = malloc(sizeof(float) * nrecv * 5);
        	MPI_Sendrecv(&send_array[nearby[n]->id * (5 * left + 1) + 1],5 * send_array[nearby[n]->id * (5 * left + 1)],MPI_FLOAT,nearby[n]->id,0,
            &recv_array[0],5 * nrecv,MPI_FLOAT,nearby[n]->id,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        	//printf("Check\n");

        	if (nrecv > 0){
        		for (int k = 0; k < nrecv; k++){
        			node_leaf_append_object(node, recv_array[5 * k],recv_array[5 * k + 1],recv_array[5 * k + 2],
                    recv_array[5 * k + 3],recv_array[5 * k + 4]);
        		}
			}	
        	free(recv_array);
        }
        free(send_array);

        // broadcasting new Center of mass data in the form of the tree structure
        for (int j = 0; j < num_procs; j++){
            Node* sending_node = node_find(root,j);

        	MPI_Bcast(&sending_node->leaf.count, 1, MPI_INT, j, MPI_COMM_WORLD);
        	MPI_Bcast(sending_node->leaf.x, sending_node->leaf.count, MPI_FLOAT, j, MPI_COMM_WORLD);
        	MPI_Bcast(sending_node->leaf.y, sending_node->leaf.count, MPI_FLOAT, j, MPI_COMM_WORLD);
        	MPI_Bcast(sending_node->leaf.vx, sending_node->leaf.count, MPI_FLOAT, j, MPI_COMM_WORLD);
        	MPI_Bcast(sending_node->leaf.vx, sending_node->leaf.count, MPI_FLOAT, j, MPI_COMM_WORLD);
        	MPI_Bcast(sending_node->leaf.mass, sending_node->leaf.count, MPI_FLOAT, j, MPI_COMM_WORLD);

       		if (rank != j)
       		++sending_node->leaf.timestamp_high_res;

        }
        node_update_low_res(root);       
    }
}