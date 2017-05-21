/*
 * Skeleton function for Parallel Computing course.
 * Assignment: K-Means Algorithm (Pthread)
 *
 * To students: You should finish the implementation of k_means function
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include "k_means.h"

#define DIST(p1, p2) (\
				(p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y)\
			)

/* Global variables that hold the centers, data points and assignments */
extern struct point p[MAX_POINTS]; /* array that holds data points */
extern struct point u[MAX_CENTERS]; /* array that holds the centers */
extern int c[MAX_POINTS]; /* cluster id for each point */

extern int iters;
extern struct global_cluster cal_centre;
extern pthread_mutex_t glob_clust_lock;
extern pthread_barrier_t thrd_bar;


/*
 * k_means: thread function that implements k_means clustering algorithm
 *
 * Input parameters:
 *     TO STUDENTS: Although the arrays of centers, data points and "points'
 *                  cluster ids" are made global variables, you still
 *                  need to pass in additional variables, such as number of
 *                  clusters, number of data points, and other variables to
 *                  help your decomposition.
 * Output parameters:
 *     TO STUDENTS: You should work on the global variables of centers and 
 *                  "points; cluster ids." They are visible in main thread as
 *                  well.
 * Return values:   
 *     NONE; main thread will not check return values.
 */
void *k_means(void *parameter)
{
	int i = 0, j = 0;
	double min_dist;
	double dist;
	struct thread_params *params = (struct thread_params *)parameter;
	double x_sum[params->num_clusters];
	double y_sum[params->num_clusters];
	unsigned long clust_size[params->num_clusters];

	
	for(i = 0; i < params->num_clusters; i++) {
		x_sum[i]      = 0;
		y_sum[i]      = 0;
		clust_size[i] = 0;
	}

	while(iters != 0) {
		for(i = params->start_point; i < params->end_point; i++) {
			min_dist = DBL_MAX;

			for(j = 0; j < params->num_clusters; j++) {
				dist = DIST(p[i], u[j]);
				if(dist < min_dist) {
					min_dist = dist;
					c[i] = j;	
				}
			}
			
			x_sum[c[i]] += p[i].x; 
			y_sum[c[i]] += p[i].y;
			clust_size[c[i]] += 1;		
		}
	
		pthread_mutex_lock(&glob_clust_lock);
		for(j = 0; j < params->num_clusters; j++) {	
			cal_centre.p[j].x += x_sum[j];
			cal_centre.p[j].y += y_sum[j];
			cal_centre.clust_size[j] += clust_size[j];
			x_sum[j] = 0;
			y_sum[j] = 0;
			clust_size[j] = 0;
		}

		cal_centre.thd_cmpt++;
		
		if(cal_centre.thd_cmpt == params->thread_count) {
			for(j = 0; j < params->num_clusters; j++) {
				if(cal_centre.clust_size[j] > 0) {
					u[j].x = cal_centre.p[j].x / (double)cal_centre.clust_size[j];
					u[j].y = cal_centre.p[j].y / (double)cal_centre.clust_size[j];
				} else {
					u[j] = random_center(p);
				}
				cal_centre.p[j].x = 0;
				cal_centre.p[j].y = 0;
				cal_centre.clust_size[j] = 0;
			}
			iters--;
			cal_centre.thd_cmpt = 0;
		}
		pthread_mutex_unlock(&glob_clust_lock);				
		pthread_barrier_wait(&thrd_bar);
	}
	return NULL;
}
