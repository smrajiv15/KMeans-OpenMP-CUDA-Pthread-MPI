/*
 * Data reading function for CS6643 Parallel Processing, 
 * Assignment 2: K-Means Algorithm (Sequential)
 *
 * To students: You should not modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "k_means.h"

/*
 * read data points from input file
 */
int read_points_from_file(char *data_file, struct point *pts, int *m)
{
	FILE *fp;
	size_t len = 0;
	ssize_t read = 0;
	char *line = NULL;
	int ret = 0;

	/* open the file */
	fp = fopen(data_file, "r");
	if(fp == NULL)
		err(-1, "Unable to open file");

	*m = 0;

	/* read in the coordinates of the points */
	while((read = getline(&line, &len, fp)) != -1){
		ret = sscanf(line, "%lf,%lf\n", &(pts[*m].x),
			     &(pts[*m].y));
		if(ret != 2)
			continue;

		/* increase the size of pts if necessary */
		(*m)++;
		if(*m == MAX_POINTS){
			printf("Too many data points (maximum %d points)\n",
			       MAX_POINTS);
			exit(-1);
		}
	}

	fclose(fp);

	if(line)
		free(line);

#ifdef DEBUG
	{
		int i;
		for(i = 0; i < *m; i++)
			printf("%lf,%lf\n", pts[i].x, pts[i].y);
	}
		
#endif
      

	return 0;
}
