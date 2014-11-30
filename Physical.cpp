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
char c[1018];
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
	//memset(c, 0, sizeof(c));

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
	while (portInfo.transmitting)
	{
		// Wait for a comm event
		WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped);
		
		if (dwEvtMask == EV_RXCHAR)
		{
			// Read in characters if character is found by waitcommevent
			if (ReadFile(portInfo.hComm, portInfo.strReceive,CHARS_TO_READ, &portInfo.dwRead,&portInfo.overlapped))
				GetCharsFromPort(portInfo.strReceive);
			else if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwRead, TRUE);
				GetCharsFromPort(portInfo.strReceive);
			}
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
		// validate packet, assuming valid for now.
		else if (true)
		{
			// checks if the second packet character is a syn bit 
			if (packet.sync == syn)
			{
				(syn == SYN1 ? syn = SYN2 : syn = SYN1); // flip SYN.
			}
			// SEND ACK 
			if (packet.status != EOT)
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
-- FUNCTION: WriteMode()
--
-- PURPOSE:  This functions writes the the packet with crc to the port
--
-- DESIGNER: Rhea Lauzon
--
--
-- PROGRAMMER: Rhea Lauzon A00848381
--
--
------------------------------------------------------------------------------*/
void WriteMode()
{
	//Wait for ACK

	int miss = 0;

	for ( int i = 0; i < MAXSENT; i++)
	{
		while ( miss < MAXMISS )
		{
			//Send packet

			//Wait for ACK
							
			//ACK Received: update buffer and packetize
			/* if (  )
			{
				//Check for EOT
				if ( packetData[0].equals( EOT ) == 0 )
				{
					return;
				}
				
				miss = 0;
				break;
			}
			*/

			//NAK Received: resend data and increment miss
			/*else if ( WaitForSingleObject returned is NAK )
			{
				miss++;
				break;
			}*/

			//No ACK or NAK; resend and increment
			//else
			//{
				miss++;
				break;
			//}
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

	memset(&c[0], 0, sizeof(c));
	packet.status = '\0';
	packet.sync = '\0';

	memset(&packet.data[0], 0, sizeof(packet.data));

	strcat(c, strToSend);

	//OutputDebugString(c);
	return strToSend;

	
	
}

GrapefruitPacket BuildPacket()
{
	if (packet.sync == SYN1)	packet.sync = SYN2;
	else						packet.sync = SYN1;
	
	int count = 0;
	for(count = 0; count < DATA_SIZE && count < sizeof(c)/sizeof(char) ; count++)
	{
		
		packet.data[count] = c[count];
		
	}

	if (sizeof(c)/sizeof(char) < DATA_SIZE)
	{
		packet.status = ETB;
		packet.data[count] = ETX;
		count++;
		for(int i = count; i < DATA_SIZE ; i++)
		{
			packet.data[i] = '\0';
		}
		
	} 
	else 
	{
		packet.status= EOT;
	}

	
	int crcBits = crcFast((unsigned char*)packet.data, DATA_SIZE);
	int *helping = &crcBits;

	strcpy(packet.crc, (char*) helping);

	OutputDebugStringA( "" + packet.status);
	OutputDebugStringA("" + packet.sync);
	
	OutputDebugString(packet.data);

	
	OutputDebugStringA(packet.crc);

	// Write the character sent to this function to the port.

	
	WriteFile(portInfo.hComm,  &packet,
		GetCommConfig(portInfo.hComm, &portInfo.cc, &portInfo.cc.dwSize),
		NULL, &portInfo.overlapped);

	

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





