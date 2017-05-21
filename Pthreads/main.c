/*
 * Main function for Parallel Computing course, 
 * Assignment 4: K-Means Algorithm (Pthreads)
 *
 * To students: You SHOULD modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <err.h>
#include <pthread.h>

#include "k_means.h"

int verbose = 0;
int iters = 0;
struct global_cluster cal_centre;
pthread_mutex_t glob_clust_lock;
pthread_barrier_t thrd_bar;

/* command line arguments */
struct option long_options[] = {
	{"data-file",     required_argument, 0, 'f'}, /* input data file */
	{"clusters",      required_argument, 0, 'k'}, /* cluster count */
	{"iterations",    required_argument, 0, 'i'}, /* iteration count */
	{"threads",       required_argument, 0, 't'}, /* number of threads */
	{"help",          no_argument,       0, 'h'}, /* print help */
	{0, 0, 0, 0}
};

void print_usage()
{
	printf("Usage: ./this_command [-h] [-f DATA_FILE] [-k K] [-i ITERS] "
	       "[-t THREAD_COUNT]\n");
	printf("\n");
	printf("-h, --help \t\t\t show this help message and exit\n");
	printf("-f, --data-file \t\t specify the path and name of input "
	       "file\n");
	printf("-k, --clusters \t\t\t specify the number of clusters to find;"
	       "default 2 clusters\n");
	printf("-t, --threads \t\t\t specify the number of threads to use;"
	       "default 1 thread\n");
	printf("-i, --iterations \t\t specify the number of iterations of "
	       "clustering to run; default 10 iterations\n");

	return;
}

/* Global variables */
struct point p[MAX_POINTS]; /* array that holds data points */
struct point u[MAX_CENTERS]; /* array that holds the centers */
int c[MAX_POINTS]; /* cluster id for each point */

void assign_thread_params(struct thread_params *p, int thr_cnt, int num_clust, int num_pts) {
	unsigned pts_per_thread = num_pts / thr_cnt;
	int max_pts = num_pts;
	int i;
	
	for(i = thr_cnt - 1; i >= 0; i--) {
		p[i].thread_count = thr_cnt;
		p[i].num_clusters = num_clust;
		if((max_pts - pts_per_thread) >= pts_per_thread && i != 0) {
			p[i].end_point = max_pts;
			max_pts -= pts_per_thread;
			p[i].start_point = max_pts;	
		}else {
			p[i].start_point = 0;
			p[i].end_point   = max_pts;
		}
	}
}


int main(int argc, char *argv[])
{
	int o;
	
	char *data_file = "";
	int k = 2; /* number of clusters */
	int m = 0; /* number of data points */
	int thread_cnt = 1;
	int i, j;
	iters = 10; /* number of iterations of clustering */
	
	pthread_t *thread_ids = NULL;
	struct thread_params *params = NULL;

	/* parse command arguments */
	while(1){
		o = getopt_long(argc, argv, "hf:k:i:t:", long_options, NULL);
		if(o == -1) /* no more options */
			break;
		switch(o){
		case 'h':
			print_usage();
			exit(0);
		case 'f':
			data_file = strdup(optarg);
			break;
		case 'k':
			k = atoi(optarg);
			if(k > MAX_CENTERS){
				printf("Too many centers (must < %d)\n",
				       MAX_CENTERS);
				exit(-1);
			}
				
			break;
		case 'i':
			iters = atoi(optarg);
			break;
		case 't':
		        thread_cnt = atoi(optarg);
			break;
		default:
			printf("Unknown argument: %d\n", o);
			print_usage();
			exit(0);
		}
	}

	thread_ids = (pthread_t*)malloc(sizeof(pthread_t) * thread_cnt);
	if(!thread_ids) {
		printf("Error: Unable to allocate memory for thread ID");
		exit(0);	
	} 

  params = (struct thread_params*)malloc(sizeof(struct thread_params) * thread_cnt);
	if(!params) {
		printf("Error: Unable to allocate memory for thread parameters\n");
		free(thread_ids);
		exit(0);
	} 

	cal_centre.p = (struct point *)malloc(sizeof(struct point) * k);
	if(!cal_centre.p) {
		printf("Error: Unable to allocate memory\n");
		free(thread_ids);
		free(params);
		exit(0);
	}

	cal_centre.clust_size = (long*)malloc(sizeof(long)*k);
	if(!cal_centre.clust_size) {
		printf("Error: Unable to allocate memory\n");
		free(thread_ids);
		free(params);
		free(cal_centre.p);
		exit(0);
	}

	pthread_mutex_init(&glob_clust_lock, NULL);
	pthread_barrier_init(&thrd_bar, NULL, thread_cnt);

	
	printf("Reading from data file: %s.\n", data_file);
	printf("Finding %d clusters\n", k);

	/* read data points from file */
	read_points_from_file(data_file, p, &m);
	
	assign_thread_params(params, thread_cnt, k, m);

	/* randomly initialized the centers */
	for(j = 0; j < k; j++)
		u[j] = random_center(p);


	/* do K-Means */
	for(i = 0; i < thread_cnt; i++){
		pthread_create(&thread_ids[i], NULL, k_means, &params[i]);	
	}


	for(i = 0; i < thread_cnt; i++) {
		pthread_join(thread_ids[i], NULL);
	}


	/* output centers and cluster assignment */
	printf("centers found:\n");
	for(j = 0; j < k; j++)
		printf("%.2lf, %.2lf\n", u[j].x, u[j].y);
		  
	free(cal_centre.p);
	free(cal_centre.clust_size);
	free(params);
	free(thread_ids);
	return 0;
}

