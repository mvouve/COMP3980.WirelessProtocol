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
#include "Session.h"

// MAXIMUM VISABLITIY
PortInfo portInfo;
HWND curHwnd;
DCB dcb = { 0 };
char c[1018];
GrapefruitPacket GlobalPacket;
HANDLE hThrd;		// Handle to the read thread
std::deque<GrapefruitPacket> packetQueue;
Timeouts t;

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
    
    CalculateTimeouts(&t, dcb.BaudRate);
	
    
	portInfo.strReceive = new char[LINE_SIZE]; // Buffer for received characters
	memset(portInfo.strReceive, 0, sizeof(portInfo.strReceive)); //Initialize the buffer to null

	crcInit();

	// Starting the read thread
	hThrd = CreateThread(NULL, 0, ProtocolThread, (LPVOID)2, 0, &threadID);

	if (!hThrd)
	{
		// Close the thread if there's an error
		CloseHandle(hThrd);
		portInfo.mode = WAITING;
	}
	else
	{
		// Continue transmitting
		portInfo.mode = WAITING;
		portInfo.empty = true;
	}

}

/*------------------------------------------------------------------------------
--	FUNCTION: SetPortSettings(char *, HWND)
--
--	PURPOSE: Set the port settings as configured by the user.
--
--		PARAMETERS:
--		s			-Name of the port the user has selected.
--		hwnd		-Handle to the program's window.
--
-- PROGRAMMER: Filip Gutica
--
-- DESIGNER:   Filip Gutica
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

/*------------------------------------------------------------------------------
--	FUNCTION: PrintCommState(DCB)
--
--	PURPOSE: Function to print information on the comm port
--
--		PARAMETERS:
--		dcb		- Defualt LPVOID parameter for thread function.
--
--	RETURN:
--		void

-- PROGRAMMER: Filip Gutica
--
-- DESIGNER:   Filip Gutica
--
--	NOTES:	
--
/*-----------------------------------------------------------------------------*/
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

-- PROGRAMMER: Filip Gutica
--
-- DESIGNER:   Filip Gutica
--
--	NOTES:	This function just reads for control chars that signal a transmission
--			is about to start.
--
/*-----------------------------------------------------------------------------*/
DWORD WINAPI ProtocolThread(LPVOID n)
{
	Statistics *s = Statistics::GetInstance();
	while (true)
	{
		if (getMode() != WRITE)
			ReadControlCharacter();

		if (getMode() == WRITE && got(ACK, t.TO2))
		{
			s->IncrementACKSReceived();
			UpdateStats();
			WriteMode();
		
		}	
	}

	ExitThread(0);
	return 0L;
}


char * ReadPort(double timeout)
{
		Statistics *s = Statistics::GetInstance();
		DWORD dwEvtMask;
		SetCommMask(portInfo.hComm, EV_RXCHAR);

		
	LPCOMMTIMEOUTS commTimeouts = new COMMTIMEOUTS();

	commTimeouts->ReadTotalTimeoutMultiplier = 0;
	commTimeouts->ReadTotalTimeoutConstant = timeout;



	if (!SetCommTimeouts(portInfo.hComm, commTimeouts))
	{
		//MessageBox(NULL, "Unable to set timeouts.", "Timeouts", MB_OK);
		return FALSE;
	}
	
	if (!SetCommMask(portInfo.hComm, EV_RXCHAR))
	{
		//MessageBox(NULL, "Error setting comm mask", "Comm Mask Error", MB_OK);
		return FALSE;
	}

	
	//clear port
	PurgeComm(portInfo.hComm, PURGE_RXCLEAR);	
	PurgeComm(portInfo.hComm, PURGE_TXCLEAR);
	PurgeComm(portInfo.hComm, PURGE_TXABORT);	
	PurgeComm(portInfo.hComm, PURGE_RXABORT);
	// Wait for a comm event
	if (WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
	{
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, portInfo.strReceive, sizeof(GrapefruitPacket), &portInfo.dwRead, &portInfo.overlapped))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
			}
		}
	}
	else
	{
		GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, portInfo.strReceive, sizeof(GrapefruitPacket), &(portInfo.dwRead), &portInfo.overlapped))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
			}
		}

	}
	//MessageBox(NULL, "PACKET READ FIN", "", MB_OK);

	dwEvtMask = NULL;
	portInfo.dwRead = 0;
	//MessageBox(NULL, portInfo.strReceive, "PACKET", MB_OK);

	return portInfo.strReceive;
}

/*-------------------------------------------------------------------------------------
-- FUNCTION ReadControlCharacter
--
-- PURPOSE:    When the thread has not begun transmission this function attempts to read
--			   control characters in constantly. If the thread is put into write mode by
--			   the user it will look for and ACK, otherwise it will look for an ENQ
--			   if the function gets the character it's looking for it will proceed to
--			   Write/Read Mode functions, if it doesn't it will continue back to the
--			   loop.
--
--
-- DATE:	   Novement 28th
--		
-- REVISIONS:  December 2nd: Adapted for double threaded application.
--
--
--
-- PROGRAMMER: Filip Gutica
--			   Marc Vouve
--
-- DESIGNER:   Filip Gutica
--			   Marc Vouve
--
--
-------------------------------------------------------------------------------------*/
void ReadControlCharacter(void)
{
	Statistics *s = Statistics::GetInstance();
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);
	char  ccontrol = '\0';

	// Wait for a comm event
	if(WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
	{
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped)) 
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
				//GetCharsFromPort(&ccontrol);
			}
		}
	}
	else
	{
		GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &(portInfo.dwRead), &portInfo.overlapped)) 
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
				//GetCharsFromPort(&ccontrol);
			}
		}
	}



	if (ccontrol == ENQ && getMode() == WAITING)
	{
		s->IncrementENQS();
		UpdateStats();
		setMode(READ);
		ReceiveMode();
		setMode(WAITING);
	}
	
	else
		false;
		
	dwEvtMask = NULL;
	portInfo.dwRead = 0;
}

BOOL got(char c, double timeout)
{
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);
	char  ccontrol = '\0';

	LPCOMMTIMEOUTS commTimeouts = new COMMTIMEOUTS();

	commTimeouts->ReadTotalTimeoutMultiplier = 0;
	commTimeouts->ReadTotalTimeoutConstant = timeout;

	if (!SetCommTimeouts(portInfo.hComm, commTimeouts))
	{
		//MessageBox(NULL, "Unable to set timeouts.", "Timeouts", MB_OK);
		return FALSE;
	}
	
	if (!SetCommMask(portInfo.hComm, EV_RXCHAR))
	{
		//MessageBox(NULL, "Error setting comm mask", "Comm Mask Error", MB_OK);
		return FALSE;
	}


	//clear port
	PurgeComm(portInfo.hComm, PURGE_RXCLEAR);	
	PurgeComm(portInfo.hComm, PURGE_TXCLEAR);
	PurgeComm(portInfo.hComm, PURGE_TXABORT);	
	PurgeComm(portInfo.hComm, PURGE_RXABORT);

	// Wait for a comm event
	if(WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
	{
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped)) 
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
				//GetCharsFromPort(&ccontrol);
			}
		}
	}
	else
	{
		GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &(portInfo.dwRead), &portInfo.overlapped)) 
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
				//GetCharsFromPort(&ccontrol);
			}
		}
	}
	dwEvtMask = NULL;
	portInfo.dwRead = 0;

	if (ccontrol == c)
		return TRUE;
	else
		return FALSE;
}



/*------------------------------------------------------------------------------
-- FUNCTION: WritePort( void * message )
--
-- PURPOSE:  This is a helper function for writing Packets to the port.
--
-- 
--
--
-- DESIGNER:   Marc Vouve 
--
-- PROGRAMMER: Marc Vouve 
--
--
------------------------------------------------------------------------------*/
BOOL WritePort( const void * message )
{
	return WriteFile(portInfo.hComm,  message,
		sizeof(message), &(portInfo.dwWritten), &(portInfo.overlapped));
}


/*------------------------------------------------------------------------------
-- FUNCTION: WritePort( void * message )
--
-- PURPOSE:  This is a helper function for writing Control chars to the port
--
-- DESIGNER:   Marc Vouve 
--
-- PROGRAMMER: Marc Vouve 
--
--
------------------------------------------------------------------------------*/
BOOL WriteControlChar( char control )
{
	if (!WriteFile(portInfo.hComm, &control, 1, &(portInfo.dwWritten), &(portInfo.overlapped)))
	{
		if (GetLastError() == 0x3e5)
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwWritten), TRUE);
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
-- DESIGNER:   Marc Vouve
--
-- PROGRAMMER: Marc Vouve 
--
-- DATE:       NOVEMBER 24th 2014
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

	//Respond to sender with ACK and await packet.
	WriteControlChar(ACK);
	stats->IncrementACKSSent();
	UpdateStats();

	for (int i = 0; i < MAXSENT; i++)
	{
		//MessageBox(NULL, "Waiting for packet" + i, "packet status", MB_OK);
		if (!WaitForPacket(packet))
		{
			return;
		}

		// validate packet, assuming valid for now.
		else if (true)
		{
			
			// checks if the second packet character is a syn bit 
			if (packet[1] != syn || ( i == 0 && ( packet[1] == SYN1 || packet[1] == SYN2 ) ) )
			{
				//MessageBox(NULL,"GOT A PACKEET", "packet", MB_OK);
				(packet[1] == SYN1 ? syn = SYN2 : syn = SYN1); // flip SYN.
	
				GetCharsFromPort(packet);
			}
			// SEND ACK on packet
			WriteControlChar(c);
			stats->IncrementACKSSent();
			UpdateStats();

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
--
------------------------------------------------------------------------------*/
void WriteMode()
{
	char packet[PACKET_SIZE];
	int miss = 0;
	Statistics *stats = Statistics::GetInstance();
	GrapefruitPacket temp = GrapefruitPacket();

	if (!packetQueue.empty())
	{

		temp = packetQueue.front();

			
		for ( int i = 0; i < MAXSENT; i++)
		{
			while ( miss < MAXMISS )
			{
				//Send packet
				//MessageBox(NULL, "sending packet", "packet status", MB_OK);

				packet[0] = temp.status;
				packet[1] = temp.sync;

				for (int i = 0 ; i < DATA_SIZE; i++)
				{
					packet[2 + i] = temp.data[i];
				}
				for (int i = 0; i < CRC_SIZE; i++)
				{
					packet[2 + DATA_SIZE + i] = temp.crc[i];
				}

				WritePort(&packet);

				//Wait for ACK

				if (got(ACK, t.TO3))
				{						
					stats->IncrementACKSReceived();					
					UpdateStats();
					packetQueue.pop_front();

					//ACK Received: update buffer and packetize
					//Check for EOT
					if ( temp.status == EOT )
					{
					
						//*******************CHANGE LATER*********************
						setBufferStatus(true);
						return;
					}						
					miss = 0;
				}
				//NAK Received: resend data and increment miss
				else if ( got(NAK, 1000) )
				{
					stats->IncrementNAKS();
					UpdateStats();
					miss++;
				}
				else
				{
					stats->IncrementPacketsLost();								
					UpdateStats();
				}
			}
		}
	}
	// no ack assumed failed
	//Go back to idle
	stats->IncrementPacketsLost();								
	UpdateStats();

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
	packet = ReadPort(t.TO1);
	//MessageBox(NULL, "got packet", "asdf", MB_OK);
	return true;
}

/*------------------------------------------------------------------------------
--	FUNCTION: PacketFactory(char *)
--
--	PURPOSE: Packetizes data and puts it in the queue to be sent.
--
--	PARAMETERS:
--		strToSend		- chars to be added to the buffer
--
-- DESIGNER:	Marc Vouve
--				Filip Gutica
--
-- PROGRAMMER:	Marc Vouve
--				Filip Gutica
--
-- NOTE: This functions sets the initial status to ETB, it is up to send mode
--		 to change it to an ETX on the last packet in Queue, or the 10th packet
--		 in the queue.
/*-----------------------------------------------------------------------------*/
void PacketFactory(char * strToSend)
{
	static char syn = SYN1;
	GrapefruitPacket temp = GrapefruitPacket();
	temp.status = ETB;

	// Itterate through the bytes to send.
	for (int i = 0; i < strlen(strToSend); i += sizeof( temp.data) )
	{
		strncpy( temp.data, strToSend + i, sizeof(temp.data));
		temp.sync = syn;
		syn ^= 1;	// swap to other SYN byte
	

		/* 
		CRC STUFF HERE!
		*/
		unsigned char packetdata[1020];
		packetdata[0] = temp.status; packetdata[1] = temp.sync;
		for(int i =0; i < DATA_SIZE; i++)
		{
			packetdata[i+2] = temp.data[i]; 
		}
		int crcBits = crcFast(packetdata, 1020);
		unsigned char *helping = (unsigned char*)&crcBits;
		temp.crc[0] = helping[3]; temp.crc[1] = helping[2];  temp.crc[2] = helping[1]; temp.crc[3] = helping[0]; 


		packetQueue.push_back(temp);
	}
	
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


	//MessageBox(NULL, "Reading for ACK.", "Timeouts", MB_OK);

	LPCOMMTIMEOUTS commTimeouts = new COMMTIMEOUTS();

	commTimeouts->ReadTotalTimeoutMultiplier = 0;
	commTimeouts->ReadTotalTimeoutConstant = timeout;

	if (!SetCommTimeouts(portInfo.hComm, commTimeouts))
	{
		//MessageBox(NULL, "Unable to set timeouts.", "Timeouts", MB_OK);
		return "";
	}
	
	if (!SetCommMask(portInfo.hComm, EV_RXCHAR))
	{
		//MessageBox(NULL, "Error setting comm mask", "Comm Mask Error", MB_OK);
		return "";
	}


	//clear port
	PurgeComm(portInfo.hComm, PURGE_RXCLEAR);	
	PurgeComm(portInfo.hComm, PURGE_TXCLEAR);
	PurgeComm(portInfo.hComm, PURGE_TXABORT);	
	PurgeComm(portInfo.hComm, PURGE_RXABORT);
	//Read port
	if (WaitCommEvent(portInfo.hComm, &commEvent, &(portInfo.overlapped)))
	{
		if (!ReadFile(portInfo.hComm, &temp, 1, &(portInfo.dwRead), &(portInfo.overlapped)))
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &numCharsRead, TRUE);
		}

		OutputDebugString(temp);
	}
	else
	{
		DWORD err = GetLastError();
 		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &numCharsRead, TRUE);

			if (!ReadFile(portInfo.hComm, &temp, 1, &(portInfo.dwRead), &(portInfo.overlapped)))
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &numCharsRead, TRUE);
			}

			OutputDebugString(temp);
		}
	}
		//MessageBox(NULL, "HERE", "HERE", MB_OK);
	portInfo.dwRead = 0;
	return temp;
}

void setBufferStatus(BOOL status)
{
	portInfo.empty = status;
}

BOOL isBufferEmpty()
{
	return portInfo.empty;
}

MODE getMode()
{
	return portInfo.mode;
}

void setMode( MODE m )
{
	portInfo.mode = m;
}
