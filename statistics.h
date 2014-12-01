#include <fstream>
#include <iostream>
#include <Windows.h>

class Statistics 
{
	public:
		Statistics();
		int GetNAKS();
		int GetACKS();	
		int GetPacketsSent();
		int GetPacketsLost();
		int GetReceived();
		int GetReceivedC();
		int GetENQS();
		static Statistics * GetInstance();	
		void IncrementACKS();
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
		void ResetIncrementPacketsSent();
		void ResetIncrementPacketsReceived();
		void ResetIncrementPacketsReceivedC();
		void SaveStatsToFile();

	private:
		static Statistics *instance;
		int NAKCount;
		int ACKCount;
		int packetsSent;
		int packetsLost;
		int received;
		int receivedCorrupt;
		int ENQCount;
};