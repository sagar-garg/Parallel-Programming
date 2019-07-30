#ifndef _HELPER_H_
#define _HELPER_H_

#include<time.h>

int count_occurences(char* string, const char* sub_string);
double time_diff(const struct timespec *first, const struct timespec *second, struct timespec *diff);
int create_lines(char* text );
void set_line_pointers(char* text, char** lines, int num_lines);
int get_max_line_length(char** line_pointers,int num_lines);

#endif
