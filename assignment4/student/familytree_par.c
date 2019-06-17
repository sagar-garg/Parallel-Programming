#include "familytree.h"
#include <omp.h>

int parallel_traverse(tree *node) {
    if (node == NULL) return 0;

    int father_iq, mother_iq;
    
    #pragma omp task shared(father_iq)
    father_iq = parallel_traverse(node->father);
    mother_iq = parallel_traverse(node->mother);

    #pragma omp taskwait
    
    node->IQ = compute_IQ(node->data, father_iq, mother_iq);
    genius[node->id] = node->IQ;
    return node->IQ;
}

int traverse(tree *node, int numThreads){
    #pragma omp parallel
    {
        #pragma omp single
        parallel_traverse(node);
    }
    return node->IQ;
}
