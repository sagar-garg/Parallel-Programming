#include "familytree_ref.h"

int traverse_ref(tree *node) {
    if (node == NULL) return 0;

    int father_iq, mother_iq;
    
    father_iq = traverse_ref(node->father);
    mother_iq = traverse_ref(node->mother);
    
    node->IQ = compute_IQ(node->data, father_iq, mother_iq);
    genius[node->id] = node->IQ;
    return node->IQ;
}
