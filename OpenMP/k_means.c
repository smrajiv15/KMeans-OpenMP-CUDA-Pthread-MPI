/*
 * Skeleton function for CS4823 Introduction of Parallel Programming, 
 * Assignment 2: K-Means Algorithm (Sequential)
 *
 * To students: You should finish the implementation of k_means function
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <omp.h>

#include "k_means.h"

double distance(struct point p1, struct point p2) {
  return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

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
	    int c[MAX_POINTS])
{
	int i, j, l; /* To Students: add your local variables */
  double min_dist;
	double dist;
	double sumx;
	double sumy;
	long clust_size = 0;
					
	/* randomly initialized the centers */
	#pragma omp parallel
	{
		#pragma omp for
		for(j = 0; j < k; j++) {
			u[j] = random_center(p); /* DO NOT change this random generator! */
		}
	}
	
	for(i = 0; i < iters; i++) {

		#pragma omp parallel private(min_dist, dist, l)
		{
			#pragma omp for
			for(j = 0; j < m; j++) {
				min_dist = DBL_MAX;

				for(l = 0; l < k; l++) {
					dist = distance(p[j], u[l]);
					if(dist < min_dist) {
						printf("Dist : %lf-- (X:%lf Y:%lf) C:(%lf,%lf)\n", dist, p[j].x, p[j].y, u[l].x, u[l].y);
						min_dist = dist;
						c[j] = l;
						printf("Cluster ID: %d\n", c[j]);
					}
				}
			}
		}

		#pragma omp parallel private(sumx, sumy, clust_size, l)
		{
			#pragma omp for
			for(j = 0; j < k; j++) {
				sumx = 0;
				sumy = 0;
				clust_size = 0;

				#pragma omp parallel
				{
					#pragma omp for reduction(+:clust_size, sumx, sumy)
					for(l = 0; l < m; l++) {
						if(c[l] == j) {
							clust_size++;
							sumx += p[l].x;
							sumy += p[l].y;
						}
					}
				}

				if(clust_size > 0) {
					printf("Cluster size : %ld cluster: %d\n", clust_size, j);
					printf("X total: %lf Y total: %lf\n", sumx, sumy);
					u[j].x = sumx / (double)clust_size;
					u[j].y = sumy / (double)clust_size;
				} else {
					printf("Random\n");
					u[j] = random_center(p);
				}
			}
		}

	}																																																																
															
	return;
}
