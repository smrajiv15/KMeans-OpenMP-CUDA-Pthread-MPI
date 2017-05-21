/*
 * Skeleton function for Parallel Computing course, 
 * Assignment: K-Means Algorithm (MPI)
 *
 * To students: You should finish the implementation of k_means function
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <mpi.h>

#include "k_means.h"

#define DIST(p1, p2) (\
        (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y)\
      )


/*
 * k_means: k_means clustering algorithm implementation.
 *
 * Input parameters:
 *     struct point p[]: array of data points
 *     int m           : number of data points in p[]
 *     int k           : number of clusters to find
 *     int iters       : number of clustering iterations to run
 *
 * Output parameters:   
 *     struct point u[]: array of cluster centers
 *     int c[]         : cluster id for each data points
 */
void k_means(struct point p[MAX_POINTS], 
	    int m, 
	    int k,
	    int iters,
	    struct point u[MAX_CENTERS],
	    int c[MAX_POINTS], int sp, int ep, int rank)
{
	int i, j;
	double min_dist, dist;

	for(i = ep; i < sp; i++) {
		min_dist = DBL_MAX;
		
		for(j = 0; j < k; j++) {
			dist = DIST(p[i], u[j]);

			if(dist < min_dist) {
				min_dist = dist;
				c[i] = j;
			}
		}
	}
	return;
}
