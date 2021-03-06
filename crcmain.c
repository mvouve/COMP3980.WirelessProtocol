/**********************************************************************
 *
 * Filename:    main.c
 * 
 * Description: A simple test program for the CRC implementations.
 *
 * Notes:       To test a different CRC standard, modify crc.h.
 *
 * 
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "crc.c"


void
main(void)
{
	unsigned char  test[] = "123456789";
	printf("Size of short is %i\n", sizeof(short));
	printf("Size of int is %i\n", sizeof(int));
	printf("Size of long is %i\n", sizeof(long));

	/*
	 * Print the check value for the selected CRC algorithm.
	 */
	printf("The check value for the %s standard is 0x%X\n", CRC_NAME, CHECK_VALUE);
	
	/*
	 * Compute the CRC of the test message, slowly.
	 */
	printf("The crcSlow() of \"123456789\" is 0x%X\n", crcSlow(test, strlen(test)));
	
	/*
	 * Compute the CRC of the test message, more efficiently.
	 */
	crcInit();
	printf("The crcFast() of \"123456789\" is 0x%X\n", crcFast(test, strlen(test)));
	int testing = crcFast(test, strlen(test));
	int *helping = &testing;
	printf("Test string with crc %s%c%c%c%c\n", test, helping[0],helping[1], helping[2],helping[3]);
	
	//How to add the characters to the packet
	/*packet[1020] = helping[0];
	packet[1021] = helping[1];
	packet[1022] = helping[2];
	packet[1023] = helping[3];*/

}   /* main() */