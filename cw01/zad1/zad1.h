#include <stdio.h>
#include <stdlib.h>

/* Returns pointer to an array of void* with n elements */
void** create_array(int n);

/* Function creates a file with
 * number of lines, words and characters respectively */
void file_stats(char* filename);

/* Function returns index of created block in an array */
int save_results_to_array(void** array, int array_len, char* results_filename);

/* Function frees i-th index in the main array */
void free_index(void** array, int index);
