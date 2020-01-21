// This software is Copyright (C) 2010 Lyndon While, Lucas Bradstreet. 

// This program is free software (software libre). You can redistribute it and/or modify it under 
// the terms of the GNU General Public License as published by the Free Software Foundation; 
// either version 2 of the License, or (at your option) any later version. 

// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU General Public License for more details. 

// Source: http://www.wfg.csse.uwa.edu.au/hypervolume/
#ifndef _WFG_H_
#define _WFG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double OBJECTIVE;

typedef struct
{
	OBJECTIVE *objectives;
} POINT;

typedef struct
{
	int nPoints;
	int n;
	POINT *points;
} FRONT;

typedef struct
{
	int nFronts;
	FRONT *fronts;
} FILECONTENTS;


double hv(FRONT ps);
void allocate_memory(int maxm_, int maxn_);
void free_memory(void);
inline double wrapped_hv(FRONT ps) {
	double tmp = 0.;
	allocate_memory(ps.nPoints, ps.n);
	tmp = hv(ps);
	free_memory();
	return tmp;
}

#endif
