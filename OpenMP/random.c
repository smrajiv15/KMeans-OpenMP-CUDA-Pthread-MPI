/*
 * Random (fake) function for CS6643 Parallel Processing, 
 * Assignment 2: K-Means Algorithm (Sequential)
 *
 * To students: You should not modify this file
 * 
 * Author:
 *     Wei Wang <wei.wang@utsa.edu>
 */
#include <stdlib.h>

#include "k_means.h"

struct point randoms[] = {{-1805.06957404 , 9357.72018133},
	{10844.1192027 , -13499.4383799},		
		{23335.0617138 , 7270.43336913},		
			{-4125.25298643 , -7844.05594317},
				{21917.4184154 , 10883.3150865},
					{4425.07624075 , 37809.9719176},
						{3337.20156029 , 17249.5907789},
							{-4517.60125126 , 20086.7275896}};

int counter = 0;
int max = 200;
int min = -200;

struct point random_center(struct point p[MAX_POINTS])
{
	int idx = 0;
	idx = __sync_fetch_and_add(&counter, 1)%MAX_POINTS;
	return p[idx];
	//struct point u;
	//u.x = (max - min) * ( (double)rand() / (double)RAND_MAX ) + min;
	//u.y = (max - min) * ( (double)rand() / (double)RAND_MAX ) + min;

	//return u;
}
