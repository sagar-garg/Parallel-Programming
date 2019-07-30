
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

#include <mpi.h>

#include "gui.h"
#include "nbody.h"

#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

#if !defined(BUILD_PAR) && !defined(BUILD_SEQ) && !defined(BUILD_UNIT)
#define BUILD_PAR
#endif

#define DEFAULT_LEAVES 31
#define DEFAULT_GUI false
#define DEFAULT_STEPS 1
#define DEFAULT_FILE_REF "output_unit_ref.svg"
#ifdef BUILD_PAR
#define DEFAULT_POINTS 100
#define DEFAULT_FILE "output_par.svg"
#define DEFAULT_TIMESTEP 0.01f
#elif defined(BUILD_SEQ)
#define DEFAULT_POINTS 100
#define DEFAULT_FILE "output_seq.svg"
#define DEFAULT_TIMESTEP 0.01f
#elif defined(BUILD_UNIT)
#define DEFAULT_POINTS 20
#define DEFAULT_FILE "output_unit_out.svg"
#define DEFAULT_TIMESTEP 1.f
#endif

void simulate_seq(int steps, int num_leaves, Node* root);
void simulate_par(int steps, int num_leaves, Node* root);

double time_diff(struct timespec* first, struct timespec* second) {
	struct timespec tmp;
	struct timespec *tmp_ptr;

	if (first->tv_sec > second->tv_sec || (first->tv_sec == second->tv_sec && first->tv_nsec > second->tv_nsec)) {
        tmp_ptr = first;
        first = second;
        second = tmp_ptr;
    }

	tmp.tv_sec = second->tv_sec - first->tv_sec;
	tmp.tv_nsec = second->tv_nsec - first->tv_nsec;

	if (tmp.tv_nsec < 0) {
        tmp.tv_sec -= 1;
        tmp.tv_nsec += 1000000000;
    }

	return tmp.tv_sec + tmp.tv_nsec / 1000000000.0;
}

int main(int argc, char **argv) {

    int num_points = DEFAULT_POINTS;
    int num_leaves = DEFAULT_LEAVES;
	char file_name[256] = DEFAULT_FILE;
	char file_name_ref[256] = DEFAULT_FILE_REF;
    bool show_gui = DEFAULT_GUI;
    int num_steps = DEFAULT_STEPS;
    float timestep = DEFAULT_TIMESTEP;

    int rank, num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
#ifdef BUILD_PAR
    num_leaves = num_procs;
#endif

    
	int c;
	while ((c = getopt(argc, argv, "p:l:f:t:gs:")) != -1) {
        switch (c) {
        case 'p':
            if (sscanf(optarg, "%d", &num_points) != 1) goto error;
			break;
        case 's':
            if (sscanf(optarg, "%d", &num_steps) != 1) goto error;
			break;
        case 't':
            if (sscanf(optarg, "%f", &timestep) != 1) goto error;
			break;
        case 'l':
            if (sscanf(optarg, "%d", &num_leaves) != 1) goto error;
            if (num_leaves != num_procs) {
                num_leaves = num_procs;
                fprintf(stderr, "Warning: -l option ignored, numer of leaves is the same as the number of processes\n");
            }
            
			break;
        case 'f':
			strncpy(file_name, optarg, sizeof(file_name)-1);
            file_name[sizeof(file_name)-1] = 0;
			break;
        case 'r':
			strncpy(file_name_ref, optarg, sizeof(file_name_ref)-1);
            file_name[sizeof(file_name_ref)-1] = 0;
			break;
        case 'g':
            show_gui = true;
            break;
        case '?':
        error:
            fprintf(stderr,
                "Usage:\n  %s [-p <num_points>] [-l <num_leaves>] [-s <num_steps>] [-f <file>] [-r <file_ref>] [-g]\n"
                "  -p <num_points> [default: " STRINGIFY(DEFAULT_POINTS) "]\n"
                "      Initialise with (approximately) this number of points per leaf node.\n"
                "  -l <num_leaves> [default: " STRINGIFY(DEFAULT_LEAVES) "]\n"
                "      Generate this many leaf nodes. Must be 1+3*n for some n. Only used for sequential execution!\n"
                "  -t <timestep> [default: " STRINGIFY(DEFAULT_TIMESTEP) "]\n"
                "      How much time a single step of the simulation simulates.\n"
                "  -s <num_steps> [default: " STRINGIFY(DEFAULT_STEPS) "]\n"
                "      How many steps of the simulation to perform.\n"
                "  -f <file> [default: " DEFAULT_FILE "]\n"
                "      Name of the output SVG file.\n"
                "  -r <file_ref> [default: " DEFAULT_FILE_REF "]\n"
                "      Name of the output SVG file for the reference code.\n"
                "  -g [default: off]\n"
                "      Whether to show the animation in a window as it runs.\n", argv[0]);
            MPI_Finalize();
            exit(138);
        }
    }

    global_timestep = timestep;
    
    if (show_gui) {
        num_steps = 1 << 30;
    }
    
    if (rank != 0) {
        show_gui = false;
#ifdef BUILD_SEQ
        fprintf(stderr, "WARNING: Trying to run sequential code with multiple processes\n");
#endif
    }
    global_show_gui = show_gui;
    

    if (show_gui) {
        gui_create_window(1, argv);
    }
    

#ifdef BUILD_PAR
	struct timespec begin, end;
    Node* root = node_create(num_leaves, num_points);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    simulate_par(num_steps, num_leaves, root);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (rank == 0) {
        printf("Time: %lf seconds\n", time_diff(&begin, &end));
        node_draw_svg(root, file_name);
    }

#elif defined(BUILD_SEQ)
	struct timespec begin, end;
    Node* root = node_create(num_leaves, num_points);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    simulate_seq(num_steps, num_leaves, root);
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Time: %lf seconds\n", time_diff(&begin, &end));
    node_draw_svg(root, file_name);

#elif defined(BUILD_UNIT)
    Node* root = node_create(num_leaves, num_points);
    simulate_par(num_steps, num_leaves, root);

    if (rank == 0) {
        node_draw_svg(root, file_name);

        Node* root_ref = node_create(num_leaves, num_points);
        simulate_seq(num_steps, num_leaves, root_ref);
        node_draw_svg(root_ref, file_name_ref);
        
        node_check_almost_equal(root, root_ref, 1e-8);
    }
#endif
   
    MPI_Finalize();
}
