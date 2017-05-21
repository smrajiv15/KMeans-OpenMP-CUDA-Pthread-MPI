/*
 * Random (fake) function for Parallel Computing Class, 
 * Assignment: K-Means Algorithm (CUDA)
 *
 * To students: You should not modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdlib.h>

#include "k_means.h"

int counter = 0;

struct point random_center(struct point p[MAX_POINTS])
{
	int idx = 0;
	idx = __sync_fetch_and_add(&counter, 1)%MAX_POINTS;
	return p[idx];
}
