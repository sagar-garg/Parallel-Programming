#include "familytree.h"

int traverse(tree *node, int num_threads) {
    if (node == NULL) return 0;

    int father_iq, mother_iq;
    
    father_iq = traverse(node->father, num_threads);
    mother_iq = traverse(node->mother, num_threads);
    
    node->IQ = compute_IQ(node->data, father_iq, mother_iq);
    genius[node->id] = node->IQ;
    return node->IQ;
}

