
#include <stdlib.h>
#include <stdio.h>

#include "nbody.h"

void simulate_seq(int steps, int num_leaves, Node* root) {
    Node* node = root;
    
    Node** nearby = malloc(sizeof(Node*) * num_leaves);
    int nearby_count = 0;
    node_find_nearby(nearby, &nearby_count, node, root);
    
    float leaving[5*MAX_LEAVING];
    
    for (int step = 0; step < steps; ++step) {
        // Do the n-body computation
        compute_acceleration(node, root);
        int left = move_objects(node, leaving);
        //printf("leaving = %d /n", left);

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
        
        // Update bookkeeping data
        node_update_low_res(root);
    }
}
