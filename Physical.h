
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
#define PACKET_SIZE 1024
#define CRC_SIZE 4

#include <Windows.h>	// Windows API
#include <string.h>		// String functions
#include <string>		// C++ string class
#include <stdio.h>		// standard c io
#include <iostream>		// io stream
#include "Menu.h"		// Menu resources
#include "crc.h"
#include "Application.h"
#include "Protocol.h"
#include <stdio.h>

using std::string;

enum MODE
{
	WAITING = 0,
	READ = 1,
	WRITE = 2
};

/* Structure containing every variable needed by the COM port */
struct PortInfo {
	HANDLE hComm, hRead, hWrite;
	LPCSTR lpszCommName;
	BOOLEAN connected;
	BOOLEAN transmitting;
	BOOLEAN receiving;
	DWORD dwWritten, dwRead;
	BOOLEAN empty;
	COMMCONFIG cc;
	DCB dcb;
	OVERLAPPED overlapped;
	char * strReceive;
	MODE mode;
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
DWORD WINAPI ProtocolThread(LPVOID);
DWORD WINAPI ReadThread(LPVOID n);
char * ReadPort(void);
BOOL   WritePort(const void *);
char * BuildBuffer(char *);
GrapefruitPacket BuildPacket();
void PrintCommState(DCB);
void setConnected(BOOL connect);
BOOL isConnected();
void setTransmitting(BOOL transmit);
BOOL isTransmit();
void setReceiving(BOOL transmit);
BOOL isReceive();
void closePort();
void PrintCommState(DCB);
void WriteMode();
void ReceiveMode();
bool WaitForPacket(char * packet);
BOOL WriteControlChar( char );
BOOL ReadENQ();
char * ReceiveControlChar(double);
void setBufferStatus(BOOL);
BOOL isBufferEmpty();
void setMode(MODE m);
MODE getMode();

#endif