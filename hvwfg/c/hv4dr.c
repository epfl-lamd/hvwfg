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
	struct NODE* neighboursY[2];
	struct NODE* neighboursZ[2];
	int x;
} NODE;

typedef struct BOX {
	OBJECTIVE corners[6];
	struct BOX* neighbours[2];
} BOX;

#define BEATS(x,y) (x > y) 
#define MAX(a,b) ((a > b) ? (a) : (b))

#define NUMOBJ 4

POINT ref;
int maxm;
NODE *nodes;
NODE firstN;
NODE lastN;
NODE s1f;
NODE s1e;
NODE s2f;
NODE s2e;
BOX firstB;
BOX lastB;
BOX *boxes;
int bindex;

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

NODE* prevZ(NODE* p, NODE* start)
{
	NODE* prev = p->neighboursZ[0];
	if (prev == start) {
		return NULL;
	}
	else {
		return prev;
	}
}

void insertBox(BOX* prev)
{
	BOX* next = prev->neighbours[1];
	prev->neighbours[1] = &boxes[bindex];
	boxes[bindex].neighbours[0] = prev;
	boxes[bindex].neighbours[1] = next;
	next->neighbours[0] = &boxes[bindex];
}

void pushRight() 
{
	BOX* neighbour = lastB.neighbours[0];
	lastB.neighbours[0] = &boxes[bindex];
	boxes[bindex].neighbours[1] = &lastB;
	boxes[bindex].neighbours[0] = neighbour;
	neighbour->neighbours[1] = &boxes[bindex];
}

void pushLeft() 
{
	BOX* neighbour = firstB.neighbours[1];
	firstB.neighbours[1] = &boxes[bindex];
	boxes[bindex].neighbours[0] = &firstB;
	boxes[bindex].neighbours[1] = neighbour;
	neighbour->neighbours[0] = &boxes[bindex];
}

double closeBoxCentre(POINT q)
{
	double volume = 0;
	BOX* current = firstB.neighbours[1];
	while (current != &lastB && current->corners[3] > q.objectives[0]) {
		current = current->neighbours[1];
	}
	if (current != &lastB) {
		volume += fabs(q.objectives[0]-current->corners[3]) * fabs(q.objectives[1]-current->corners[4]) * fabs(current->corners[2]-q.objectives[2]);
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		boxes[bindex].corners[0] = current->corners[0];
		boxes[bindex].corners[1] = current->corners[1];
		boxes[bindex].corners[2] = current->corners[2];
		boxes[bindex].corners[3] = q.objectives[0];
		boxes[bindex].corners[4] = current->corners[4];
		insertBox(prev);
		bindex++;
		boxes[bindex].corners[0] = q.objectives[0];
		boxes[bindex].corners[1] = current->corners[1];
		boxes[bindex].corners[2] = current->corners[2];
		boxes[bindex].corners[4] = q.objectives[1];
		BOX* last = current;
		current = current->neighbours[1];
		while (current != &lastB && current->corners[4] < q.objectives[1]) {
			volume += fabs(current->corners[0]-current->corners[3]) * fabs(q.objectives[1]-current->corners[4]) * 
				fabs(current->corners[2]-q.objectives[2]);
			BOX* prev = current->neighbours[0];
			BOX* next = current->neighbours[1];
			prev->neighbours[1] = next;
			next->neighbours[0] = prev;
			last = current;
			current = current->neighbours[1];
		}
		boxes[bindex].corners[3] = last->corners[3];
		insertBox(&boxes[bindex-1]);
		bindex++;
	}
	return volume;
}

double closeBoxesRight(POINT q)
{
	bool add = false;
	double volume = 0;
	BOX* current = lastB.neighbours[0];
	while (current != &firstB && current->corners[3] < q.objectives[0]) {
		if (current->corners[0] > q.objectives[0]) {
			boxes[bindex].corners[0] = current->corners[0];
			boxes[bindex].corners[1] = current->corners[1];
			boxes[bindex].corners[2] = current->corners[2];
			boxes[bindex].corners[3] = q.objectives[0];
			boxes[bindex].corners[4] = current->corners[4];
			current->corners[0] = q.objectives[0];
			add = true;
		}
		volume += fabs(current->corners[0]-current->corners[3]) * fabs(current->corners[1]-current->corners[4]) * 
			fabs(current->corners[2]-q.objectives[2]);
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		current = current->neighbours[0];
	}
	if (add) {
		pushRight();
		bindex++;
	}
	return volume;
}

double closeBoxesLeft(POINT p, POINT q)
{
	double volume = 0;
	double lastx = -1;
	BOX* current = firstB.neighbours[1];
	while (current != &lastB && current->corners[4] < q.objectives[1]) {
		volume += fabs(current->corners[0]-current->corners[3]) * fabs(q.objectives[1]-current->corners[4]) * fabs(current->corners[2]-q.objectives[2]);
		lastx = current->corners[3];
		BOX* prev = current->neighbours[0];
		BOX* next = current->neighbours[1];
		prev->neighbours[1] = next;
		next->neighbours[0] = prev;
		current = current->neighbours[1];
	}
	if (lastx >= 0) {
		boxes[bindex].corners[0] = p.objectives[0];
		boxes[bindex].corners[1] = p.objectives[1];
		boxes[bindex].corners[2] = p.objectives[2];
		boxes[bindex].corners[3] = lastx;
		boxes[bindex].corners[4] = q.objectives[1];
		pushLeft();
		bindex++;
	}
	return volume;
}

double closeAllBoxes(double z)
{
	double volume = 0;
	BOX* current = firstB.neighbours[1];
	while (current != &lastB) {
		volume += fabs(current->corners[0]-current->corners[3]) * fabs(current->corners[1]-current->corners[4]) * fabs(current->corners[2]-z);
		current = current->neighbours[1];
	}
	firstB.neighbours[1] = &lastB;
	lastB.neighbours[0] = &firstB;
	return volume;
}

NODE* lowerZ(POINT p, NODE* start, NODE* end)
{
	NODE* prev = start;
	NODE* current = start->neighboursZ[1];
	while (current != end && current->p.objectives[2] < p.objectives[2]) {
		prev = current;
		current = current->neighboursZ[1];
	}
	if (prev == start) {
		return NULL;
	}
	else {
		return prev;
	}
}

NODE* prevY(NODE* p, NODE* start)
{
	NODE* prev = p->neighboursY[0];
	if (prev == start) {
		return NULL;
	}
	else {
		return prev;
	}
}

NODE* getXRightAbove(POINT p, NODE* start, NODE* end)
{
	NODE* current = end->neighboursY[0];
	while (current != start && current->p.objectives[0] > p.objectives[0] && current->p.objectives[1] >= p.objectives[1]) {
		current = current->neighboursY[0];
	}
	if (current == start || current->p.objectives[1] < p.objectives[1]) {
		return NULL;
	}
	else {
		NODE* q = current;
		current = current->neighboursY[0];
		while (current != start && current->p.objectives[1] >= p.objectives[1]) {
			if (current->p.objectives[0] <= p.objectives[0] && current->p.objectives[0] > q->p.objectives[0]) {
				q = current;
			}
			current = current->neighboursY[0];
		}
		return q;
	}
}

NODE* lowerY(POINT p, NODE* start, NODE* end)
{
	NODE* prev = start;
	NODE* current = start->neighboursY[1];
	while (current != end && current->p.objectives[1] < p.objectives[1]) {
		prev = current;
		current = current->neighboursY[1];
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
	firstB.neighbours[0] = NULL;
	firstB.neighbours[1] = &lastB;
	lastB.neighbours[0] = &firstB;
	lastB.neighbours[1] = NULL;
	NODE* q = lowerY(p,&s1f,&s1e);
	NODE* m = getXRightAbove(p,&s1f,&s1e);
	bindex = 0;
	while (q != NULL && q->p.objectives[0] < p.objectives[0]) {
		if (m == NULL) {
			boxes[bindex].corners[0] = q->p.objectives[0];
			boxes[bindex].corners[1] = p.objectives[1];
			boxes[bindex].corners[2] = p.objectives[2];
			boxes[bindex].corners[3] = 0;
			boxes[bindex].corners[4] = q->p.objectives[1];
			boxes[bindex].corners[5] = p.objectives[2];
			pushLeft();
			bindex++;
			m = q;
		}
		else if (q->p.objectives[0] > m->p.objectives[0]) {
			boxes[bindex].corners[0] = q->p.objectives[0];
			boxes[bindex].corners[1] = p.objectives[1];
			boxes[bindex].corners[2] = p.objectives[2];
			boxes[bindex].corners[3] = m->p.objectives[0];
			boxes[bindex].corners[4] = q->p.objectives[1];
			boxes[bindex].corners[5] = p.objectives[2];
			pushLeft();
			bindex++;
			m = q;
		}
		q = prevY(q,&s1f);
	}
	boxes[bindex].corners[0] = p.objectives[0];
	boxes[bindex].corners[1] = p.objectives[1];
	boxes[bindex].corners[2] = p.objectives[2];
	if (m != NULL) {
		boxes[bindex].corners[3] = m->p.objectives[0];
	}
	else {
		boxes[bindex].corners[3] = 0;
	}
	if (q != NULL) {
		boxes[bindex].corners[4] = q->p.objectives[1];
	}
	else {
		boxes[bindex].corners[4] = 0;
	}
	boxes[bindex].corners[5] = p.objectives[2];
	pushLeft();
	bindex++;
}

void insert(NODE *add, NODE* start, NODE* end) 
{
	NODE* prev = start;
	NODE* next = start->neighboursY[1];
	while (next != end && next->p.objectives[1] < add->p.objectives[1]) {
		prev = next;
		next = next->neighboursY[1];
	}
	prev->neighboursY[1] = add;
	add->neighboursY[0] = prev;
	add->neighboursY[1] = next;
	next->neighboursY[0] = add;
	prev = start;
	next = start->neighboursZ[1];
	while (next != end && next->p.objectives[2] < add->p.objectives[2]) {
		prev = next;
		next = next->neighboursZ[1];
	}
	prev->neighboursZ[1] = add;
	add->neighboursZ[0] = prev;
	add->neighboursZ[1] = next;
	next->neighboursZ[0] = add;
}

void appendY(NODE* add, NODE* end) {
	NODE* prev = end->neighboursY[0];
	prev->neighboursY[1] = add;
	add->neighboursY[0] = prev;
	add->neighboursY[1] = end;
	end->neighboursY[0] = add;
}

void appendZ(NODE* add, NODE* end) {
	NODE* prev = end->neighboursZ[0];
	prev->neighboursZ[1] = add;
	add->neighboursZ[0] = prev;
	add->neighboursZ[1] = end;
	end->neighboursZ[0] = add;
}

void split(double pz) 
{
	s1f.neighboursY[0] = NULL;
	s1f.neighboursY[1] = &s1e;
	s1f.neighboursZ[0] = NULL;
	s1f.neighboursZ[1] = &s1e;
	s1e.neighboursY[0] = &s1f;
	s1e.neighboursY[1] = NULL;
	s1e.neighboursZ[0] = &s1f;
	s1e.neighboursZ[1] = NULL;
	s2f.neighboursY[0] = NULL;
	s2f.neighboursY[1] = &s2e;
	s2f.neighboursZ[0] = NULL;
	s2f.neighboursZ[1] = &s2e;
	s2e.neighboursY[0] = &s2f;
	s2e.neighboursY[1] = NULL;
	s2e.neighboursZ[0] = &s2f;
	s2e.neighboursZ[1] = NULL;
	NODE* current = firstN.neighboursY[1];
	while (current != &lastN) {
		nodes[maxm+current->x].p = current->p;
		if (current->p.objectives[2] >= pz) {
			appendY(&nodes[maxm+current->x],&s1e);
		}
		else {
			appendY(&nodes[maxm+current->x],&s2e);
		}
		current = current->neighboursY[1];
	}
	current = firstN.neighboursZ[1];
	while (current != &lastN) {
		if (current->p.objectives[2] >= pz) {
			appendZ(&nodes[maxm+current->x],&s1e);
		}
		else {
			appendZ(&nodes[maxm+current->x],&s2e);
		}
		current = current->neighboursZ[1];
	}
}

double contribution(POINT p)
{
	if (firstN.neighboursZ[1] == &lastN) {
		return p.objectives[0] * p.objectives[1] * p.objectives[2];
	}
	split(p.objectives[2]);
	initialiseBoxes(p);
	double c = 0;
	NODE* q = lowerZ(p,&s2f,&s2e);
	while (firstB.neighbours[1] != &lastB) {
		if (q == NULL) {
			c += closeAllBoxes(0);
			return c;
		}
		else if (q->p.objectives[0] > p.objectives[0]) {
			if (q->p.objectives[1] > p.objectives[1]) {
				c += closeAllBoxes(q->p.objectives[2]);
				return c;
			}
			else {
				c += closeBoxesLeft(p,q->p);
			}
		}
		else if (q->p.objectives[1] > p.objectives[1]) {
			c += closeBoxesRight(q->p);
		}
		else {
			c += closeBoxCentre(q->p);
		}
		q = prevZ(q,&s2f);
	}
	return c;
}

void delete(NODE* n) 
{
	NODE* prevY = n->neighboursY[0];
	NODE* prevZ = n->neighboursZ[0];
	NODE* nextY = n->neighboursY[1];
	NODE* nextZ = n->neighboursZ[1];
	prevY->neighboursY[1] = nextY;
	nextY->neighboursY[0] = prevY;
	prevZ->neighboursZ[1] = nextZ;
	nextZ->neighboursZ[0] = prevZ;
}

void initialiseNodes(FRONT ps)
{
	for (int i=0; i<ps.nPoints*2; i++) {
		nodes[i].neighboursY[0] = NULL;
		nodes[i].neighboursY[1] = NULL;
		nodes[i].neighboursZ[0] = NULL;
		nodes[i].neighboursZ[1] = NULL;
	}
	firstN.neighboursY[0] = NULL;
	firstN.neighboursY[1] = &lastN;
	firstN.neighboursZ[0] = NULL;
	firstN.neighboursZ[1] = &lastN;
	lastN.neighboursY[0] = &firstN;
	lastN.neighboursY[1] = NULL;
	lastN.neighboursZ[0] = &firstN;
	lastN.neighboursZ[1] = NULL;
}

double hv(FRONT ps)
{
	qsort(ps.points, ps.nPoints, sizeof(POINT), greater);
	initialiseNodes(ps);
	double hypervolume = 0;
	double volume = ps.points[0].objectives[0] * ps.points[0].objectives[1] * ps.points[0].objectives[2];
	nodes[0].p = ps.points[0];
	nodes[0].x = 0;
	insert(&nodes[0],&firstN,&lastN);
	for (int i=1; i<ps.nPoints; i++) {
		hypervolume += volume * fabs(ps.points[i].objectives[3] - ps.points[i-1].objectives[3]);
		NODE* current = firstN.neighboursZ[1];
		volume += contribution(ps.points[i]);
		while (current != &lastN && current->p.objectives[2] < ps.points[i].objectives[2]) {
			if (current->p.objectives[0] <= ps.points[i].objectives[0] && current->p.objectives[1] <= ps.points[i].objectives[1]) {
				delete(current);
			}
			current = current->neighboursZ[1];
		}
		nodes[i].p = ps.points[i];
		nodes[i].x = i;
		insert(&nodes[i],&firstN,&lastN);
	}
	hypervolume += volume * ps.points[ps.nPoints-1].objectives[3];
	return hypervolume;
}

void allocate_memory(int maxm_, int maxn_)
{
	maxm = maxm_;
	nodes = malloc(sizeof(NODE) * maxm * 3);
	boxes = malloc(sizeof(BOX) * maxm * 2);
}

void free_memory()
{
	free(nodes);
	free(boxes);
}
