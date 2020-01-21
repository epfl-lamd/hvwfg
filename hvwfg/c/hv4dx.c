// This software is Copyright (C) 2010 Lyndon While, Lucas Bradstreet. 

// This program is free software (software libre). You can redistribute it and/or modify it under 
// the terms of the GNU General Public License as published by the Free Software Foundation; 
// either version 2 of the License, or (at your option) any later version. 

// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU General Public License for more details. 

// Source: http://www.wfg.csse.uwa.edu.au/hypervolume/
#include "wfg.h"
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct BOX {
	OBJECTIVE corners[5];
	struct BOX* neighbours[2];
} BOX;

#define BEATS(x,y) (x > y) 
#define MAX(a,b) ((a > b) ? (a) : (b))
#define MIN(a,b) ((a < b) ? (a) : (b))

#define NUMOBJ 4

POINT ref;
int maxm;
BOX* boxes;
int boxi;
BOX boxF;
BOX boxL;

double totaltime;

int greater(const void *v1, const void *v2)
// this sorts points worsening in the last objective
{
	POINT p = *(POINT*)v1;
	POINT q = *(POINT*)v2;
	for (int i = NUMOBJ - 1; i >= 0; i--) {
		if BEATS(p.objectives[i],q.objectives[i]) {
			return -1;
		}
		else if BEATS(q.objectives[i],p.objectives[i]) {
			return  1;
		}
	}
	return 0;
}

void insert(BOX* add, BOX* place, int ni) {
	BOX* n = place->neighbours[ni];
	place->neighbours[ni] = add;
	add->neighbours[1-ni] = place;
	add->neighbours[ni] = n;
	n->neighbours[1-ni] = add;
} 

void insertAbove(BOX* add, BOX* place) {
	insert(add,place,0);
}

void insertBelow(BOX* add, BOX* place) {
	insert(add,place,1);
}

void deleteBox(BOX* box) {
	BOX* left = box->neighbours[0];
	BOX* right = box->neighbours[1];
	left->neighbours[1] = right;
	right->neighbours[0] = left;
}

double contribution(POINT p) {
	double volume = 0;
	BOX* current = boxF.neighbours[1];
	double maxXAbove = 0;
	double minYAbove = current->corners[1] + 1;
	double area = 0;
	double totalarea = 0;
	int inbi = boxi;
	int extra = -1;
	BOX* insertPoint = NULL;
	while (current != &boxL) {
		BOX* nextcurr = current->neighbours[1];
		bool closed = false;
		if (current->corners[2] < p.objectives[2]) {
			if (current->corners[3] < p.objectives[0] && current->corners[4] < p.objectives[1]) {
				if (current->corners[0] < p.objectives[0]) {
					if (current->corners[1] < p.objectives[1]) {
						volume += fabs(current->corners[0] - current->corners[3]) * fabs(current->corners[1] - current->corners[4]) * 
							fabs(current->corners[2] - p.objectives[2]);
						area += fabs(current->corners[0] - current->corners[3]) * fabs(current->corners[1] - current->corners[4]);
						deleteBox(current);
						closed = true;
					}
					else {
						volume += fabs(current->corners[0] - current->corners[3]) * fabs(p.objectives[1] - current->corners[4]) * 
							fabs(current->corners[2] - p.objectives[2]);
						area += fabs(current->corners[0] - current->corners[3]) * fabs(p.objectives[1] - current->corners[4]);
						current->corners[4] = p.objectives[1];
					}
				}
				else if (current->corners[1] < p.objectives[1]) {
					volume += fabs(p.objectives[0] - current->corners[3]) * fabs(current->corners[1] - current->corners[4]) * 
						fabs(current->corners[2] - p.objectives[2]);
					area += fabs(p.objectives[0] - current->corners[3]) * fabs(current->corners[1] - current->corners[4]);
					current->corners[3] = p.objectives[0];
				}
				else {
					volume += fabs(p.objectives[0] - current->corners[3]) * fabs(p.objectives[1] - current->corners[4]) * 
						fabs(current->corners[2] - p.objectives[2]);
					area += fabs(p.objectives[0] - current->corners[3]) * fabs(p.objectives[1] - current->corners[4]);
					boxes[boxi].corners[0] = p.objectives[0];
					boxes[boxi].corners[1] = current->corners[1];
					boxes[boxi].corners[2] = current->corners[2];
					boxes[boxi].corners[3] = current->corners[3];
					boxes[boxi].corners[4] = p.objectives[1];
					current->corners[3] = p.objectives[0];
					insertBelow(&boxes[boxi],current);
					extra = boxi;
					boxi++;
				}
			}
		}
		else {
			if (current->corners[0] > maxXAbove && current->corners[1] < minYAbove) {
				if (current->corners[1] < p.objectives[1] && maxXAbove < p.objectives[0]) {
					boxes[boxi].corners[0] = MIN(current->corners[0],p.objectives[0]);
					boxes[boxi].corners[1] = p.objectives[1];
					boxes[boxi].corners[2] = p.objectives[2];
					boxes[boxi].corners[3] = maxXAbove;
					boxes[boxi].corners[4] = current->corners[1];
					totalarea += fabs(boxes[boxi].corners[0] - boxes[boxi].corners[3]) * fabs(boxes[boxi].corners[1] - boxes[boxi].corners[4]);
					boxi++;
				}
				maxXAbove = current->corners[0];
				minYAbove = current->corners[1];
			}
		}
		if (insertPoint == NULL && !closed && current->corners[1] < p.objectives[1]) {
			insertPoint = current;
		}
		current = nextcurr;
	}
	if (insertPoint == NULL) {
		insertPoint = &boxL;
	}
	if (maxXAbove < p.objectives[0]) {
		boxes[boxi].corners[0] = p.objectives[0];
		boxes[boxi].corners[1] = p.objectives[1];
		boxes[boxi].corners[2] = p.objectives[2];
		boxes[boxi].corners[3] = maxXAbove;
		boxes[boxi].corners[4] = 0;
		totalarea += fabs(boxes[boxi].corners[0] - boxes[boxi].corners[3]) * fabs(boxes[boxi].corners[1] - boxes[boxi].corners[4]);
		boxi++;
	}
	for (int i=inbi; i<boxi; i++) {
		if (i == extra) {
			continue;
		}
		insertAbove(&boxes[i],insertPoint);
		insertPoint = &boxes[i];
	}
	volume += (totalarea - area) * p.objectives[2];
	return volume;
}

double initialiseBoxes(POINT p) {
	boxi = 0;
	boxes[boxi].corners[0] = p.objectives[0];
	boxes[boxi].corners[1] = p.objectives[1];
	boxes[boxi].corners[2] = p.objectives[2];
	boxes[boxi].corners[3] = 0;
	boxes[boxi].corners[4] = 0;
	boxF.neighbours[1] = &boxes[boxi];
	boxes[boxi].neighbours[0] = &boxF;
	boxes[boxi].neighbours[1] = &boxL;
	boxL.neighbours[0] = &boxes[boxi];
	boxi++;
	return p.objectives[0] * p.objectives[1] * p.objectives[2];
}

double hv(FRONT ps) {
	qsort(ps.points, ps.nPoints, sizeof(POINT), greater);
	double hypervolume = 0;
	double volume = initialiseBoxes(ps.points[0]);
	for (int i=1; i<ps.nPoints; i++) {
		hypervolume += volume * fabs(ps.points[i].objectives[3] - ps.points[i-1].objectives[3]);
		volume += contribution(ps.points[i]);
	}
	hypervolume += volume * ps.points[ps.nPoints-1].objectives[3];
	return hypervolume;
}

void allocate_memory(int maxm_, int maxn_)
{
	maxm = maxm_;
	boxes = malloc(sizeof(BOX) * maxm * maxm);

}
void free_memory() {
	free(boxes);
}

// int main(int argc, char *argv[]) 
// // processes each front from the file 
// {
// 	FILECONTENTS *f = readFile(argv[1]);

// 	//check number of objectives and points
// 	maxm = 0;
// 	for (int i=0; i<f->nFronts; i++) {
// 		if (f->fronts[i].n != NUMOBJ) {
// 			printf("This algorithm is for the %dD case only\n", NUMOBJ);
// 			return 0;
// 		}
// 		maxm = MAX(maxm,f->fronts[i].nPoints);
// 	}

// 	boxes = malloc(sizeof(BOX) * maxm * maxm);

// 	// initialise the reference point
// 	ref.objectives = malloc(sizeof(OBJECTIVE) * NUMOBJ);
// 	if (argc == 2) {
// 		printf("No reference point provided: using the origin\n");
// 		for (int i = 0; i < NUMOBJ; i++) {
// 			ref.objectives[i] = 0;
// 		}
// 	}
// 	else if (argc - 2 != NUMOBJ) {
// 		printf("Your reference point should have %d values\n", NUMOBJ);
// 		return 0;
// 	}
// 	else {
// 		for (int i = 2; i < argc; i++) {
// 			ref.objectives[i - 2] = atof(argv[i]);
// 		}
// 	}

// 	// modify the objective values relative to the reference point 
// 	for (int i = 0; i < f->nFronts; i++) {
// 		for(int j = 0; j < f->fronts[i].nPoints; j++) {
// 			for(int k = 0; k < NUMOBJ; k++) {
// 				f->fronts[i].points[j].objectives[k] = fabs(f->fronts[i].points[j].objectives[k] - ref.objectives[k]);
// 			}
// 		}
// 	}

// 	totaltime = 0;
// 	for (int i = 0; i < f->nFronts; i++) {
// 		struct timeval tv1, tv2;
// 		struct rusage ru_before, ru_after;
// 		getrusage (RUSAGE_SELF, &ru_before);

// 		printf("hv(%d) = %1.10f\n", i+1, hv(f->fronts[i])); 

// 		getrusage (RUSAGE_SELF, &ru_after);
// 		tv1 = ru_before.ru_utime;
// 		tv2 = ru_after.ru_utime;
// 		printf("Time: %f (s)\n", tv2.tv_sec + tv2.tv_usec * 1e-6 - tv1.tv_sec - tv1.tv_usec * 1e-6);
// 		totaltime += tv2.tv_sec + tv2.tv_usec * 1e-6 - tv1.tv_sec - tv1.tv_usec * 1e-6;
// 	}
// 	printf("Total time = %f (s)\n", totaltime);

// 	return 0;
// }