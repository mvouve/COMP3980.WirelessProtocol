#ifndef session_h
#define session_h

#define TO3BASE 1200
#define TO2BASE 5
#define TOMS 1000
#define TO4MIN 1
#define TO4MAX 4
#define BYTESIZE 8

#include "Physical.h"

typedef struct Timeouts
{
	double TO1;
	double TO2;
	double TO3;
	double resetMin;
	double resetMax;

};

void CalculateTimeouts(Timeouts *, int);

#endif