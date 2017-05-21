/*
 * Common definitions for CS6643 Parallel Processing, 
 * Assignment 3: K-Means Algorithm (OpenMP)
 *
 * To students: You should not modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */

#ifndef __CS4823_ASSIGNMENT_2__
#define __CS4823_ASSIGNMENT_2__

#define MAX_POINTS 32768
#define MAX_CENTERS 16

/*
 * data structure for holding 
 */
struct point{
	double x;
	double y;
};

/*
 * read data points from input file
 */
int read_points_from_file(char *data_file, struct point *pts, int *m);

/*
 * find K_means 
 */
void k_means(struct point p[], int m, int k, int iters, struct point u[],
	    int c[]);

/*
 * return a (faked) random point
 */
struct point random_center(struct point p[]);

#endif
