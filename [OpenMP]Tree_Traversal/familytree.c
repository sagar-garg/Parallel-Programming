#include "familytree.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void fill(tree *node, int level){
    
    node->father = NULL;
    node->mother = NULL;
    
    node->id = g_node_id;
    sprintf(node->name, "%s%d", names[g_node_id % 31], g_node_id/31);
    node->data = (113*(g_node_id + node->name[1] * g_node_id / 31)) % 91137;
    node->IQ = 0;
    g_node_id++;
    
    if (level < MAXLEVEL-1) {
        node->father = malloc(sizeof(tree));
        node->mother = malloc(sizeof(tree));
        fill(node->father, level+2);
        fill(node->mother, level+1);
    }
}

void initialize(tree *node) {
    g_node_id = 0;
    fill(node, 0);
}

void tearDown(tree *node) {
    if (node != NULL) {
        tearDown(node->father);
        tearDown(node->mother);
        free(node);
    }
}

uint64_t scramble(uint64_t val, int iter) {
    val += 3405695742;
    for (int i = 0; i < iter; ++i) {
        val ^= val << 13;
        val ^= val >> 7;
        val ^= val << 17;
    }
    return val * 99989;
}

int compute_IQ(int data, int father_iq, int mother_iq) {
    // Nothing to see here, this is just an expensive function.
    
    uint64_t x = scramble(father_iq + mother_iq - data*100, 200000);
    x = (x >> 32) * (x & -1) >> 32;
    x = (x*x >> 32) * (x*x >> 32) >> 32;
    x = (x*x >> 32) * (x*x >> 32) >> 32;
    x = (x*x >> 32) * (x*x >> 32) >> 32;
    x = (x*x >> 32) * (x*x >> 32) >> 32;
    return 100 + (x >> 26);
}
