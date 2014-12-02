#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <fstream>
#include <iostream>
#include <Windows.h>

class Statistics 
{
	public:
		Statistics();
		int GetNAKS();		
		int GetACKSent();	
		int GetACKReceived();	
		int GetPacketsSent();
		int GetPacketsLost();
		int GetReceived();
		int GetReceivedC();
		int GetENQS();
		static Statistics * GetInstance();	
		void IncrementACKSSent();		
		void IncrementACKSReceived();
		void IncrementNAKS();
		void IncrementENQS();
		void IncrementPacketsLost();
		void IncrementPacketsSent();
		void IncrementPacketsReceived();
		void IncrementPacketsReceivedC();
		void ResetACKS();
		void ResetNAKS();
		void ResetENQS();
		void ResetPacketsLost();
		void ResetPacketsSent();
		void ResetPacketsReceived();
		void ResetPacketsReceivedC();
		void SaveStatsToFile();

	private:
		static Statistics *instance;
		int NAKCount;
		int ACKSentCount;		
		int ACKReceivedCount;
		int packetsSent;
		int packetsLost;
		int received;
		int receivedCorrupt;
		int ENQCount;
};

#endif