/*
 * Main function for CS6643 Parallel Processing, 
 * Assignment 3: K-Means Algorithm (OpenMP)
 *
 * To students: You should not modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <err.h>

#include "k_means.h"

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
	
	char *data_file = "";
	int k = 2; /* number of clusters */
	int m = 0; /* number of data points */
	struct point p[MAX_POINTS]; /* array that holds data points */
	struct point u[MAX_CENTERS]; /* array that holds the centers */
	int c[MAX_POINTS]; /* cluster id for each point */
	int iters = 10; /* number of iterations of clustering */

	int j;
	
	/* parse command arguments */
	while(1){
		o = getopt_long(argc, argv, "hf:k:i:", long_options, NULL);
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

	printf("Reading from data file: %s.\n", data_file);
	printf("Finding %d clusters\n", k);

	/* read data points from file */
	read_points_from_file(data_file, p, &m);

	/* do k-means */
	k_means(p, m, k, iters, u, c);

	/* output centers and cluster assignment */
	printf("centers found:\n");
	for(j = 0; j < k; j++)
		printf("%lf, %lf\n", u[j].x, u[j].y);
		    

	return 0;
}

