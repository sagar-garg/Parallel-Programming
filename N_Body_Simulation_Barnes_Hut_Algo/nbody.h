#ifndef _NBODY_H
#define _NBODY_H

#include <stdbool.h>

// How far do two nodes have to be apart before low resolution information is consulted
// instead. This is basically low enough that everything which is not adjacent is far enough apart.
#define DISTANCE_CUTOFF 10.f

// What is the maximum number of objects leaving their domain in one step. (This is not per domain,
// but in total.)
#define MAX_LEAVING 128

typedef struct Node Node;

typedef struct Tree {
    Node* lower_left;
    Node* lower_right;
    Node* upper_left;
    Node* upper_right;
} Tree;

typedef struct Leaf {
    // Position (x,y), velocity (x,y), mass and number of objects inside this node
    float* x;
    float* y;
    float* vx;
    float* vy;
    float* mass;
    int count; // How many objects are in this node
    int capacity; // The capacity for the above arrays, i.e. for how many objects is there memory in this node

    // The timestep at which this data has last been updated
    int timestamp_high_res;
} Leaf;

typedef struct Node {
    int id; // Leaves always have ids 0 to num_leaves-1, the other ids are given to the trees
    bool is_leaf; // You can probably guess what this means

    // This contains the weighted average position (x,y) and total mass of all objects inside this
    // node. (A node is weighted by its mass, so this is a weighted average in the most literal
    // sense.)
    float low_res_x, low_res_y, low_res_mass;
    int timestamp_low_res; // The timestep at which the low_res data has last been updated
    
    // Domain boundary. This is a rectangle in which every point in this node resides.
    float x0, y0, x1, y1;
    
    Node* parent;
    // Depending on whether is_leaf is true or false, either one contains valid data.
    union {
        Tree tree;
        Leaf leaf;
    };
} Node;

// Ensures that there is enough space in node for it to contain capacity nodes _in total_. If
// necessary, this allocates new memory and moves the old objects.
void node_leaf_ensure_capacity(Node* node, int capacity);

// Add an object with the specified parameters to the node. If necessary, this causes a reallocation
// of the arrays.
void node_leaf_append_object(Node* node, float x, float y, float vx, float vy, float mass);

// Initialisation. You probably do not want to call this.
Node* node_create(int num_leaves, int points);

// Write an SVG to the specified path which visualises the current state of the node. For debugging
// purposes.
void node_draw_svg(Node* node, char* file_name);

// Correctness check. You probably do not want to call this.
void node_check_almost_equal(Node* a, Node* b, float max_err);

// Calculate the distance between things
float distance_point_point(float x0, float y0, float px, float py);
float distance_rectangle_point(float x0, float y0, float x1, float y1, float px, float py);
float distance_rectangle_rectangle(float x0, float y0, float x1, float y1, float px0, float py0, float px1, float py1);
float distance_node_node(Node* node0, Node* node1);

// Return whether the point is inside the node.
bool node_contains_point(Node* node, float px, float py);

// Return a list of all leaves descendent of root that are close enough to node to fall within the
// DISTANCE_CUTOFF limit. 'Return' in this case means that a pointer to an array will be placed in
// *nearby, while the length of that array is written to *nearby_count.
//  The node itself will not be contained in that list.
void node_find_nearby(Node** nearby, int* nearby_count, Node* root, Node* node);

// Return the node with id id if it exists. Else, return NULL
Node* node_find(Node* root, int id);

// Update the velocities of all objects by applying the gravitational pull of all objects in source
// to node. Consequently, _only_ the objects in node are changed! This does not move anything.
void compute_acceleration(Node* node, Node* source);

// Update low-resolution information for node. That means that for trees we combine the
// low-resolution data of their four children. For leaves we calculate it from the high-resolution
// data, UNLESS the low-resulution data has a higher timestamp, in which case we silently do
// nothing.
void node_update_low_res(Node* node);

// Move all objects in node node according to their velocities. leaving should point to an array of
// size 5 * MAX_LEAVING, which will be populated with the data of objects that have left the region of
// their nodes. (These objects are removed from their respective nodes.) The number of _objects_ in
// the leaving array is returned, meaning that there are 5 * <result> float values set in leaving.
//  For each object in leaving we store five floats, which correspond to (x, y, vx, vy, mass) in this
// order.
int move_objects(Node* node, float* leaving);

// Whether the gui is shown.
extern bool global_show_gui;
// How much to move in a single step of the simulation
extern float global_timestep;

#endif
