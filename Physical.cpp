/*----------------------------------------------------------------------------------------------------
--	SOURCE FILE:	Physical.cpp -		Physical layer of the Dumb terminal program. 
--	 									
--	PROGRAM:	DumbTerminal
--
--	DATE: 		September 28, 2014
--
--	DESIGNER: 	Filip Gutica A00781910
--
--	PROGRAMMER: Filip Gutica A00781910
--
--------------------------------------------------------------------------------------------------------*/


#define STRICT
#define _CRT_SECURE_NO_WARNINGS

#include "Physical.h"
#include "Application.h"
#include "Protocol.h"
#include <stdio.h>

PortInfo portInfo;
HWND curHwnd;
DCB dcb = { 0 };
char *c = new char[DATA_SIZE];
GrapefruitPacket packet;

/*------------------------------------------------------------------------------
--	FUNCTION: Connect()
--
--	PURPOSE:	Connects to the COM port. Checks which com port your cable is
--				curently pluged into. Starts reading thread.
--
/*-----------------------------------------------------------------------------*/
void Connect()
{
	HANDLE hThrd;		// Handle to the read thread
	DWORD threadID;		// ID for the read thread
	
	portInfo.strReceive = new char[LINE_SIZE]; // Buffer for received characters
	memset(portInfo.strReceive, 0, sizeof(portInfo.strReceive)); //Initialize the buffer to null

	if ((portInfo.hComm = CreateFile(portInfo.lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
		== INVALID_HANDLE_VALUE)
	{
		// Error opening the COM port
		MessageBox(NULL, "Error opening COM port: \nRemember to set port settings first.", "", MB_OK);
		portInfo.connected = FALSE;
		return;
	}
	else
	{
		// Succesfully connected
		OutputDebugString("Connected.");
		portInfo.connected = TRUE;
	}
	// Starting the read thread
	hThrd = CreateThread(NULL, 0, ReadPort, (LPVOID)2, 0, &threadID);

	if (!hThrd)
	{
		// Close the thread if there's an error
		CloseHandle(hThrd);
		portInfo.transmitting = FALSE;
	}
	else
		// Continue transmitting
		portInfo.transmitting = TRUE;
}

/*------------------------------------------------------------------------------
--	FUNCTION: SetPortSettings(char *, HWND)
--
--	PURPOSE: Set the port settings as configured by the user.
--
--		PARAMETERS:
**		s			-Name of the port the user has selected.
**		hwnd		-Handle to the program's window.
**
**
/*-----------------------------------------------------------------------------*/

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! NOTE:
!!	THIS FUNCTION ONLY WORKS AFTER THE CONNECTION IS CREATED
!!  THIS NEEDS TO BE FIXED.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
void SetPortSettings(char *s, HWND hwnd)
{
	portInfo.cc.dwSize = sizeof(COMMCONFIG);
	portInfo.cc.wVersion = 0x100;
	GetCommConfig(portInfo.hComm, &portInfo.cc, &portInfo.cc.dwSize);
	portInfo.lpszCommName = TEXT(s);
	if (!CommConfigDialog(portInfo.lpszCommName, hwnd, &portInfo.cc))
	{
		MessageBox(NULL, "You did not connect to any port.", 
							"Not Connected to Selected Port", MB_OK);
		return;
	}
	else
	{
		SetCommConfig(portInfo.hComm, &portInfo.cc, portInfo.cc.dwSize);
		GetCommState(portInfo.hComm, &dcb);
		PrintCommState(dcb);
	}

}

void PrintCommState(DCB dcb)
{
	char s[256] = "";
	//  Print some of the DCB structure values
	sprintf_s(s, "\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n",
		dcb.BaudRate,
		dcb.ByteSize,
		dcb.Parity,
		dcb.StopBits);

	OutputDebugString(s);
}

/*------------------------------------------------------------------------------
--	FUNCTION: ReadPort(LPVOID)
--
--	PURPOSE: Thread function continuously reads from the COM port
--
--		PARAMETERS:
--		n		- Defualt LPVOID parameter for thread function.
--
--	RETURN:
--		Always 0
--
--	NOTES:	Used function found in the MSDN Serial Communications tutoial,
--			Modified for this assignment. 
--
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ReadPort(LPVOID n)
{
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);
	while (portInfo.transmitting == TRUE)
	{
		// Wait for a comm event
		WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped);
		
		if (dwEvtMask == EV_RXCHAR)
		{
			// Read in characters if character is found by waitcommevent
			ReadFile(portInfo.hComm, portInfo.strReceive, 
					CHARS_TO_READ, &portInfo.dwRead, 
					&portInfo.overlapped);
			GetCharsFromPort(portInfo.strReceive);
		}
		dwEvtMask = NULL;
	}
	ExitThread(0);
	return 0L;
}

/*------------------------------------------------------------------------------
-- FUNCTION: ReceieveMode()
--
-- PURPOSE:  This function reaads the comm port waiting for a signal.
--           it will try to read MAXSET times, and if it fails it will fall back
--			 to the invoking function
--
-- DESIGNER:
--
--
-- PROGRAMMER: Marc Vouve A00848381
--
--
------------------------------------------------------------------------------*/
void ReceiveMode()
{
	GrapefruitPacket packet;
	// The last syn reseived.
	char syn = SYN1;

	for (int i = 0; i > MAXSENT; i++)
	{
		if (!WaitForPacket(&packet))
		{
			//TO1
			return;
		}
		// validate packet
		else if (true)
		{
			// checks if the second packet character is a syn bit 
			if (packet.sync == syn)
			{
				(syn == SYN1 ? syn = SYN2 : syn = SYN1); // flip SYN.
			}
			// SEND ACK 
			if (packet.type != EOT)
			{
				return;
			}
		}
		else
		{
			//Send NAK
		}
	}
}

/*------------------------------------------------------------------------------
-- FUNCTION: bool WaitForPacket(char* packet)
--
-- PARAMS:	char* packet: A buffer to read in the recieved packet onto.
--
-- PURPOSE:  This function holds the thread for a predefined wait time, reading
--			 for an incoming packet. when a packet is received it writes the
--			 data to the packet char*.
--
--
--
-- DESIGNER: Marc Vouve A00848381
--
--
-- PROGRAMMER: Marc Vouve A00848381
--
-- RETURN: True on read false on no incoming packets
------------------------------------------------------------------------------*/
bool WaitForPacket(GrapefruitPacket* packet)
{
	return true;
}

/*------------------------------------------------------------------------------
--	FUNCTION: WritePort(char *)
--
--	PURPOSE: Sends characters to the COM port
--
--	PARAMETERS:
--		c		- character to be written to the COM port
--
/*-----------------------------------------------------------------------------*/
char * BuildBuffer(char * strToSend)
{
	strcat(c, strToSend);

	OutputDebugString(c);
	return c;

	// Write the character sent to this function to the port.

	/*WriteFile(portInfo.hComm, strToSend,
		GetCommConfig(portInfo.hComm, &portInfo.cc, &portInfo.cc.dwSize),
		&portInfo.dwWritten, &portInfo.overlapped);*/
	
}

GrapefruitPacket BuildPacket()
{
	int count = 0;
	while (count < DATA_SIZE)
	{
		strcat(packet.data, &c[count]);
		count++;
	}

	if (count < DATA_SIZE)
	{
		packet.control[0] = EOT;
		while (count < DATA_SIZE)
		{
			//Add padding characters 
			/*if ()
			{
				packet.data[count] = ETX;
				strcat(packet.data, '\0');
			}*/
			count++;
		}
	}

	return packet;
}

/*------------------------------------------------------------------------------
--	FUNCTION: SetConnected(BOOL connect)
--
--	PURPOSE: Sets the connection status true or false.
--
--	PARAMETERS:
--		connect		- Boolean to tell if were connected to the COM port or not.
--
/*-----------------------------------------------------------------------------*/
void setConnected(BOOL connect)
{
	portInfo.connected = connect;
}

/*------------------------------------------------------------------------------
--	FUNCTION: isConnected()
--
--	PURPOSE: Returns the connection status true or false.
--
--	RETURN:
--		Always True if currently connected, false if not connected
--
/*-----------------------------------------------------------------------------*/
BOOL isConnected()
{
	return portInfo.connected;
}

/*------------------------------------------------------------------------------
--	FUNCTION: closePort()
--
--	PURPOSE: Close the handle to the port
--
/*-----------------------------------------------------------------------------*/
void closePort()
{
	CloseHandle(portInfo.hComm);
}





