/*******************************************************************************
** Source File : Session.cpp -- Grapefruit protocol session data; timeouts.
**
** Program : Grapefruit Protocol Implementation
**
** Functions :
**			
**
**
**	DATE: 		December 2nd, 2014
**
**	REVISIONS:	N/A
**
**	DESIGNER: 	Alex Dellow 
**
**
**	PROGRAMMER: Rhea Lauzon
**				Alex Dellow
**
** Notes :
**
***********************************************************************/
#define STRICT
#define _CRT_SECURE_NO_WARNINGS	
#include "Session.h"

#pragma warning (disable: 4096)

void CalculateTimeouts(Timeouts *timeouts, int bitRate)
{
	//Calculate each timeout in milliseconds so that they are compatible with
	//WaitForSingleObject

	//TO3 is 1200/BPS in milliseconds. We round it to make it an int
	timeouts->TO3 = ( (double)TO3BASE / ( bitRate / BYTESIZE ) ) * TOMS;

	//TO2 is 5/BPS in milliseconds. We round it up to make it an int
	timeouts->TO2 = ((double)TO2BASE / (bitRate / BYTESIZE)) * TOMS;

	//TO1 is TO3 * MAX_MISS (3)
	timeouts->TO1 = timeouts->TO3 * MAXMISS;

	//Calculate min and max reset values in bits
	timeouts->resetMin = ceil(((double)TO4MIN / (bitRate * BYTESIZE)) * BYTESIZE / TOMS);
	timeouts->resetMax = ceil(((double)TO4MAX / (bitRate * BYTESIZE)) * BYTESIZE / TOMS);

	char s[100];
	char d[100];
	char x[100];
	sprintf(s, " TO1: %f", timeouts->TO1);
	sprintf(d, "TO2: %f", timeouts->TO2);
	sprintf(x, "TO3: %f", timeouts->TO3);
	OutputDebugString(s);
	OutputDebugString(d);
	OutputDebugString(x);

}