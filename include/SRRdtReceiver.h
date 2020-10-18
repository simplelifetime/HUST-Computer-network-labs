#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// �ڴ��յ�����һ���������
	int seqsize;
	int winsizeN;
	int tempPacketrcv[8];
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���
	Packet temppacket[8];

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
	bool isinwindow(int seqNum);
	// void printSlideWindow();
};

#endif