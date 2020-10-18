#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	int seqsize;
	int winsizeN;
	int tempPacketrcv[8];
	Packet lastAckPkt;				//上次发送的确认报文
	Packet temppacket[8];

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
	bool isinwindow(int seqNum);
	// void printSlideWindow();
};

#endif