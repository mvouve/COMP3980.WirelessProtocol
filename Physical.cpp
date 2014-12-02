/*----------------------------------------------------------------------------------------------------
--	SOURCE FILE:	Physical.cpp -		Physical layer of the Dumb terminal program. 
--	 									
--	PROGRAM:	GrapefruitProtocol
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
GrapefruitPacket GlobalPacket;
HANDLE hThrd;		// Handle to the read thread

/*------------------------------------------------------------------------------
--	FUNCTION: Connect()
--
--	PURPOSE:	Connects to the COM port. Checks which com port your cable is
--				curently pluged into. Starts reading thread.
--
/*-----------------------------------------------------------------------------*/
void Connect()
{
	DWORD threadID;		// ID for the read thread
	
	portInfo.strReceive = new char[LINE_SIZE]; // Buffer for received characters
	memset(portInfo.strReceive, 0, sizeof(portInfo.strReceive)); //Initialize the buffer to null

	crcInit();

	// Starting the read thread
	hThrd = CreateThread(NULL, 0, ProtocolThread, (LPVOID)2, 0, &threadID);

	if (!hThrd)
	{
		// Close the thread if there's an error
		CloseHandle(hThrd);
		portInfo.transmitting = FALSE;
	}
	else
		// Continue transmitting
		portInfo.transmitting = FALSE;

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

void SetPortSettings(char *s, HWND hwnd)
{
	portInfo.lpszCommName = TEXT(s);
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


	portInfo.cc.dwSize = sizeof(COMMCONFIG);
	portInfo.cc.wVersion = 0x100;
	GetCommConfig(portInfo.hComm, &portInfo.cc, &portInfo.cc.dwSize);
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
--	FUNCTION: ProtocolThread(LPVOID)
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
DWORD WINAPI ProtocolThread(LPVOID n)
{
	

	HANDLE hThrd;
	DWORD threadID;
	hThrd = CreateThread(NULL, 0, ReadThread, (LPVOID)2, 0, &threadID);
	
	while (true)
	{
		if ( isConnected() && !isBufferEmpty() )
		{
			if (!isTransmit()  )
			{
				MessageBox(NULL, "Transmitting", "yay!", MB_OK);
				//Enter the write mode
				setTransmitting(true);

				//Enter write mode
				WriteMode();
				setTransmitting(false);
			}
		}
	}
	ExitThread(0);
	return 0L;
}

DWORD WINAPI ReadThread(LPVOID n)
{
	Statistics *s = Statistics::GetInstance();
	char* control;

	while (true)
	{
		if (!isTransmit())
		{
			control = ReadControlChar();

			if (control[0] == ENQ)
			{		
				s->IncrementENQS();
				PrintStats();
				InvalidateStats();
				//ReadMode
				ReceiveMode();
			}
		}
	}

	ExitThread(0);
	return 0L;
}


char * ReadPort(void)
{
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);

		// Wait for a comm event
		WaitCommEvent(portInfo.hComm, &dwEvtMask,NULL);
		
		if (dwEvtMask == EV_RXCHAR)
		{
			//MessageBox(NULL, "Getting packets and stuff", "", MB_OK);
			
			// Read in characters if character is found by waitcommevent
			if (ReadFile(portInfo.hComm, portInfo.strReceive, sizeof(GrapefruitPacket), &portInfo.dwRead,&portInfo.overlapped)) 
			{
				
				//GetCharsFromPort(portInfo.strReceive);
			}
			else if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwRead, TRUE);
				//GetCharsFromPort(portInfo.strReceive);
			}
		}

		dwEvtMask = NULL;
		portInfo.dwRead = 0;

	return portInfo.strReceive;
}

char * ReadControlChar(void)
{
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);
	char  ccontrol[1];

		// Wait for a comm event
		if(WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
		{
			// Read in characters if character is found by waitcommevent
			if (!ReadFile(portInfo.hComm, ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped)) 
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwRead, TRUE);
					//GetCharsFromPort(ccontrol);
				}
			}
		}
		else
		{
			GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwRead, TRUE);
			// Read in characters if character is found by waitcommevent
			if (!ReadFile(portInfo.hComm, ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped)) 
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwRead, TRUE);
					//GetCharsFromPort(ccontrol);
				}
			}
		}
		dwEvtMask = NULL;
		portInfo.dwRead = 0;

	return ccontrol;
}

/*------------------------------------------------------------------------------
-- FUNCTION: WritePort( void * message )
--
-- PURPOSE:  This is a helper function for writing Packets/Control Characters
--           to the port.
--
--
--
-- DESIGNER: Marc Vouve A00848381
--
--
-- PROGRAMMER: Marc Vouve A00848381
--
--
------------------------------------------------------------------------------*/
BOOL WritePort( const void * message )
{
	char s[256] = "";
	return WriteFile(portInfo.hComm,  message,
		sizeof(message), &portInfo.dwWritten, &portInfo.overlapped);
}


BOOL WriteControlChar( char control )
{
	if (!WriteFile(portInfo.hComm, &control, 1, &portInfo.dwWritten, &portInfo.overlapped))
	{
		if (GetLastError() == 0x3e5)
		{
			GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &portInfo.dwWritten, TRUE);
		}

		portInfo.dwWritten = 0;
		return TRUE;
		
	}
	portInfo.dwWritten = 0;

	return TRUE;
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
	char *packet = "";
	// The last syn received.
	char syn = SYN1;	
	char c = ACK;
	Statistics *stats = Statistics::GetInstance();

	//Send ACK back to them for packet

	WriteControlChar(c);
	stats->IncrementACKSSent();
	PrintStats();
	InvalidateStats();

	for (int i = 0; i < MAXSENT; i++)
	{
		if (!WaitForPacket(packet))
		{
			//TO1 this will be replaced
			Sleep(1000);
			return;
		}

		// validate packet, assuming valid for now.
		else if (true)
		{
			
			// checks if the second packet character is a syn bit 
			if (packet[1] != syn || ( i == 0 && ( packet[1] == SYN1 || packet[1]
				== SYN2 ) ) )
			{
				//MessageBox(NULL,"GOT A PACKEET", "packet", MB_OK);
				(packet[1] == SYN1 ? syn = SYN2 : syn = SYN1); // flip SYN.
	
				GetCharsFromPort(packet);
			}
			// SEND ACK on packet
			WriteControlChar(c);
			stats->IncrementACKSSent();
			PrintStats();
			InvalidateStats();

			if (packet[1] == EOT )
			{
				return;
			}
		}
		else
		{
			//Send NAK
			char c = NAK;
			WriteControlChar(c);
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
	char temp[2];
	char packet[PACKET_SIZE];
	int miss = 0;
	char *controlC;
	Statistics *stats = Statistics::GetInstance();

	//Wait for ACK		
	MessageBox(NULL,"reading control char", "char", MB_OK);
	controlC = ReceiveControlChar(10000);

	//Recieved an ACK
	if ( controlC[0] == ACK )
	{
		MessageBox(NULL,"got a control char", "char", MB_OK);
		char s[256];
		sprintf(s, "CONTROL CHAR: %x", controlC);
		OutputDebugString(s);
		stats->IncrementACKSReceived();
		PrintStats();
		InvalidateStats();
			
		for ( int i = 0; i < MAXSENT; i++)
		{
			while ( miss < MAXMISS )
			{
				//Send packet
				//MessageBox(NULL, "sending packet", "packet status", MB_OK);

				packet[0] = GlobalPacket.status;
				packet[1] = GlobalPacket.sync;

				for (int i = 0 ; i < DATA_SIZE; i++)
				{
					packet[2 + i] = GlobalPacket.data[i];
				}
				for (int i = 0; i < CRC_SIZE; i++)
				{
					packet[DATA_SIZE + i] = GlobalPacket.crc[i];
				}

				WritePort(&packet);

				//Wait for ACK
				controlC = ReceiveControlChar(1000);

				if (temp[0] == ACK)
				{						
					stats->IncrementACKSReceived();
					PrintStats();
					InvalidateStats();
					//ACK Received: update buffer and packetize
					//Check for EOT
					if ( GlobalPacket.status == EOT )
					{
						
						//*******************CHANGE LATER*********************
						setBufferStatus(false);
						setTransmitting(false);
						return;
					}						
					miss = 0;
				}
			}

		}
	}
	//NAK Received: resend data and increment miss
	else if ( controlC[0] == NAK )
	{
		stats->IncrementNAKS();
		PrintStats();
		InvalidateStats();
		miss++;
	}
	//No NAK or ACK: assumed failed
	else
	{
		stats->IncrementPacketsLost();
		PrintStats();							
		InvalidateStats();
		miss++;
	}

	/*stats->IncrementPacketsLost();
	InvalidateStats();
	PrintStats();*/

	//No ACK; end transmission
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
bool WaitForPacket(char* packet)
{
	packet = ReadPort();
	//MessageBox(NULL, "got packet", "asdf", MB_OK);
	return true;
}

/*------------------------------------------------------------------------------
--	FUNCTION: BuildBuffer(char *)
--
--	PURPOSE: Sends characters to the COM port
--
--	PARAMETERS:
--		strToSend		- chars to be added to the buffer
--
-- DESIGNER: Filip Gutica A00781910
--
--
-- PROGRAMMER: Filip Gutica A00781910
--
/*-----------------------------------------------------------------------------*/
char * BuildBuffer(char * strToSend)
{
	memset(&c[0], 0, sizeof(c));
	GlobalPacket.status = '\0';
	GlobalPacket.sync = '\0';

	memset(GlobalPacket.data, 0, sizeof(GlobalPacket.data));

	strcat(c, strToSend);

	//OutputDebugString(c);
	return strToSend;

	
	
}

/*------------------------------------------------------------------------------
--	FUNCTION: BuildPacket(char *)
--
--	PURPOSE: Sends characters to the COM port
--
--	PARAMETERS:
--		strToSend		- chars to be added to the buffer
--
-- DESIGNER: Filip Gutica A00781910
--
--
-- PROGRAMMER: Filip Gutica A00781910
--
/*-----------------------------------------------------------------------------*/
GrapefruitPacket BuildPacket()
{
	
	if (GlobalPacket.sync == SYN1)	GlobalPacket.sync = SYN2;
	else							GlobalPacket.sync = SYN1;
	
	int count = 0;
	for(count = 0; count < DATA_SIZE && count < sizeof(c)/sizeof(char) ; count++)
	{
		
		GlobalPacket.data[count] = c[count];
		
	}

	if (sizeof(c)/sizeof(char) < DATA_SIZE)
	{
		GlobalPacket.status = ETB;
		GlobalPacket.data[count] = ETX;
		count++;
		for(int i = count; i < DATA_SIZE ; i++)
		{
			GlobalPacket.data[i] = '\0';
		}
		
	} 
	else 
	{
		GlobalPacket.status= EOT;
	}

	
	int crcBits = crcFast((unsigned char*)GlobalPacket.data, DATA_SIZE);
	unsigned char *helping = (unsigned char*)&crcBits;
	GlobalPacket.crc[0] = helping[3]; GlobalPacket.crc[1] = helping[2];  GlobalPacket.crc[2] = helping[1]; GlobalPacket.crc[3] = helping[0]; 

	//OutputDebugStringA( "" + GlobalPacket.status);
	//OutputDebugStringA("" + GlobalPacket.sync);
	
	//OutputDebugString(GlobalPacket.data);

	
	//OutputDebugStringA(GlobalPacket.crc);


		

	return GlobalPacket;
}

bool checkPacketCrc(GrapefruitPacket gfp)
{
	char data[1020];
	data[0] = gfp.status;
	data[1] = gfp.sync;
	for(int i = 0; i < DATA_SIZE; i++)
	{
		data[i+2] = gfp.data[i];
	}
	int crcCheck = crcFast((unsigned char*)data, 1020);
	unsigned char *crc = (unsigned char*)&crcCheck;
	if(crc[3] == gfp.crc[0] && crc[2] == gfp.crc[1] && crc[1] == gfp.crc[2] && crc[0] == gfp.crc[3])
	{
		return true;
	}
	
	return false;
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
--	FUNCTION: SetConnected(BOOL connect)
--
--	PURPOSE: Sets the connection status true or false.
--
--	PARAMETERS:
--		connect		- Boolean to tell if were connected to the COM port or not.
--
/*-----------------------------------------------------------------------------*/
void setTransmitting(BOOL transmit)
{
	Statistics *s = Statistics::GetInstance();
	if (transmit == true )
	{
		s->IncrementENQS();
		PrintStats();
		InvalidateStats();
		//Send ENQ to other side
		char cChar = ENQ;
		WriteControlChar(cChar);
	}
	portInfo.transmitting = transmit;
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
BOOL isTransmit()
{
	return portInfo.transmitting;
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


char * ReceiveControlChar(double timeout)
{
	DWORD numCharsRead;
	char *temp ="";
	DWORD commEvent;

	LPCOMMTIMEOUTS commTimeouts = new COMMTIMEOUTS();

	commTimeouts->ReadTotalTimeoutMultiplier = 0;
	commTimeouts->ReadTotalTimeoutConstant = timeout;

	if (!SetCommTimeouts(portInfo.hComm, commTimeouts))
	{
		MessageBox(NULL, "Unable to set timeouts.", "Timeouts", MB_OK);
		return "";
	}
	
	if (!SetCommMask(portInfo.hComm, EV_RXCHAR))
	{
		MessageBox(NULL, "Error setting comm mask", "Comm Mask Error", MB_OK);
		return "";
	}


	//clear port
	PurgeComm(portInfo.hComm, PURGE_RXCLEAR);	
	PurgeComm(portInfo.hComm, PURGE_TXCLEAR);
	PurgeComm(portInfo.hComm, PURGE_TXABORT);	
	PurgeComm(portInfo.hComm, PURGE_RXABORT);

	//Read port
	if (WaitCommEvent(portInfo.hComm, &commEvent, &portInfo.overlapped))
	{
		if (!ReadFile(portInfo.hComm, &temp, 1, &portInfo.dwRead, &portInfo.overlapped))
		{
			GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &numCharsRead, TRUE);
		}

		OutputDebugString(temp);
	}
	else
	{
		DWORD err = GetLastError();
		Sleep(100);
 		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &numCharsRead, TRUE);

			if (!ReadFile(portInfo.hComm, &temp, 1, &portInfo.dwRead, &portInfo.overlapped))
			{
				GetOverlappedResult(portInfo.hComm, &portInfo.overlapped, &numCharsRead, TRUE);
			}

			OutputDebugString(temp);
		}
	}
	
	portInfo.dwRead = 0;
	return temp;
}

void setBufferStatus(BOOL status)
{
	portInfo.empty = status;
}

BOOL isBufferEmpty()
{
	if (portInfo.empty = true)
	{
		return true;
	}
		return false;
}
