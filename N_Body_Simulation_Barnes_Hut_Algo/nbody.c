#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <mpi.h>

#include "gui.h"
#include "nbody.h"

static uint64_t rand_state = 0xd1620b2a7a243d4bull;
uint64_t random_u64() {
    uint64_t x = rand_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rand_state = x;
    return x * 0x2545f4914f6cdd1dull;
}
uint64_t random_u64_seed(uint64_t seed) {
    uint64_t x = 0xd1620b2a7a243d4bull ^ seed;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545f4914f6cdd1dull;
}
float random_float() {
    return random_u64() / 18446744073709551616.f;
}
float random_float_normal() {
    // Ok, this is cheating.
    return random_float() + random_float() + random_float() + random_float()
         + random_float() + random_float() + random_float() + random_float()
         + random_float() + random_float() + random_float() + random_float() - 6.f;
}

void node_update_low_res_single(Node* node) {
    if (node->is_leaf) {
        // This is not an error, as in the parallel case there are some leaves for which we do not
        // need to high-resolution data at all. In that case the low-resulution data is transmitted
        // via MPI.
        if (node->timestamp_low_res > node->leaf.timestamp_high_res) return;
        
        node->low_res_x    = 0.f;
        node->low_res_y    = 0.f;
        node->low_res_mass = 0.f;
        for (int i = 0; i < node->leaf.count; ++i) {
            node->low_res_x    += node->leaf.x[i] * node->leaf.mass[i];
            node->low_res_y    += node->leaf.y[i] * node->leaf.mass[i];
            node->low_res_mass += node->leaf.mass[i];
        }
        if (node->low_res_mass > 0.001f) {
            node->low_res_x    /= node->low_res_mass;
            node->low_res_y    /= node->low_res_mass;
        }
        node->timestamp_low_res = node->leaf.timestamp_high_res;
    } else {
        if (   node->tree.lower_left ->timestamp_low_res != node->tree.lower_right->timestamp_low_res
            || node->tree.lower_right->timestamp_low_res != node->tree.upper_left ->timestamp_low_res
            || node->tree.upper_left ->timestamp_low_res != node->tree.upper_right->timestamp_low_res)
        {
            fprintf(stderr, "Error: while combining data of for tree node %d\n", node->id);
            fprintf(stderr, "Error: mismatching timestamps for low-resolution data (%d, %d, %d, %d)\n", node->tree.lower_left->timestamp_low_res, node->tree.lower_right->timestamp_low_res, node->tree.upper_left->timestamp_low_res, node->tree.upper_right->timestamp_low_res);
            exit(15);
        }
        
        node->low_res_x = node->tree.lower_left ->low_res_x * node->tree.lower_left ->low_res_mass
                        + node->tree.lower_right->low_res_x * node->tree.lower_right->low_res_mass
                        + node->tree.upper_left ->low_res_x * node->tree.upper_left ->low_res_mass
                        + node->tree.upper_right->low_res_x * node->tree.upper_right->low_res_mass;
        node->low_res_y = node->tree.lower_left ->low_res_y * node->tree.lower_left ->low_res_mass
                        + node->tree.lower_right->low_res_y * node->tree.lower_right->low_res_mass
                        + node->tree.upper_left ->low_res_y * node->tree.upper_left ->low_res_mass
                        + node->tree.upper_right->low_res_y * node->tree.upper_right->low_res_mass;
        node->low_res_mass = node->tree.lower_left->low_res_mass + node->tree.lower_right->low_res_mass
                           + node->tree.upper_left->low_res_mass + node->tree.upper_right->low_res_mass;
        if (node->low_res_mass > 0.001f) {
            node->low_res_x /= node->low_res_mass;
            node->low_res_y /= node->low_res_mass;
        }
        node->timestamp_low_res = node->tree.lower_left->timestamp_low_res;
    }
}

void node_leaf_ensure_capacity(Node* node, int capacity) {
    if (node->leaf.capacity < capacity) {
        node->leaf.capacity = 2 * node->leaf.capacity;
        if (node->leaf.capacity < capacity)
            node->leaf.capacity = capacity;
        node->leaf.x    = realloc(node->leaf.x,    sizeof(float) * node->leaf.capacity);
        node->leaf.y    = realloc(node->leaf.y,    sizeof(float) * node->leaf.capacity);
        node->leaf.vx   = realloc(node->leaf.vx,   sizeof(float) * node->leaf.capacity);
        node->leaf.vy   = realloc(node->leaf.vy,   sizeof(float) * node->leaf.capacity);
        node->leaf.mass = realloc(node->leaf.mass, sizeof(float) * node->leaf.capacity);
    }
}

void node_leaf_append_object(Node* node, float x, float y, float vx, float vy, float mass) {
    node_leaf_ensure_capacity(node, node->leaf.count + 1);
    
    node->leaf.x   [node->leaf.count] = x;
    node->leaf.y   [node->leaf.count] = y;
    node->leaf.vx  [node->leaf.count] = vx;
    node->leaf.vy  [node->leaf.count] = vy;
    node->leaf.mass[node->leaf.count] = mass;
    node->leaf.count += 1;
}

Node* node_create_helper(int num_leaves, int points, Node* parent, float x0, float y0, float x1, float y1, int* id_leaf, int* id_tree) {
    Node* node = calloc(1, sizeof(Node));
    node->parent = parent;
    node->x0 = x0; node->y0 = y0;
    node->x1 = x1; node->y1 = y1;
    
    if ((num_leaves - 1) % 3 != 0) {
        fprintf(stderr, "Error: You are trying to run the program using %d threads, which is not"
            " one plus a multiple of three.\n", num_leaves);
        exit(12);
    } else if (num_leaves == 1) {
        node->is_leaf = true;
        node->id = (*id_leaf)++;
        int actual_points = points * (1.f + 0.2f * random_float_normal());
        if (actual_points < points / 4) actual_points = points / 4;

        for (int i = 0; i < actual_points; ++i) {
            node_leaf_append_object(node,
                random_float() * (x1 - x0) + x0, random_float() * (y1 - y0) + y0,
                random_float_normal()*0.01f,      random_float_normal()*0.01f,
                fabsf(random_float_normal()) + 1.f
            );
            float vx = node->leaf.x[i] - 500.f;
            float vy = node->leaf.y[i] - 500.f;
            node->leaf.vx[i] += -vy / 2000.f;
            node->leaf.vy[i] +=  vx / 2000.f;
        }
    } else {
        node->is_leaf = false;
        node->id = (*id_tree)++;

        int f[4] = {1, 1, 1, 1};
        num_leaves -= 4;
        while (num_leaves > 0) {
            f[random_u64() % 4] += 3;
            num_leaves -= 3;
        }
        node->tree.lower_left  = node_create_helper(f[0], points, node, x0, y0, 0.5f*(x0 + x1), 0.5f*(y0 + y1), id_leaf, id_tree);
        node->tree.lower_right = node_create_helper(f[1], points, node, 0.5f*(x0 + x1), y0, x1, 0.5f*(y0 + y1), id_leaf, id_tree);
        node->tree.upper_left  = node_create_helper(f[2], points, node, x0, 0.5f*(y0 + y1), 0.5f*(x0 + x1), y1, id_leaf, id_tree);
        node->tree.upper_right = node_create_helper(f[3], points, node, 0.5f*(x0 + x1), 0.5f*(y0 + y1), x1, y1, id_leaf, id_tree);
    }

    node_update_low_res_single(node);

    return node;
}
Node* node_create(int num_leaves, int points) {
    rand_state = 0xd1620b2a7a243d4bull;
    int id_leaf = 0;
    int id_tree = num_leaves;
    return node_create_helper(num_leaves, points, NULL, 0.f, 0.f, 1000.f, 1000.f, &id_leaf, &id_tree);
}

void node_draw_svg_helper(Node* node, FILE* file) {
    fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"#f0f0ff\" stroke=\"none\" />\n",
        node->low_res_x, node->low_res_y, sqrtf(node->low_res_mass));
    
    if (node->is_leaf) {
        int r = random_u64_seed(node->id) % 175 + 10;
        int g = random_u64_seed(node->id) % 177 + 10;
        int b = random_u64_seed(node->id) % 179 + 10;
        for (int i = 0; i < node->leaf.count; ++i) {
            fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"#%02x%02x%02x\" stroke=\"none\" />\n",
                    node->leaf.x[i], node->leaf.y[i], sqrtf(node->leaf.mass[i]), r, g, b);
        }
        fprintf(file, "<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" stroke-width=\"2\" stroke=\"#f0f0f0\" fill=\"none\" />\n",
            node->x0, node->y0, node->x1 - node->x0, node->y1 - node->y0);
    } else {
        node_draw_svg_helper(node->tree.lower_left,  file);
        node_draw_svg_helper(node->tree.lower_right, file);
        node_draw_svg_helper(node->tree.upper_left,  file);
        node_draw_svg_helper(node->tree.upper_right, file);
    }
}

void node_draw_svg(Node* node, char* file_name) {
    FILE* file = fopen(file_name, "w");
    if (file == NULL) {
        perror("");
        fprintf(stderr, "Error: Could not open file %s for writing\n", file_name);
        exit(16);
    }

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"-5 -5 1010 1010\">\n");
    fprintf(file, "<rect x=\"-5\" y=\"-5\" width=\"1010\" height=\"1010\" fill=\"white\" stroke=\"none\" />\n");
    node_draw_svg_helper(node, file);
    fprintf(file, "</svg>\n");

    fclose(file);
}

void node_draw_circles(Node* node, float** circles, int* circles_count, int* circles_capacity) {
    if (node->is_leaf) {
        if (*circles_count + node->leaf.count >= *circles_capacity) {
            *circles_capacity = 2 * *circles_capacity;
            if (*circles_capacity < *circles_count + node->leaf.count)
                *circles_capacity = *circles_count + node->leaf.count + 16;
            *circles = realloc(*circles, *circles_capacity * sizeof(float) * 3);
        }
        for (int i = 0; i < node->leaf.count; ++i) {
            (*circles)[3 * *circles_count    ] = node->leaf.x[i];
            (*circles)[3 * *circles_count + 1] = node->leaf.y[i];
            (*circles)[3 * *circles_count + 2] = sqrtf(node->leaf.mass[i]);
            ++*circles_count;
        }
    } else {
        node_draw_circles(node->tree.lower_left,  circles, circles_count, circles_capacity);
        node_draw_circles(node->tree.lower_right, circles, circles_count, circles_capacity);
        node_draw_circles(node->tree.upper_left,  circles, circles_count, circles_capacity);
        node_draw_circles(node->tree.upper_right, circles, circles_count, circles_capacity);
    }
}

void node_check_almost_equal(Node* a, Node* b, float max_err) {
    float x = a->low_res_x - b->low_res_x;
    float y = a->low_res_y - b->low_res_y;
    float mass = a->low_res_mass - b->low_res_mass;
    float err = fmaxf(sqrtf(x*x+y*y), abs(mass));
    if (err > max_err) {
        fprintf(stderr, "Error while doing unit test. You should inpect the generated SVG files to debug the differences.\n");
        exit(1);
    }
}

float distance_point_point(float x0, float y0, float px, float py) {
    return sqrtf((x0-px)*(x0-px)+(y0-py)*(y0-py));
}
float distance_rectangle_point(float x0, float y0, float x1, float y1, float px, float py) {
    if (x0 <= px && px <= x1) {
        if (y0 <= py && py <= y1) {
            return 0.f;
        } else if (py < y0) {
            return y0 - py;
        } else {
            return py - y1;
        }
    } else if (px < x0) {
        if (y0 <= py && py <= y1) {
            return x0 - px;
        } else if (py < y0) {
            return distance_point_point(x0, y0, px, py);
        } else {
            return distance_point_point(x0, y1, px, py);
        }
    } else {
        if (y0 < py && py < y1) {
            return px - x1;
        } else if (py < y0) {
            return distance_point_point(x1, y0, px, py);
        } else {
            return distance_point_point(x1, y1, px, py);
        }
    }
}
float distance_rectangle_rectangle(float x0, float y0, float x1, float y1, float px0, float py0, float px1, float py1) {
    // Note that the rectangles are either non-overlapping, or one is a subset of the other
    float f1 = distance_rectangle_point(x0, y0, x1, y1, px0, py0);
    float f2 = distance_rectangle_point(x0, y0, x1, y1, px1, py0);
    float f3 = distance_rectangle_point(x0, y0, x1, y1, px0, py1);
    float f4 = distance_rectangle_point(x0, y0, x1, y1, px1, py1);
    float f5 = distance_rectangle_point(px0, py0, px1, py1, x0, y0);
    float f6 = distance_rectangle_point(px0, py0, px1, py1, x1, y0);
    float f7 = distance_rectangle_point(px0, py0, px1, py1, x0, y1);
    float f8 = distance_rectangle_point(px0, py0, px1, py1, x1, y1);
    return fminf(fminf(fminf(f1, f2), fminf(f3, f4)), fminf(fminf(f5, f6), fminf(f7, f8)));
}
float distance_node_node(Node* node0, Node* node1) {
    return distance_rectangle_rectangle(node0->x0, node0->y0, node0->x1, node0->y1,
        node1->x0, node1->y0, node1->x1, node1->y1);
}

float global_timestep = 0.01f;

void compute_acceleration_point(Node* node, float px, float py, float pmass) {
    for (int j = 0; j < node->leaf.count; ++j) {
        float dx = px - node->leaf.x[j];
        float dy = py - node->leaf.y[j];
        // Ok, if there are any physicists here, please do not look too closely at this line. It is
        // numerically stable, anyways.
        float d = sqrtf(dx*dx + dy*dy) + 1.f;
        float acc = pmass / (d*d) * global_timestep;
        node->leaf.vx[j] += acc * dx / d;
        node->leaf.vy[j] += acc * dy / d;
    }
}
void compute_acceleration(Node* node, Node* source) {
    if (!node->is_leaf) {
        compute_acceleration(node->tree.lower_left,  source);
        compute_acceleration(node->tree.lower_right, source);
        compute_acceleration(node->tree.upper_left,  source);
        compute_acceleration(node->tree.upper_right, source);
        return;
    }
    
    if (distance_node_node(node, source) <= DISTANCE_CUTOFF) {
        if (source->is_leaf) {
            if (source->leaf.timestamp_high_res != node->leaf.timestamp_high_res) {
                fprintf(stderr, "Error: mismatching high-res timestamps for nodes %d and %d (timestamps %d, %d)\n", node->id, source->id, source->leaf.timestamp_high_res, node->leaf.timestamp_high_res);
                fprintf(stderr, "Error: did you forget to transfer high-res data to neighbours between timesteps?\n");
                exit(17);
            }
            
            for (int i = 0; i < source->leaf.count; ++i) {
                compute_acceleration_point(node, source->leaf.x[i], source->leaf.y[i], source->leaf.mass[i]);
            }
        } else {
            compute_acceleration(node, source->tree.lower_left);
            compute_acceleration(node, source->tree.lower_right);
            compute_acceleration(node, source->tree.upper_left);
            compute_acceleration(node, source->tree.upper_right);
        }
    } else {
        if (source->timestamp_low_res != node->leaf.timestamp_high_res) {
            fprintf(stderr, "Error: mismatching timestamps for nodes %d and %d\n", node->id, source->id);
            fprintf(stderr, "Error: did you forget to transfer low-res data to neighbours between timesteps?\n");
            exit(18);
        }
            
        compute_acceleration_point(node, source->low_res_x, source->low_res_y, source->low_res_mass);
    }
}

void node_find_nearby(Node** nearby, int* nearby_count, Node* root, Node* other) {
    if (other->is_leaf) {
        if (root->id != other->id && distance_node_node(root, other) <= DISTANCE_CUTOFF) {
            nearby[*nearby_count] = other;
            ++*nearby_count;
        }
    } else {
        node_find_nearby(nearby, nearby_count, root, other->tree.lower_left);
        node_find_nearby(nearby, nearby_count, root, other->tree.lower_right);
        node_find_nearby(nearby, nearby_count, root, other->tree.upper_left);
        node_find_nearby(nearby, nearby_count, root, other->tree.upper_right);
    }
}

bool node_contains_point(Node* node, float px, float py) {
    return node->x0 <= px && px <= node->x1 && node->y0 <= py && py <= node->y1;
}

void node_update_low_res(Node* node) {
    if (!node->is_leaf) {
        // Recursively update the children.
        node_update_low_res(node->tree.lower_left);
        node_update_low_res(node->tree.lower_right);
        node_update_low_res(node->tree.upper_left);
        node_update_low_res(node->tree.upper_right);
    }

    node_update_low_res_single(node);
}

int move_objects_helper(Node* node, float* leaving) {
    int left = 0;
    if (node->is_leaf) {
        // Update the positions
        for (int i = 0; i < node->leaf.count; ++i) {
            node->leaf.x[i] += node->leaf.vx[i];
            node->leaf.y[i] += node->leaf.vy[i];
        }

        // Check whether any object have left the rectangle
        for (int i = 0; i < node->leaf.count; ++i) {
            if (!node_contains_point(node, node->leaf.x[i], node->leaf.y[i])) {
                // Write the object into the leaving array
                leaving[5*left]   = node->leaf.x[i];
                leaving[5*left+1] = node->leaf.y[i];
                leaving[5*left+2] = node->leaf.vx[i];
                leaving[5*left+3] = node->leaf.vy[i];
                leaving[5*left+4] = node->leaf.mass[i];
                ++left;

                // Remove it from the node
                node->leaf.x[i]    = node->leaf.x[node->leaf.count-1];
                node->leaf.y[i]    = node->leaf.y[node->leaf.count-1];
                node->leaf.vx[i]   = node->leaf.vx[node->leaf.count-1];
                node->leaf.vy[i]   = node->leaf.vy[node->leaf.count-1];
                node->leaf.mass[i] = node->leaf.mass[node->leaf.count-1];
                --node->leaf.count;
                --i;
            }
        }

        ++node->leaf.timestamp_high_res;
    } else {
        // Recursively update the children.
        left += move_objects_helper(node->tree.lower_left,  leaving + 5*left);
        left += move_objects_helper(node->tree.lower_right, leaving + 5*left);
        left += move_objects_helper(node->tree.upper_left,  leaving + 5*left);
        left += move_objects_helper(node->tree.upper_right, leaving + 5*left);
    }

    if (left+1 >= MAX_LEAVING) {
        // This should never happen...
        fprintf(stderr, "Error: No enough buffer for leaving nodes (have %d, need at least %d)\n", MAX_LEAVING, left);
        exit(13);
    }

    return left;
}

float* global_circles;
int global_circles_count, global_circles_capacity;
bool global_show_gui;

int move_objects(Node* node, float* leaving) {
    if (global_show_gui) {
        global_circles_count = 0;
        node_draw_circles(node, &global_circles, &global_circles_count, &global_circles_capacity);
        gui_draw(global_circles, global_circles_count);
    }
    
    return move_objects_helper(node, leaving);
}

Node* node_find(Node* root, int id) {
    if (root->id == id) {
        return root;
    } else if (root->is_leaf) {
        return NULL;
    } else {
        Node* node;
        if ((node = node_find(root->tree.lower_left,  id))) return node;
        if ((node = node_find(root->tree.lower_right, id))) return node;
        if ((node = node_find(root->tree.upper_left,  id))) return node;
        if ((node = node_find(root->tree.upper_right, id))) return node;
        return NULL;
    }
}
