/*
 * Main function for Parallel Computing course, 
 * Assignment: K-Means Algorithm (MPI)
 *
 * To students: You may modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <err.h>
#include <mpi.h>

#include "k_means.h"

#define SEND_TAG 1000
#define RECV_TAG 2000

int verbose = 0;

/* command line arguments */
struct option long_options[] = {
	{"data-file",     required_argument, 0, 'f'}, /* input data file */
	{"clusters",      required_argument, 0, 'k'}, /* cluster count */
	{"iterations",    required_argument, 0, 'i'}, /* iteration count */
	{"help",          no_argument,       0, 'h'}, /* print help */
	{0, 0, 0, 0}
};

void print_usage()
{
	printf("Usage: ./this_command [-h] [-f DATA_FILE] [-k K] [-i ITERS]\n");
	printf("\n");
	printf("-h, --help \t\t\t show this help message and exit\n");
	printf("-f, --data-file \t\t specify the path and name of input "
	       "file\n");
	printf("-k, --clusters \t\t\t specify the number of clusters to find;"
	       "default 2 clusters\n");
	printf("-i, --iterations \t\t specify the number of iterations of "
	       "clustering to run; default 10 iterations\n");

	return;
}

int main(int argc, char *argv[])
{
	int o;
	double *sumx, *sumy = 0;
	long *clust_size = 0;

	char *data_file = "";
	int k = 2; /* number of clusters */
	int m = 0; /* number of data points */
	int iters = 10; /* number of iterations of clustering */
	
	int i, j;
	struct point p[MAX_POINTS]; /* array that holds data points */
	struct point u[MAX_CENTERS]; /* array that holds the centers */
	int c[MAX_POINTS]; /* cluster id for each point */

	int proc_id;
	int c_max = 0;
	int *c_dup = NULL;
	int num_procs = 0;
	struct parition_info *info = NULL;
	int pts_per_procs, start_point, end_point, tot_pts = 0;
	MPI_Status status;
	
	//registering new datatype for point
	MPI_Datatype point_obj;
	int count = 1;
	int blk_len[1] = {2}; //only for double
	MPI_Aint offsets[1] = {0};
	MPI_Datatype types[1] = {MPI_DOUBLE};

	MPI_Init(&argc,&argv);
	/* get process information */
	MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	//register for point structure
	MPI_Type_struct(count, blk_len, offsets, types, &point_obj);
	MPI_Type_commit(&point_obj); 

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
		default:
			printf("Unknown argument: %d\n", o);
			print_usage();
			exit(0);
		}
	}

	if(proc_id == 0){
		printf("Reading from data file: %s.\n", data_file);
		printf("Finding %d clusters\n", k);

		info = (struct parition_info*)malloc(sizeof(struct parition_info) * num_procs);
		if(!info) {
			printf("Error: Unable to allocation Memory\n");
			exit(0);
		}

		sumx = (double *)malloc(sizeof(double) * k);
		if(!sumx) {
			printf("Error: Unable to allocate memory\n");
			exit(0);
		}

		memset(sumx, 0, sizeof(double) * k);

		sumy = (double *)malloc(sizeof(double) * k);
		if(!sumx) {
			printf("Error: Unable to allocate memory\n");
			exit(0);
		}

		memset(sumy, 0, sizeof(double) * k);

		clust_size = (long *)malloc(sizeof(long) * k);
		
		if(!clust_size) {
			printf("Error: Unable to allocate memory\n");
			exit(0);
		}

		memset(clust_size, 0, sizeof(long) * k);
	}

	/* read data points from file */
	read_points_from_file(data_file, p, &m);

	if(proc_id == 0) {
		//generating random centers
		for(j = 0; j < k; j++) {
			u[j] = random_center(p);
		}

		//splitting the points to compute by each process
		tot_pts = m;
		pts_per_procs = m / num_procs;
	
		for(j = 0; j < num_procs; j++) {
			if (j + 1 !=  num_procs) {
				start_point = tot_pts;
				tot_pts -= pts_per_procs;
				end_point = tot_pts;
			} else {
				start_point = tot_pts;
				end_point = 0;
				tot_pts = 0;
			}

			info[j].sp = start_point;
			info[j].ep = end_point;

			o = start_point - end_point;

			if(c_max < o) {
				c_max = o;
			}
			
			if( j != 0) { //send data point range to all the slaves
				MPI_Send(&start_point, 1, MPI_INT, j, SEND_TAG, MPI_COMM_WORLD); 
				MPI_Send(&end_point, 1, MPI_INT, j, SEND_TAG, MPI_COMM_WORLD);
			}	
		}
		
		for(j = 1; j < num_procs; j++) {		
			MPI_Send(&c_max, 1, MPI_INT, j, SEND_TAG, MPI_COMM_WORLD); 
		}

		c_dup = (int *) malloc(sizeof(int) * c_max);
		if(!c_dup) {
			printf("Error: Unable to allocate memory\n");
			exit(0);
		}
	
		while(iters > 0) {
			//template for passing the recomputed centers to slaves
			for(j = 0; j < num_procs; j++) {
				if(j != 0)
					MPI_Send(u, MAX_CENTERS, point_obj, j, SEND_TAG, MPI_COMM_WORLD);
			} 

			k_means(p, m, k, iters, u, c, info[proc_id].sp, info[proc_id].ep, proc_id);
	
			//receive cluster ID's from slaves			
			for(j = 1; j < num_procs; j++) {
				MPI_Recv(c_dup, c_max, MPI_INT, MPI_ANY_SOURCE, RECV_TAG, MPI_COMM_WORLD, &status);
				i = 0;
				for(o = info[status.MPI_SOURCE].ep; o < info[status.MPI_SOURCE].sp; o++) {
					c[o] = c_dup[i++];	
				}			
			}

			//compute new centers
			for(i = 0; i < m; i++) {
					clust_size[c[i]] += 1;
					sumx[c[i]] += p[i].x;
					sumy[c[i]] += p[i].y;
			}
			
			for(i = 0; i < k; i++) {	
				if(clust_size[i] > 0) {			
					u[i].x = sumx[i] / (double) clust_size[i];
					u[i].y = sumy[i] / (double) clust_size[i];
				} else {
					u[i] = random_center(p);
				}
			}

			memset(clust_size, 0, sizeof(long) * k);
			memset(sumx, 0, sizeof(double) * k);
			memset(sumy, 0, sizeof(double) * k);

			iters--;
		}
		free(clust_size);
		free(sumx);
		free(sumy);
		free(info);
		free(c_dup);
	} else {
		
		MPI_Recv(&start_point, 1, MPI_INT, 0, SEND_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&end_point, 1, MPI_INT, 0, SEND_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&c_max, 1, MPI_INT, 0, SEND_TAG, MPI_COMM_WORLD, &status);

		c_dup = (int *) malloc(sizeof(int) * c_max);
		if(!c_dup) {
			printf("Error: Unable to allocate memory\n");
			exit(0);
		}

		while(iters > 0) {
			//template for receiving the centre from master
			MPI_Recv(u, MAX_CENTERS, point_obj, 0, SEND_TAG, MPI_COMM_WORLD, &status);
			k_means(p, m, k, iters, u, c, start_point, end_point, proc_id);
			memcpy(c_dup, &c[end_point], (start_point - end_point) * sizeof(int));
			MPI_Send(c_dup, c_max, MPI_INT, 0, RECV_TAG, MPI_COMM_WORLD);  
			iters--;
		}
		free(c_dup);
	}

	/* output centers and cluster assignment */
	if(proc_id == 0){
		printf("centers found:\n");
		for(j = 0; j < k; j++)
			printf("%.2lf, %.2lf\n", u[j].x, u[j].y);
	}

	MPI_Type_free(&point_obj);
	MPI_Finalize();
	
	return 0;
}

