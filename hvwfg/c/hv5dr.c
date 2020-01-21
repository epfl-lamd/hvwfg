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
#include <stdio.h>
#include <stdbool.h>

typedef struct NODE {
	POINT p;
	struct NODE* neighbours[3][2];
	int x;
} NODE;

typedef struct BOX {
	OBJECTIVE corners[5];
	struct BOX* neighbours[2];
} BOX;

typedef struct BOXOID {
	OBJECTIVE corners[7];
	struct BOXOID* neighbours[3][2];
} BOXOID;

#define BEATS(x,y) (x > y) 
#define MAX(a,b) ((a > b) ? (a) : (b))

#define NUMOBJ 5

POINT ref;
int maxm;
NODE ndsF;
NODE ndsL;
NODE upper4F;
NODE upper4L;
NODE lower4F;
NODE lower4L;
NODE upper3F;
NODE upper3L;
NODE lower3F;
NODE lower3L;
NODE* nodes;
BOX* boxes;
BOXOID* boxoids;
BOXOID boxoidF;
BOXOID boxoidL;
BOX boxF;
BOX boxL;
int boxi;
int boxoidi;

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

void appendBoxoid(BOXOID* add, BOXOID* end, int ni)
{
	BOXOID* prev = end->neighbours[ni][0];
	prev->neighbours[ni][1] = add;
	add->neighbours[ni][0] = prev;
	add->neighbours[ni][1] = end;
	end->neighbours[ni][0] = add;
}

NODE* prevNode(NODE* p, NODE* start, int d)
{
	if (p == NULL) {
		return NULL;
	}
	NODE* prev = p->neighbours[d - 1][0];
	if (prev == start) {
		return NULL;
	}
	else {
		return prev;
	}
}

void insertBoxoid(BOXOID* start, BOXOID* end, int ni)
{
	BOXOID* prev = end;
	BOXOID* next = prev->neighbours[ni][0];
	while (next != start && next->corners[4 + ni] < boxoids[boxoidi].corners[4 + ni]) {
		prev = next;
		next = next->neighbours[ni][0];
	}
	prev->neighbours[ni][0] = &boxoids[boxoidi];
	boxoids[boxoidi].neighbours[ni][1] = prev;
	boxoids[boxoidi].neighbours[ni][0] = next;
	next->neighbours[ni][1] = &boxoids[boxoidi];
}

void insertBox(BOX* prev)
{
	BOX* next = prev->neighbours[1];
	prev->neighbours[1] = &boxes[boxi];
	boxes[boxi].neighbours[0] = prev;
	boxes[boxi].neighbours[1] = next;
	next->neighbours[0] = &boxes[boxi];
}

void pushRight() 
{
	BOX* neighbour = boxL.neighbours[0];
	boxL.neighbours[0] = &boxes[boxi];
	boxes[boxi].neighbours[1] = &boxL;
	boxes[boxi].neighbours[0] = neighbour;
	neighbour->neighbours[1] = &boxes[boxi];
}

void pushLeft() 
{
	BOX* neighbour = boxF.neighbours[1];
	boxF.neighbours[1] = &boxes[boxi];
	boxes[boxi].neighbours[0] = &boxF;
	boxes[boxi].neighbours[1] = neighbour;
	neighbour->neighbours[0] = &boxes[boxi];
}

void closeBoxCentre(POINT q, double a)
{
	BOX* current = boxF.neighbours[1];
	while (current != &boxL && current->corners[3] > q.objectives[0]) {
		current = current->neighbours[1];
	}
	if (current != &boxL && current->corners[4] < q.objectives[1]) {
		boxoids[boxoidi].corners[0] = q.objectives[0];
		boxoids[boxoidi].corners[1] = q.objectives[1];
		boxoids[boxoidi].corners[2] = current->corners[2];
		boxoids[boxoidi].corners[3] = a;
		boxoids[boxoidi].corners[4] = current->corners[3];
		boxoids[boxoidi].corners[5] = current->corners[4];
		boxoids[boxoidi].corners[6] = q.objectives[2];
		insertBoxoid(&boxoidF,&boxoidL,0);
		insertBoxoid(&boxoidF,&boxoidL,1);
		appendBoxoid(&boxoids[boxoidi],&boxoidL,2);
		boxoidi++;
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		boxes[boxi].corners[0] = current->corners[0];
		boxes[boxi].corners[1] = current->corners[1];
		boxes[boxi].corners[2] = current->corners[2];
		boxes[boxi].corners[3] = q.objectives[0];
		boxes[boxi].corners[4] = current->corners[4];
		insertBox(prev);
		boxi++;
		boxes[boxi].corners[0] = q.objectives[0];
		boxes[boxi].corners[1] = current->corners[1];
		boxes[boxi].corners[2] = current->corners[2];
		boxes[boxi].corners[4] = q.objectives[1];
		BOX* last = current;
		current = current->neighbours[1];
		while (current != &boxL && current->corners[4] < q.objectives[1]) {
			boxoids[boxoidi].corners[0] = current->corners[0];
			boxoids[boxoidi].corners[1] = q.objectives[1];
			boxoids[boxoidi].corners[2] = current->corners[2];
			boxoids[boxoidi].corners[3] = a;
			boxoids[boxoidi].corners[4] = current->corners[3];
			boxoids[boxoidi].corners[5] = current->corners[4];
			boxoids[boxoidi].corners[6] = q.objectives[2];
			insertBoxoid(&boxoidF,&boxoidL,0);
			insertBoxoid(&boxoidF,&boxoidL,1);
			appendBoxoid(&boxoids[boxoidi],&boxoidL,2);
			boxoidi++;
			BOX* prev = current->neighbours[0];
			BOX* next = current->neighbours[1];
			prev->neighbours[1] = next;
			next->neighbours[0] = prev;
			last = current;
			current = current->neighbours[1];
		}
		boxes[boxi].corners[3] = last->corners[3];
		insertBox(&boxes[boxi-1]);
		boxi++;
	}
}

void closeBoxesRight(POINT q, double a)
{
	bool add = false;
	BOX* current = boxL.neighbours[0];
	while (current != &boxF && current->corners[3] < q.objectives[0]) {
		if (current->corners[0] > q.objectives[0]) {
			boxes[boxi].corners[0] = current->corners[0];
			boxes[boxi].corners[1] = current->corners[1];
			boxes[boxi].corners[2] = current->corners[2];
			boxes[boxi].corners[3] = q.objectives[0];
			boxes[boxi].corners[4] = current->corners[4];
			current->corners[0] = q.objectives[0];
			add = true;
		}
		boxoids[boxoidi].corners[0] = current->corners[0];
		boxoids[boxoidi].corners[1] = current->corners[1];
		boxoids[boxoidi].corners[2] = current->corners[2];
		boxoids[boxoidi].corners[3] = a;
		boxoids[boxoidi].corners[4] = current->corners[3];
		boxoids[boxoidi].corners[5] = current->corners[4];
		boxoids[boxoidi].corners[6] = q.objectives[2];
		insertBoxoid(&boxoidF,&boxoidL,0);
		insertBoxoid(&boxoidF,&boxoidL,1);
		appendBoxoid(&boxoids[boxoidi],&boxoidL,2);
		boxoidi++;
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		current = current->neighbours[0];
	}
	if (add) {
		pushRight();
		boxi++;
	}
}

void closeBoxesLeft(POINT p, POINT q)
{
	double lastx = -1;
	BOX* current = boxF.neighbours[1];
	while (current != &boxL && current->corners[4] < q.objectives[1]) {
		boxoids[boxoidi].corners[0] = current->corners[0];
		boxoids[boxoidi].corners[1] = q.objectives[1];
		boxoids[boxoidi].corners[2] = current->corners[2];
		boxoids[boxoidi].corners[3] = p.objectives[3];
		boxoids[boxoidi].corners[4] = current->corners[3];
		boxoids[boxoidi].corners[5] = current->corners[4];
		boxoids[boxoidi].corners[6] = q.objectives[2];
		insertBoxoid(&boxoidF,&boxoidL,0);
		insertBoxoid(&boxoidF,&boxoidL,1);
		appendBoxoid(&boxoids[boxoidi],&boxoidL,2);
		boxoidi++;
		lastx = current->corners[3];
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		current = current->neighbours[1];
	}
	if (lastx >= 0) {
		boxes[boxi].corners[0] = p.objectives[0];
		boxes[boxi].corners[1] = p.objectives[1];
		boxes[boxi].corners[2] = p.objectives[2];
		boxes[boxi].corners[3] = lastx;
		boxes[boxi].corners[4] = q.objectives[1];
		pushLeft();
		boxi++;
	}
}

void closeAllBoxes(double z, double a)
{
	BOX* current = boxF.neighbours[1];
	while (current != &boxL) {
		boxoids[boxoidi].corners[0] = current->corners[0];
		boxoids[boxoidi].corners[1] = current->corners[1];
		boxoids[boxoidi].corners[2] = current->corners[2];
		boxoids[boxoidi].corners[3] = a;
		boxoids[boxoidi].corners[4] = current->corners[3];
		boxoids[boxoidi].corners[5] = current->corners[4];
		boxoids[boxoidi].corners[6] = z;
		insertBoxoid(&boxoidF,&boxoidL,0);
		insertBoxoid(&boxoidF,&boxoidL,1);
		appendBoxoid(&boxoids[boxoidi],&boxoidL,2);
		boxoidi++;
		current = current->neighbours[1];
	}
	boxF.neighbours[1] = &boxL;
	boxL.neighbours[0] = &boxF;
}

NODE* getXRightAbove(POINT p, NODE* start, NODE* end)
{
	NODE* current = end->neighbours[0][0];
	while (current != start && current->p.objectives[0] > p.objectives[0] && current->p.objectives[1] >= p.objectives[1]) {
		current = current->neighbours[0][0];
	}
	if (current == start || current->p.objectives[1] < p.objectives[1]) {
		return NULL;
	}
	else {
		NODE* q = current;
		current = current->neighbours[0][0];
		while (current != start && current->p.objectives[1] >= p.objectives[1]) {
			if (current->p.objectives[0] <= p.objectives[0] && current->p.objectives[0] > q->p.objectives[0]) {
				q = current;
			}
			current = current->neighbours[0][0];
		}
		return q;
	}
}

NODE* lowerNode(POINT p, NODE* start, NODE* end, int d) {
	NODE* prev = start;
	NODE* current = start->neighbours[d - 1][1];
	while (current != end && current->p.objectives[d] < p.objectives[d]) {
		prev = current;
		current = current->neighbours[d - 1][1];
	}
	if (prev == start) {
		return NULL;
	}
	else {
		return prev;
	}
}

void initialiseBoxes(POINT p)
{
	boxF.neighbours[0] = NULL;
	boxF.neighbours[1] = &boxL;
	boxL.neighbours[0] = &boxF;
	boxL.neighbours[1] = NULL;
	NODE* q = lowerNode(p,&upper3F,&upper3L,1);
	NODE* m = getXRightAbove(p,&upper3F,&upper3L);
	boxi = 0;
	while (q != NULL && q->p.objectives[0] < p.objectives[0]) {
		if (m == NULL) {
			boxes[boxi].corners[0] = q->p.objectives[0];
			boxes[boxi].corners[1] = p.objectives[1];
			boxes[boxi].corners[2] = p.objectives[2];
			boxes[boxi].corners[3] = 0;
			boxes[boxi].corners[4] = q->p.objectives[1];
			pushLeft();
			boxi++;
			m = q;
		}
		else if (q->p.objectives[0] > m->p.objectives[0]) {
			boxes[boxi].corners[0] = q->p.objectives[0];
			boxes[boxi].corners[1] = p.objectives[1];
			boxes[boxi].corners[2] = p.objectives[2];
			boxes[boxi].corners[3] = m->p.objectives[0];
			boxes[boxi].corners[4] = q->p.objectives[1];
			pushLeft();
			boxi++;
			m = q;
		}
		q = prevNode(q,&upper3F,1);
	}
	boxes[boxi].corners[0] = p.objectives[0];
	boxes[boxi].corners[1] = p.objectives[1];
	boxes[boxi].corners[2] = p.objectives[2];
	if (m != NULL) {
		boxes[boxi].corners[3] = m->p.objectives[0];
	}
	else {
		boxes[boxi].corners[3] = 0;
	}
	if (q != NULL) {
		boxes[boxi].corners[4] = q->p.objectives[1];
	}
	else {
		boxes[boxi].corners[4] = 0;
	}
	pushLeft();
	boxi++;
}

void appendNode(NODE* add, NODE* end, int ni) 
{
	NODE* prev = end->neighbours[ni][0];
	prev->neighbours[ni][1] = add;
	add->neighbours[ni][0] = prev;
	add->neighbours[ni][1] = end;
	end->neighbours[ni][0] = add;
}

void split(double pv, int depth, NODE* start, NODE* end, NODE* upF, NODE* upL, NODE* loF, NODE* loL) 
{
	for (int i=0; i<4 - depth; i++) {
		upF->neighbours[i][0] = NULL;
		upF->neighbours[i][1] = upL;
		upL->neighbours[i][0] = upF;
		upL->neighbours[i][1] = NULL;
		loF->neighbours[i][0] = NULL;
		loF->neighbours[i][1] = loL;
		loL->neighbours[i][0] = loF;
		loL->neighbours[i][1] = NULL;
	}
	for (int i=0; i<4 - depth; i++) {
		NODE* current = start->neighbours[i][1];
		while (current != end) {
			if (i == 0) {
				nodes[depth * maxm + current->x].p = current->p;
				nodes[depth * maxm + current->x].x = current->x;
			}
			if (current->p.objectives[4 - depth] >= pv) {
				appendNode(&nodes[depth * maxm + current->x],upL,i);
			}
			else {
				appendNode(&nodes[depth * maxm + current->x],loL,i);
			}
			current = current->neighbours[i][1];
		}
	}
}

void initialiseBoxoids(POINT p)
{
	for (int i=0; i<3; i++) {
		boxoidF.neighbours[i][0] = NULL;
		boxoidF.neighbours[i][1] = &boxoidL;
		boxoidL.neighbours[i][0] = &boxoidF;
		boxoidL.neighbours[i][1] = NULL;
	}
	boxoidi = 0;
	if (upper4F.neighbours[2][1] == &upper4L) {
		for (int i=0; i<3; i++) {
			boxoids[0].corners[i] = p.objectives[i];
			boxoids[0].corners[4 + i] = 0;
			boxoidF.neighbours[i][1] = &boxoids[0];
			boxoidL.neighbours[i][0] = &boxoids[0];
			boxoids[0].neighbours[i][0] = &boxoidF;
			boxoids[0].neighbours[i][1] = &boxoidL;
		}
		boxoids[0].corners[3] = p.objectives[3];
		boxoidi++;
	}
	else {
		split(p.objectives[2],2,&upper4F,&upper4L,&upper3F,&upper3L,&lower3F,&lower3L);
		initialiseBoxes(p);
		NODE* q = lowerNode(p,&lower3F,&lower3L,2);
		while (boxF.neighbours[1] != &boxL) {
			if (q == NULL) {
				closeAllBoxes(0,p.objectives[3]);
				break;
			}
			else if (q->p.objectives[0] > p.objectives[0]) {
				if (q->p.objectives[1] > p.objectives[1]) {
					closeAllBoxes(q->p.objectives[2],p.objectives[3]);
					break;
				}
				else {
					closeBoxesLeft(p,q->p);
				}
			}
			else if (q->p.objectives[1] > p.objectives[1]) {
				closeBoxesRight(q->p,p.objectives[3]);
			}
			else {
				closeBoxCentre(q->p,p.objectives[3]);
			}
			q = prevNode(q,&lower3F,2);
		}
	}
}

double inclhv(POINT p, int numobj)
{
	double hvol = p.objectives[0];
	for (int i=1; i<numobj; i++) {
		hvol *= p.objectives[i];
	}
	return hvol;
}

void insertBoxoidAbove(BOXOID* place, int ni)
{
	BOXOID* prev = place;
	BOXOID* next = place->neighbours[ni][0];
	while (next != &boxoidF && next->corners[4 + ni] < boxoids[boxoidi].corners[4 + ni]) {
		prev = next;
		next = next->neighbours[ni][0];
	}
	next->neighbours[ni][1] = &boxoids[boxoidi];
	boxoids[boxoidi].neighbours[ni][0] = next;
	boxoids[boxoidi].neighbours[ni][1] = prev;
	prev->neighbours[ni][0] = &boxoids[boxoidi];
}

void insertBoxoidBelow(BOXOID* place, int ni)
{
	BOXOID* neighbour = place->neighbours[ni][1];
	place->neighbours[ni][1] = &boxoids[boxoidi];
	boxoids[boxoidi].neighbours[ni][0] = place;
	boxoids[boxoidi].neighbours[ni][1] = neighbour;
	neighbour->neighbours[ni][0] = &boxoids[boxoidi];
}

double closeFor(POINT p, POINT q)
{
	double hvol = 0;
	bool below[3] = {false,false,false};
	int first = 2;
	for (int i=2; i>=0; i--) {
		if (q.objectives[i] < p.objectives[i]) {
			below[i] = true;
			first = i;
		}
	}
	BOXOID* place[3] = {NULL,NULL,NULL};
	BOXOID* current = boxoidL.neighbours[first][0];
	while (current != &boxoidF && current->corners[4 + first] < q.objectives[first]) {
		BOXOID* nextCurr = current->neighbours[first][0];
		bool toClose = true;
		for (int i=first + 1; i<3; i++) {
			if (below[i] && current->corners[4 + i] > q.objectives[i]) {
				toClose = false;
				break;
			}
		}
		if (toClose) {
			for (int i=first; i<3; i++) {
				if (below[i] && current->corners[i] > q.objectives[i]) {
					for (int j=0; j<7; j++) {
						boxoids[boxoidi].corners[j] = current->corners[j];
					}
					boxoids[boxoidi].corners[4 + i] = q.objectives[i];
					current->corners[i] = q.objectives[i];
					for (int j=0; j<3; j++) {
						if (i == j) {
							if (place[j] == NULL) {
								insertBoxoidAbove(current,j);
								place[j] = &boxoids[boxoidi];
							}
							else {
								insertBoxoidBelow(place[j],j);
							}
						}
						else {
							insertBoxoidBelow(current,j);
						}
					}
					boxoidi++;
				}
			}
			hvol += fabs(current->corners[0]-current->corners[4]) * fabs(current->corners[1]-current->corners[5]) * 
				fabs(current->corners[2]-current->corners[6]) * fabs(current->corners[3]-q.objectives[3]);
			for (int i=0; i<3; i++) {
				BOXOID* prev = current->neighbours[i][0];
				BOXOID* next = current->neighbours[i][1];
				prev->neighbours[i][1] = next;
				next->neighbours[i][0] = prev;
			}
		}
		current = nextCurr;
	}
	return hvol;
}

double closeAllBoxoids(double a)
{
	double hvol = 0;
	BOXOID* current = boxoidF.neighbours[0][1];
	while (current != &boxoidL) {
		hvol += fabs(current->corners[0] - current->corners[4]) * fabs(current->corners[1] - current->corners[5]) * 
			fabs(current->corners[2] - current->corners[6]) * fabs(current->corners[3] - a);
		current = current->neighbours[0][1];
	}
	for (int i=0; i<3; i++) {
		boxoidF.neighbours[i][1] = &boxoidL;
		boxoidL.neighbours[i][0] = &boxoidF;
	}
	return hvol;
}

bool dominates(POINT a, POINT b, int numobj)
{
	for (int i=0; i<numobj; i++) {
		if (a.objectives[i] < b.objectives[i]) {
			return false;
		}
	}
	return true;
}

double contribution(POINT p) 
{
	if (ndsF.neighbours[2][1] == &ndsL) {
		return inclhv(p,4);
	}
	split(p.objectives[3],1,&ndsF,&ndsL,&upper4F,&upper4L,&lower4F,&lower4L);
	initialiseBoxoids(p);
	double c = 0;
	NODE* q = lowerNode(p,&lower4F,&lower4L,3);
	while (boxoidF.neighbours[0][1] != &boxoidL) {
		if (q==NULL) {
			c += closeAllBoxoids(0);
			return c;
		}
		else if (dominates(q->p,p,3)) {
			c += closeAllBoxoids(q->p.objectives[3]);
			return c;
		}
		else {
			c += closeFor(p,q->p);
		}
		q = prevNode(q,&lower4F,3);
	}
	return c;
}

void delete(NODE* n) 
{
	for (int i=0; i<3; i++) {
		NODE* prev = n->neighbours[i][0];
		NODE* next = n->neighbours[i][1];
		prev->neighbours[i][1] = next;
		next->neighbours[i][0] = prev;
	}
}

void insert(NODE *add, NODE* start, NODE* end) 
{
	for (int i=0; i<3; i++) {
		NODE* prev = start;
		NODE* next = start->neighbours[i][1];
		while (next != end && next->p.objectives[i + 1] < add->p.objectives[i + 1]) {
			prev = next;
			next = next->neighbours[i][1];
		}
		prev->neighbours[i][1] = add;
		add->neighbours[i][0] = prev;
		add->neighbours[i][1] = next;
		next->neighbours[i][0] = add;
	}
}

void initialiseNodes()
{
	for (int i=0; i<3; i++) {
		ndsF.neighbours[i][0] = NULL;
		ndsF.neighbours[i][1] = &ndsL;
		ndsL.neighbours[i][0] = &ndsF;
		ndsL.neighbours[i][1] = NULL;
	}
}

double hv(FRONT ps)
{
	qsort(ps.points, ps.nPoints, sizeof(POINT), greater);
	initialiseNodes();
	double hypervolume = 0;
	double cross = inclhv(ps.points[0],4);
	nodes[0].p = ps.points[0];
	nodes[0].x = 0;
	insert(&nodes[0],&ndsF,&ndsL);
	for (int i=1; i<ps.nPoints; i++) {
		hypervolume += cross * fabs(ps.points[i].objectives[4] - ps.points[i-1].objectives[4]);
		NODE* current = ndsF.neighbours[2][1];
		cross += contribution(ps.points[i]);
		while (current != &ndsL && current->p.objectives[3] < ps.points[i].objectives[3]) {
			if (dominates(ps.points[i],current->p,3)) {
				delete(current);
			}
			current = current->neighbours[2][1];
		}
		nodes[i].p = ps.points[i];
		nodes[i].x = i;
		insert(&nodes[i],&ndsF,&ndsL);
	}
	hypervolume += cross * ps.points[ps.nPoints-1].objectives[4];
	return hypervolume;
}

void allocate_memory(int maxm_, int maxn_)
{
	maxm = maxm_;
	nodes = malloc(sizeof(NODE) * maxm * 3);
	boxes = malloc(sizeof(BOX) * maxm * 2);
	boxoids = malloc(sizeof(BOXOID) * maxm);
}

void free_memory()
{
	free(nodes);
	free(boxes);
	free(boxoids);
	printf("Total boxoids %d\n", boxoidi);
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

// 	//allocate memory
// 	nodes = malloc(sizeof(NODE) * maxm * 3);
// 	boxes = malloc(sizeof(BOX) * maxm * 2);
// 	boxoids = malloc(sizeof(BOXOID) * maxm * maxm);

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
// 	printf("nfronts = %d\n",f->nFronts);
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