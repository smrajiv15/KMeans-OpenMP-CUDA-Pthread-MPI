
/*
 * Skeleton function for Parallel Computing Class, 
 * Assignment: K-Means Algorithm (CUDA)
 *
 * To students: You should finish the implementation of k_means algorithm.
 *              You should add device functions/kernels to perform k_means on 
 *              GPU. The "k_means" function in this file is just an interface
 *              for passing in basic parameters needed.. You need to add GPU 
 *              kernels and launch them in the "k_means" function.
 *
 *              Note that the "k_means" function has two input parameters for
 *              block count and thread count per block. Please use these two
 *              parameters when launching your kernels.
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "k_means.h"

#define CHECK(call)\
{\
	const cudaError_t error = call; \
	if(error != cudaSuccess) { \
		printf("Error: %s: Line: %d\n", __FILE__, __LINE__);\
		printf("Code : %d - Reason : %s\n", error, cudaGetErrorString(error));\
		exit(1);\
	}\
}

#define DIST(p1, p2) ((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y))

struct thread_limits {
	int start_point;
	int end_point;
	int tot_pts;
	double sumx;
	double sumy;
	unsigned clust_size_pt;
};


__host__ void assign_thread_limits(struct thread_limits *tl, int tt, int num_pts) {
	float pts_per_thread = ceil(num_pts / (float)tt); 
	int max_pts = num_pts;
	int i;
	
	for(i = 0; i < tt; i++) {
		if((max_pts - pts_per_thread) >= pts_per_thread && max_pts > 0) {
			tl[i].tot_pts = pts_per_thread;
			tl[i].end_point = max_pts;	
			max_pts -= pts_per_thread;
			tl[i].start_point = max_pts;
		} else {
			tl[i].start_point = 0;
			tl[i].end_point = max_pts;
			tl[i].tot_pts = max_pts;
			max_pts = 0;
			break;
		}
	}
}


__global__ void cluster_identifier(struct point *p, struct point *u, int *c, \
																        struct thread_limits *tl, int m, int k) {
	int i, j;
	double dist;
	double min_dist;
	int t_id = blockIdx.x * blockDim.x + threadIdx.x;	

	if(tl[t_id].tot_pts > 0) {
		for(i = tl[t_id].start_point; i < tl[t_id].end_point; i++) {
			min_dist = DBL_MAX;
			for(j = 0; j < k; j++) {				
				dist = DIST(p[i], u[j]);

				if(dist < min_dist) {
					min_dist = dist;
					c[i] = j;
				}		
			}
		}
	} 
}


__global__ void compute_center(int ci, int m, struct point *p, \
																				int *c, struct thread_limits *tl) {
	int i;	
	int t_id = blockIdx.x * blockDim.x + threadIdx.x;	

	//computing per thread sum w.r.t the cluster	
	if(tl[t_id].tot_pts > 0) {
		for(i = tl[t_id].start_point; i < tl[t_id].end_point; i++) {		
			if(c[i] == ci) {
				tl[t_id].sumx += p[i].x;
				tl[t_id].sumy += p[i].y;
				tl[t_id].clust_size_pt += 1;
			}
		}
	}
}


/*
 * k_means: k_means clustering algorithm implementation.
 *
 * Input parameters:
 *     struct point p[]: array of data points
 *     int m           : number of data points in p[]
 *     int k           : number of clusters to find
 *     int iters       : number of clustering iterations to run
 *     int block_cnt   : number of blocks to use
 *     int threads_per_block: number of threads per block
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
	     int c[MAX_POINTS],
	     int block_cnt,
	     int threads_per_block)
{
	int i, j, l;
	//Device Memory
	struct point *d_p;
	struct point *d_u;
	int *d_c;
	int mc = m;
	struct thread_limits *d_tl;
	double sumx, sumy;
	long clust_size;
	int tot_threads = block_cnt * threads_per_block;
	struct thread_limits tl[tot_threads];

	memset(tl, 0, sizeof(struct thread_limits) * tot_threads);
	assign_thread_limits(tl, tot_threads, m); 
	
	/* randomly initialized the centers */
	for(j = 0; j < k; j++)
		u[j] = random_center(p);
	
	cudaMalloc(&d_p, sizeof(struct point) * MAX_POINTS);
	cudaMalloc(&d_u, sizeof(struct point) * MAX_CENTERS);
	cudaMalloc(&d_c, sizeof(int) * MAX_POINTS);
	cudaMalloc(&d_tl,sizeof(struct thread_limits) * tot_threads); 
	
	CHECK(cudaMemcpy(d_p, p, sizeof(struct point) * MAX_POINTS, cudaMemcpyHostToDevice));
	CHECK(cudaMemcpy(d_u, u, sizeof(struct point) * MAX_CENTERS, cudaMemcpyHostToDevice));
	CHECK(cudaMemcpy(d_c, c, sizeof(int) * MAX_POINTS, cudaMemcpyHostToDevice));
	CHECK(cudaMemcpy(d_tl, tl, sizeof(struct thread_limits) * tot_threads, cudaMemcpyHostToDevice));

	for(i = 0; i < iters; i++) {
		cluster_identifier<<<block_cnt, threads_per_block>>>(d_p, d_u, d_c, d_tl, m, k);	
	
		for(j = 0; j < k; j++) {
			sumx = 0;
			sumy = 0;
			clust_size = 0;

			compute_center<<<block_cnt, threads_per_block>>>(j, m, d_p, d_c, d_tl); 
			
			CHECK(cudaMemcpy(tl, d_tl, sizeof(struct thread_limits) * tot_threads, cudaMemcpyDeviceToHost));

			for(l = 0; l < tot_threads; l++) {
				if(tl[l].tot_pts > 0) {
					sumx += tl[l].sumx;
					sumy += tl[l].sumy;
					clust_size += tl[l].clust_size_pt;
					tl[l].sumx = 0;
					tl[l].sumy = 0;
					tl[l].clust_size_pt = 0;
				}
			}

			if(clust_size > 0) {
				u[j].x = sumx / (double)clust_size;
				u[j].y = sumy / (double) clust_size;
			} else {
				u[j] = random_center(p);
			}
			CHECK(cudaMemcpy(d_tl, tl, sizeof(struct thread_limits) * tot_threads, cudaMemcpyHostToDevice));
		}	
		CHECK(cudaMemcpy(d_u, u, sizeof(struct point) * MAX_CENTERS, cudaMemcpyHostToDevice));
	}
	
	cudaFree(d_p);
	cudaFree(d_u);
	cudaFree(d_c);
	cudaFree(d_tl);	

  return;
}
