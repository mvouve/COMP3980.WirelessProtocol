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

#include <sstream>

using std::stringstream;

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
		
		ReadControlCharacter();

		
	}

	ExitThread(0);
	return 0L;
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
char * ReadPort(double timeout)
{
	Statistics *s = Statistics::GetInstance();
	DWORD dwEvtMask;
	SetCommMask(portInfo.hComm, EV_RXCHAR);
	
	if (!SetCommMask(portInfo.hComm, EV_RXCHAR))
	{
		//MessageBox(NULL, "Error setting comm mask", "Comm Mask Error", MB_OK);
		return FALSE;
	}

	// Wait for a comm event
	int bytesRead = 0;
	while (bytesRead < PACKET_SIZE) 
	{
		if (WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
		{
			// Read in characters if character is found by waitcommevent
			if (!ReadFile(portInfo.hComm, portInfo.strReceive + bytesRead, PACKET_SIZE, &portInfo.dwRead, &portInfo.overlapped))
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
			if (!ReadFile(portInfo.hComm, portInfo.strReceive + bytesRead, PACKET_SIZE, &(portInfo.dwRead), &portInfo.overlapped))
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
				}
			}

		}

		bytesRead += portInfo.dwRead;

		if ( time(0) > t.TO1)
			break;
	}

	/*char temp[1024];
	sprintf(temp, "\nReceived Packet: %s ,%d", portInfo.strReceive, portInfo.dwRead);
	OutputDebugString(temp);*/

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
	if (WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
	{
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped))
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
		}
	}
	else
	{
		DWORD err = GetLastError();
		Sleep(100);

		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);

			if (dwEvtMask != EV_RXCHAR)
			{
				return;
			}

			if (!ReadFile(portInfo.hComm, &ccontrol, 1, &(portInfo.dwRead), &(portInfo.overlapped)))
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
			}
		}
	}
	char temp[50];
	sprintf(temp, "Got byte: %x", ccontrol);
	OutputDebugString(temp);

	if (getMode() == WRITE && time(0) > portInfo.timeout)
	{
		setMode(WAITING);
	}

	if (ccontrol == ENQ && getMode() == WAITING)
	{
		s->IncrementENQS();
		UpdateStats();
		setMode(READ);
		ReceiveMode();
		setMode(WAITING);
	}
	else if (ccontrol == ACK && getMode() == WRITE )
	{
		s->IncrementACKSReceived();
		UpdateStats();
		WriteMode();
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
	commTimeouts->ReadTotalTimeoutConstant = timeout;//timeout;

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

	//stringstream s;
	//s << "\n" << "what we want " << c << "\n";
	//OutputDebugString(s.str().c_str());
	// Wait for a comm event
	if(WaitCommEvent(portInfo.hComm, &dwEvtMask, &portInfo.overlapped))
	{
		// Read in characters if character is found by waitcommevent
		if (!ReadFile(portInfo.hComm, &ccontrol, 1, &portInfo.dwRead, &portInfo.overlapped)) 
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
		}
	}
	else
	{
		DWORD err = GetLastError();
		Sleep(100);
		
		if (err == ERROR_IO_PENDING)
		{
			GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);

			if (dwEvtMask != EV_RXCHAR)
			{
				return FALSE;
			}

			if (!ReadFile(portInfo.hComm, &ccontrol, 1, &(portInfo.dwRead), &(portInfo.overlapped)))
			{
				GetOverlappedResult(portInfo.hComm, &(portInfo.overlapped), &(portInfo.dwRead), TRUE);
			}
		}
		else
			return FALSE;
	}
	dwEvtMask = NULL;
	portInfo.dwRead = 0;

	//s.str("");
	//s << "what we got " << ccontrol << "\n";
	//OutputDebugString(s.str().c_str());
	if (ccontrol == c)
	{
		OutputDebugString("Got what im looking for");
		return TRUE;
	}
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
		//clear port
	PurgeComm(portInfo.hComm, PURGE_RXCLEAR);	
	PurgeComm(portInfo.hComm, PURGE_TXCLEAR);
	PurgeComm(portInfo.hComm, PURGE_TXABORT);	
	PurgeComm(portInfo.hComm, PURGE_RXABORT);
	
	if (!WriteFile(portInfo.hComm,  message, strlen((char *)message), 
			&portInfo.dwWritten, &portInfo.overlapped))
	{
		if (GetLastError() == ERROR_IO_PENDING)
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
	
	char *packet = '\0';
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
		packet = ReadPort(t.TO1);
		

		// validate packet, assuming valid for now.
		
		if (checkPacketCrc(packet))
		{
			
			// checks if the second packet character is a syn bit 
			if (packet[1] != syn || ( i == 0 && ( packet[1] == SYN1 || packet[1] == SYN2 ) ) )
			{
				(packet[1] == SYN1 ? syn = SYN2 : syn = SYN1); // flip SYN.
				//MessageBox(NULL, "GOT PACKET", "ASSFSF",MB_OK);
			
				OutputDebugString("RECEIVED STUFF: ");
				
				GrapefruitPacket receivedPacket = PacketFactory(packet);
			
				GetCharsFromPort(receivedPacket.data);
			}
			// SEND ACK on packet
			WriteControlChar(ACK);
			stats->IncrementACKSSent();
			UpdateStats();

			if (packet[1] == EOT )
			{
				return;
			}

			return;
		}
		else
		{
			//Send NAK
			WriteControlChar(NAK);
			stats->IncrementNAKS();
			UpdateStats();
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

	char *packet= new char[PACKET_SIZE];
	memset(packet, 0, sizeof(packet));
	int miss = 0;
	Statistics *stats = Statistics::GetInstance();
	


	if (!packetQueue.empty())
	{
		GrapefruitPacket temp = packetQueue.front();

	
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

				
				WritePort(packet);
				

				//Wait for ACK

				if (got(ACK, t.TO3))
				{						
					stats->IncrementACKSReceived();					
					UpdateStats();
					if (!packetQueue.empty())
						packetQueue.pop_front();

					//ACK Received: update buffer and packetize
					//Check for EOT
					if ( temp.status == EOT )
					{
						
						return;
					}						
					miss = 0;
					return;
				}
				//NAK Received: resend data and increment miss
				else if ( got(NAK, 0) )
				{
					stats->IncrementNAKS();
					UpdateStats();
					miss++;
				}
				else
				{
					stats->IncrementPacketsLost();								
					UpdateStats();
					miss++;
				}
			}
		}
		
	}
	// no ack assumed failed
	//Go back to idle
	setMode(WAITING);
	return;

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
GrapefruitPacket PacketFactory(char * strToSend)
{
	
	static char syn = SYN1;
	GrapefruitPacket temp = GrapefruitPacket();
	temp.status = ETB;

	temp.sync = syn;

	if (syn == SYN1)	syn = SYN2;
	else				syn = SYN1;
	int count = 0;
	for(count = 0; count < DATA_SIZE && count < strlen(strToSend) ; count++)
	{
		temp.data[count] = strToSend[count];
	}
	if (strlen(strToSend) < DATA_SIZE)
	{
		temp.status = ETB;
		count++;
		for(int i = count; i < DATA_SIZE ; i++)
		{
			temp.data[i] = ETX;
		}
	}
	else
	{ 
		temp.status = EOT;
	}
	
	int crcBits = crcFast((unsigned char*)temp.data, DATA_SIZE);
	unsigned char *helping = new unsigned char[4];
	helping = reinterpret_cast<unsigned char*>(&crcBits);
	//MessageBox(NULL, "hkjhge", "fdssdfe", MB_OK);
	//unsigned char *helping = (unsigned char*)&crcBits;

	for(int i = 0; i < CRC_SIZE; i++)
	{
		temp.crc[i] = helping[i];
	}

	//int *bob = reinterpret_cast<int*>(temp.crc);


	//char packetThing[1024];
	
	packetQueue.push_back(temp);

	return temp;
		
}

BOOL checkPacketCrc(char *thing)
{
	int crcCheck = crcFast((unsigned char*)thing+2, DATA_SIZE);
	int *helping = reinterpret_cast<int*>(thing+1020);

	/*char output[200];
	sprintf(output, "Crc calc : %x, Crc there : %x", crcCheck, *helping);
	OutputDebugString(output);*/

	OutputDebugString("CHECKING CRC ON: ");
	OutputDebugString(thing);
	

	if(0 == *helping)
	{
		//MessageBox(NULL,"CRC GOOD", "CRC", MB_OK);
		return true;
	}
	//MessageBox(NULL,"CRC BAD", "CRC", MB_OK);
	return true;
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
	if (m == WRITE)
		portInfo.timeout = time(0) + t.TO2;

	portInfo.mode = m;
}
