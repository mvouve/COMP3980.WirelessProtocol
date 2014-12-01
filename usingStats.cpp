#include "statistics.h"

using namespace std;

Statistics *Statistics::instance = nullptr;
Statistics::Statistics()
{
	NAKCount = 0;
	ACKCount = 0;
	ENQCount = 0;
	packetsSent = 0;
	packetsLost = 0;
	received = 0;
	receivedCorrupt = 0;

}


int Statistics::GetNAKS()
{
	return NAKCount;

}


int Statistics::GetPacketsSent()
{
	return packetsSent;
}

int Statistics::GetPacketsLost()
{
	return packetsLost;
}

int Statistics::GetReceived()
{
	return received;
}

int Statistics::GetReceivedC()
{
	return receivedCorrupt;
}


int Statistics::GetACKS()
{
	return ACKCount;

}

int Statistics::GetENQS()
{
	return ENQCount;

}
void Statistics::IncrementACKS()
{
	ACKCount++;
	return;

}

void Statistics::IncrementNAKS()
{
	NAKCount++;
	return;
}


void Statistics::IncrementPacketsLost()
{
	packetsLost++;
	return;
}

void Statistics::IncrementPacketsSent()
{
	packetsSent++;
	return;
}

void Statistics::IncrementPacketsReceived()
{
	received++;
	return;
}

void Statistics::IncrementPacketsReceivedC()
{
	receivedCorrupt++;
	return;
}

void Statistics::IncrementENQS()
{
	ENQCount++;
	return;
}


void Statistics::ResetACKS()
{
	ACKCount = 0;	
	return;
}

void Statistics::ResetNAKS()
{
	NAKCount = 0;
}


void  Statistics::ResetPacketsLost()
{
	packetsLost = 0;
}

void  Statistics::ResetIncrementPacketsSent()
{
	packetsSent = 0;
}

void  Statistics::ResetIncrementPacketsReceived()
{
	received = 0;
}
void  Statistics::ResetIncrementPacketsReceivedC()
{
	receivedCorrupt = 0;
}

void Statistics::ResetENQS()
{
	ENQCount = 0;
}

void Statistics::SaveStatsToFile()
{
	std::ofstream outFile;
	outFile.open("finalStats.txt");

	if (outFile.is_open())
	{
		outFile << "NAKs: " << NAKCount << endl;
		outFile << "ACKs: " << ACKCount << endl;
		outFile << "Packets Sent: " << packetsSent << endl;
		outFile.close();

	}
	else
	{
		MessageBox(NULL, "File writing issue.", "Error", MB_OK | MB_ICONERROR);
	}

	return;
}

Statistics * Statistics::GetInstance()
{
	if ( instance == nullptr )
	{
		instance = new Statistics();
		OutputDebugString("Debugstatement");
	}

	return instance;
}