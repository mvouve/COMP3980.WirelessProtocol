
/*****************************************************************************************************
**	SOURCE FILE:	Physical.h *		Header file for Physical.cpp
**
**	Purpose:	Contains function definitions used in Physical.cpp and
**				any other global declarations.
**
**	DATE: 		September 28, 2014
**
**	DESIGNER: 	Filip Gutica A00781910
**
**	PROGRAMMER: Filip Gutica A00781910
**
*********************************************************************************************************/
#ifndef physical_h
#define physical_h

/* Global constants, Size of a line read and number of Chars the 
   COM port will read
*/
#define LINE_SIZE 128
#define CHARS_TO_READ 1
#define CONTROL_SIZE 2
#define DATA_SIZE 1018
#define CRC_SIZE 4

#include <Windows.h>	// Windows API
#include <string.h>		// String functions
#include <string>		// C++ string class
#include <stdio.h>		// standard c io
#include <iostream>		// io stream
#include "Menu.h"		// Menu resources
#include "crc.h"

using std::string;

/* Structure containing every veriable needed by the COM port */
struct PortInfo {
	HANDLE hComm, hRead, hWrite;
	LPCSTR lpszCommName;
	BOOLEAN connected;
	BOOLEAN transmitting;
	DWORD dwWritten, dwRead;
	COMMCONFIG cc;
	DCB dcb;
	OVERLAPPED overlapped;
	char * strReceive;
};

struct GrapefruitPacket {
	char status;
	char sync;
	char data[DATA_SIZE];
	char crc[CRC_SIZE];
};

/* Function prototypes used in Physical.cpp */
void Connect();
void SetPortSettings(char *, HWND);
DWORD WINAPI ReadPort(LPVOID);
char * BuildBuffer(char *);
GrapefruitPacket BuildPacket(char *);
void PrintCommState(DCB);
void setConnected(BOOL connect);
BOOL isConnected();
void closePort();
void PrintCommState(DCB);
void ReceiveMode();
bool WaitForPacket(char* packet);

#endif